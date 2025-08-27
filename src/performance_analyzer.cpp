#include "performance_analyzer.h"
#include "bottleneck_detector.h"
#include "optimization_engine.h"
#include "gpu_analyzer.h"
#include "parallel_insights.h"
#include "metrics_collector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cmath>
#include <thread>

namespace perf_analyzer {

PerformanceAnalyzer::PerformanceAnalyzer() 
    : analysis_duration_(10.0)
    , gpu_analysis_enabled_(true)
    , verbose_output_(false) {
    initializeComponents();
}

PerformanceAnalyzer::~PerformanceAnalyzer() = default;

void PerformanceAnalyzer::initializeComponents() {
    bottleneck_detector_ = std::make_unique<BottleneckDetector>();
    optimization_engine_ = std::make_unique<OptimizationEngine>();
    gpu_analyzer_ = std::make_unique<GPUAnalyzer>();
    parallel_insights_ = std::make_unique<ParallelInsights>();
    metrics_collector_ = std::make_unique<MetricsCollector>();
    
    // Configure components
    parallel_insights_->setAnalysisPeriod(analysis_duration_);
    gpu_analyzer_->setProfilingDuration(analysis_duration_);
}

bool PerformanceAnalyzer::analyzeProgram(const std::string& program_path, 
                                        const std::vector<std::string>& args) {
    if (verbose_output_) {
        std::cout << "Starting analysis of program: " << program_path << std::endl;
    }
    
    // Fork and execute the target program
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute the target program
        std::vector<char*> argv_c;
        argv_c.push_back(const_cast<char*>(program_path.c_str()));
        for (const auto& arg : args) {
            argv_c.push_back(const_cast<char*>(arg.c_str()));
        }
        argv_c.push_back(nullptr);
        
        execv(program_path.c_str(), argv_c.data());
        // If execv fails
        std::cerr << "Failed to execute program: " << program_path << std::endl;
        exit(1);
    } else if (pid > 0) {
        // Parent process - analyze the child
        bool result = analyzeRunningProcess(pid);
        
        // Wait for child to complete or terminate it if analysis is done
        int status;
        if (waitpid(pid, &status, WNOHANG) == 0) {
            // Child still running, terminate it
            kill(pid, SIGTERM);
            waitpid(pid, &status, 0);
        }
        
        return result;
    } else {
        std::cerr << "Failed to fork process" << std::endl;
        return false;
    }
}

bool PerformanceAnalyzer::analyzeRunningProcess(int pid) {
    if (verbose_output_) {
        std::cout << "Analyzing running process PID: " << pid << std::endl;
    }
    
    // Check if process exists
    if (kill(pid, 0) != 0) {
        std::cerr << "Process " << pid << " does not exist or is not accessible" << std::endl;
        return false;
    }
    
    try {
        // Configure metrics collector
        metrics_collector_->setTargetProcess(pid);
        metrics_collector_->enableGPUMonitoring(gpu_analysis_enabled_);
        
        // Start analysis
        runAnalysis();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Analysis failed: " << e.what() << std::endl;
        return false;
    }
}

void PerformanceAnalyzer::runAnalysis() {
    if (verbose_output_) {
        std::cout << "Running performance analysis..." << std::endl;
    }
    
    // Start continuous monitoring
    if (!metrics_collector_->startCollection()) {
        throw std::runtime_error("Failed to start metrics collection");
    }
    
    // Initialize GPU profiling if enabled
    if (gpu_analysis_enabled_) {
        gpu_analyzer_->initializeGPUProfiling();
    }
    
    // Wait for analysis duration
    std::this_thread::sleep_for(std::chrono::milliseconds(
        static_cast<int>(analysis_duration_ * 1000)));
    
    // Stop collection and gather results
    metrics_collector_->stopCollection();
    
    // Collect final metrics
    collectMetrics();
    
    // Detect bottlenecks
    detectBottlenecks();
    
    // Generate optimization suggestions
    generateSuggestions();
    
    if (verbose_output_) {
        std::cout << "Analysis complete." << std::endl;
    }
}

void PerformanceAnalyzer::collectMetrics() {
    // Get current metrics snapshot
    metrics_ = metrics_collector_->collectCurrentMetrics();
    
    // Enhance with GPU metrics if available
    if (gpu_analysis_enabled_ && gpu_analyzer_->isCUDAAvailable()) {
        auto gpu_metrics = gpu_analyzer_->analyzeGPUPerformance();
        metrics_.gpu_utilization = gpu_metrics.gpu_utilization.empty() ? 0.0 : 
                                  gpu_metrics.gpu_utilization[0];
        metrics_.gpu_memory_usage_mb = gpu_metrics.used_gpu_memory_gb * 1024.0;
    }
    
    // Add parallel insights
    auto load_balance = parallel_insights_->analyzeLoadBalance();
    metrics_.thread_load_balance = load_balance.thread_workloads;
    metrics_.synchronization_overhead = parallel_insights_->calculateSynchronizationOverhead();
    metrics_.communication_overhead = parallel_insights_->calculateCommunicationOverhead();
}

