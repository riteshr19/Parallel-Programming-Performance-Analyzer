#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <map>

namespace perf_analyzer {

// Forward declarations
class BottleneckDetector;
class OptimizationEngine;
class GPUAnalyzer;
class ParallelInsights;
class MetricsCollector;

// Performance metrics structure
struct PerformanceMetrics {
    double execution_time_ms;
    double cpu_utilization;
    double memory_usage_mb;
    double gpu_utilization;
    double gpu_memory_usage_mb;
    int thread_count;
    double synchronization_overhead;
    double communication_overhead;
    std::vector<double> thread_load_balance;
};

// Bottleneck information
struct Bottleneck {
    enum Type {
        CPU_BOUND,
        MEMORY_BOUND,
        GPU_BOUND,
        SYNCHRONIZATION,
        COMMUNICATION,
        LOAD_IMBALANCE
    };
    
    Type type;
    std::string description;
    double severity_score;  // 0.0 to 1.0
    std::string location;
};

// Optimization suggestion
struct OptimizationSuggestion {
    std::string title;
    std::string description;
    double potential_improvement;  // Estimated percentage improvement
    int priority;  // 1 (highest) to 5 (lowest)
    std::vector<std::string> implementation_steps;
};

// Main performance analyzer class
class PerformanceAnalyzer {
public:
    PerformanceAnalyzer();
    ~PerformanceAnalyzer();

    // Main analysis functions
    bool analyzeProgram(const std::string& program_path, const std::vector<std::string>& args = {});
    bool analyzeRunningProcess(int pid);
    
    // Get analysis results
    const PerformanceMetrics& getMetrics() const { return metrics_; }
    const std::vector<Bottleneck>& getBottlenecks() const { return bottlenecks_; }
    const std::vector<OptimizationSuggestion>& getSuggestions() const { return suggestions_; }
    
    // Configuration
    void setAnalysisDuration(double seconds) { analysis_duration_ = seconds; }
    void enableGPUAnalysis(bool enable) { gpu_analysis_enabled_ = enable; }
    void setVerboseOutput(bool verbose) { verbose_output_ = verbose; }
    
    // Report generation
    void generateReport(const std::string& output_file = "") const;
    void printSummary() const;

private:
    // Core components
    std::unique_ptr<BottleneckDetector> bottleneck_detector_;
    std::unique_ptr<OptimizationEngine> optimization_engine_;
    std::unique_ptr<GPUAnalyzer> gpu_analyzer_;
    std::unique_ptr<ParallelInsights> parallel_insights_;
    std::unique_ptr<MetricsCollector> metrics_collector_;
    
    // Analysis results
    PerformanceMetrics metrics_;
    std::vector<Bottleneck> bottlenecks_;
    std::vector<OptimizationSuggestion> suggestions_;
    
    // Configuration
    double analysis_duration_;
    bool gpu_analysis_enabled_;
    bool verbose_output_;
    
    // Internal methods
    void initializeComponents();
    void runAnalysis();
    void collectMetrics();
    void detectBottlenecks();
    void generateSuggestions();
};

} // namespace perf_analyzer