void PerformanceAnalyzer::detectBottlenecks() {
    bottlenecks_ = bottleneck_detector_->detectBottlenecks(metrics_);
    
    // Sort bottlenecks by severity
    std::sort(bottlenecks_.begin(), bottlenecks_.end(), 
              [](const Bottleneck& a, const Bottleneck& b) {
                  return a.severity_score > b.severity_score;
              });
}

void PerformanceAnalyzer::generateSuggestions() {
    suggestions_ = optimization_engine_->generateSuggestions(metrics_, bottlenecks_);
    
    // Add GPU-specific suggestions if GPU analysis is enabled
    if (gpu_analysis_enabled_ && gpu_analyzer_->isCUDAAvailable()) {
        auto gpu_metrics = gpu_analyzer_->analyzeGPUPerformance();
        auto gpu_suggestions = gpu_analyzer_->suggestGPUOptimizations(gpu_metrics);
        suggestions_.insert(suggestions_.end(), gpu_suggestions.begin(), gpu_suggestions.end());
    }
    
    // Add parallel programming suggestions
    auto parallel_suggestions = parallel_insights_->suggestCommunicationOptimizations();
    suggestions_.insert(suggestions_.end(), parallel_suggestions.begin(), parallel_suggestions.end());
    
    // Sort suggestions by priority and potential improvement
    std::sort(suggestions_.begin(), suggestions_.end(),
              [](const OptimizationSuggestion& a, const OptimizationSuggestion& b) {
                  if (a.priority != b.priority) {
                      return a.priority < b.priority;  // Lower priority number = higher priority
                  }
                  return a.potential_improvement > b.potential_improvement;
              });
}

void PerformanceAnalyzer::generateReport(const std::string& output_file) const {
    std::ofstream file;
    std::ostream* out = &std::cout;
    
    if (!output_file.empty()) {
        file.open(output_file);
        if (file.is_open()) {
            out = &file;
        } else {
            std::cerr << "Warning: Could not open output file, using stdout" << std::endl;
        }
    }
    
    *out << "Parallel Programming Performance Analysis Report\n";
    *out << "================================================\n\n";
    
    // System information
    *out << "Analysis Configuration:\n";
    *out << "  Duration: " << analysis_duration_ << " seconds\n";
    *out << "  GPU Analysis: " << (gpu_analysis_enabled_ ? "Enabled" : "Disabled") << "\n";
    *out << "  Timestamp: " << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << "\n\n";
    
    // Performance metrics
    *out << "Performance Metrics:\n";
    *out << "  Execution Time: " << std::fixed << std::setprecision(2) 
         << metrics_.execution_time_ms << " ms\n";
    *out << "  CPU Utilization: " << std::fixed << std::setprecision(1) 
         << metrics_.cpu_utilization << "%\n";
    *out << "  Memory Usage: " << std::fixed << std::setprecision(1) 
         << metrics_.memory_usage_mb << " MB\n";
    *out << "  Thread Count: " << metrics_.thread_count << "\n";
    
    if (gpu_analysis_enabled_) {
        *out << "  GPU Utilization: " << std::fixed << std::setprecision(1) 
             << metrics_.gpu_utilization << "%\n";
        *out << "  GPU Memory Usage: " << std::fixed << std::setprecision(1) 
             << metrics_.gpu_memory_usage_mb << " MB\n";
    }
    
    *out << "  Synchronization Overhead: " << std::fixed << std::setprecision(1) 
         << metrics_.synchronization_overhead << "%\n";
    *out << "  Communication Overhead: " << std::fixed << std::setprecision(1) 
         << metrics_.communication_overhead << "%\n\n";
    
    // Load balance analysis
    if (!metrics_.thread_load_balance.empty()) {
        *out << "Thread Load Balance:\n";
        double total_load = 0.0;
        for (size_t i = 0; i < metrics_.thread_load_balance.size(); ++i) {
            *out << "  Thread " << i << ": " << std::fixed << std::setprecision(1) 
                 << metrics_.thread_load_balance[i] << "%\n";
            total_load += metrics_.thread_load_balance[i];
        }
        double avg_load = total_load / metrics_.thread_load_balance.size();
        double load_variance = 0.0;
        for (double load : metrics_.thread_load_balance) {
            load_variance += (load - avg_load) * (load - avg_load);
        }
        load_variance /= metrics_.thread_load_balance.size();
        *out << "  Load Balance Factor: " << std::fixed << std::setprecision(3) 
             << sqrt(load_variance) / avg_load << "\n\n";
    }
    
    // Bottlenecks
    *out << "Identified Bottlenecks:\n";
    if (bottlenecks_.empty()) {
        *out << "  No significant bottlenecks detected.\n\n";
    } else {
        for (size_t i = 0; i < bottlenecks_.size(); ++i) {
            const auto& bottleneck = bottlenecks_[i];
            *out << "  " << (i + 1) << ". " << bottleneck.description << "\n";
            *out << "     Severity: " << std::fixed << std::setprecision(1) 
                 << (bottleneck.severity_score * 100) << "%\n";
            if (!bottleneck.location.empty()) {
                *out << "     Location: " << bottleneck.location << "\n";
            }
            *out << "\n";
        }
    }
    
    // Optimization suggestions
    *out << "Optimization Suggestions:\n";
    if (suggestions_.empty()) {
        *out << "  No optimization suggestions available.\n\n";
    } else {
        for (size_t i = 0; i < suggestions_.size(); ++i) {
            const auto& suggestion = suggestions_[i];
            *out << "  " << (i + 1) << ". " << suggestion.title << "\n";
            *out << "     Description: " << suggestion.description << "\n";
            *out << "     Priority: " << suggestion.priority << "/5\n";
            if (suggestion.potential_improvement > 0) {
                *out << "     Potential Improvement: " << std::fixed << std::setprecision(1) 
                     << suggestion.potential_improvement << "%\n";
            }
            if (!suggestion.implementation_steps.empty()) {
                *out << "     Implementation Steps:\n";
                for (const auto& step : suggestion.implementation_steps) {
                    *out << "       - " << step << "\n";
                }
            }
            *out << "\n";
        }
    }
    
    if (file.is_open()) {
        file.close();
    }
}

void PerformanceAnalyzer::printSummary() const {
    std::cout << "Performance Analysis Summary\n";
    std::cout << "============================\n\n";
    
    std::cout << "📊 Key Metrics:\n";
    std::cout << "  • Execution Time: " << std::fixed << std::setprecision(2) 
              << metrics_.execution_time_ms << " ms\n";
    std::cout << "  • CPU Utilization: " << std::fixed << std::setprecision(1) 
              << metrics_.cpu_utilization << "%\n";
    std::cout << "  • Memory Usage: " << std::fixed << std::setprecision(1) 
              << metrics_.memory_usage_mb << " MB\n";
    std::cout << "  • Active Threads: " << metrics_.thread_count << "\n";
    
    if (gpu_analysis_enabled_ && metrics_.gpu_utilization > 0) {
        std::cout << "  • GPU Utilization: " << std::fixed << std::setprecision(1) 
                  << metrics_.gpu_utilization << "%\n";
    }
    
    std::cout << "\n⚡ Performance Efficiency:\n";
    
    // Calculate overall efficiency score
    double efficiency_score = 100.0;
    if (metrics_.synchronization_overhead > 10.0) efficiency_score -= 20.0;
    if (metrics_.communication_overhead > 5.0) efficiency_score -= 15.0;
    if (metrics_.cpu_utilization < 70.0) efficiency_score -= 15.0;
    
    std::cout << "  • Overall Efficiency: " << std::fixed << std::setprecision(1) 
              << std::max(0.0, efficiency_score) << "%\n";
    
    if (metrics_.synchronization_overhead > 0) {
        std::cout << "  • Synchronization Overhead: " << std::fixed << std::setprecision(1) 
                  << metrics_.synchronization_overhead << "%\n";
    }
    
    if (metrics_.communication_overhead > 0) {
        std::cout << "  • Communication Overhead: " << std::fixed << std::setprecision(1) 
                  << metrics_.communication_overhead << "%\n";
    }
    
    // Load balance summary
    if (!metrics_.thread_load_balance.empty() && metrics_.thread_load_balance.size() > 1) {
        double min_load = *std::min_element(metrics_.thread_load_balance.begin(), 
                                          metrics_.thread_load_balance.end());
        double max_load = *std::max_element(metrics_.thread_load_balance.begin(), 
                                          metrics_.thread_load_balance.end());
        double load_imbalance = (max_load - min_load) / max_load * 100.0;
        
        std::cout << "  • Load Imbalance: " << std::fixed << std::setprecision(1) 
                  << load_imbalance << "%\n";
    }
    
    std::cout << "\n";
}

} // namespace perf_analyzer