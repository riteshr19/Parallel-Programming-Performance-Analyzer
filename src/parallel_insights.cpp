#include "parallel_insights.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <thread>

namespace perf_analyzer {

ParallelInsights::ParallelInsights() 
    : analysis_period_(10.0)
    , detailed_thread_analysis_(false)
    , sampling_rate_hz_(100) {
}

ParallelInsights::~ParallelInsights() = default;

std::vector<ThreadMetrics> ParallelInsights::analyzeThreadPerformance() {
    // Simulate thread performance analysis
    // In a real implementation, this would collect actual thread metrics
    
    std::vector<ThreadMetrics> metrics;
    int thread_count = std::thread::hardware_concurrency();
    
    for (int i = 0; i < thread_count; ++i) {
        ThreadMetrics thread_metric;
        thread_metric.thread_id = i;
        thread_metric.cpu_time_ms = 1000.0 + (i * 100.0); // Simulated CPU time
        thread_metric.wall_time_ms = 1200.0 + (i * 80.0);  // Simulated wall time
        thread_metric.cpu_utilization = 70.0 + (i % 3) * 10.0; // Varied utilization
        thread_metric.context_switches = 50 + (i * 10);
        thread_metric.cache_misses = 1000 + (i * 200);
        thread_metric.synchronization_wait_time = 50.0 + (i % 4) * 25.0;
        thread_metric.synchronization_objects = {"mutex_1", "condition_var_1"};
        
        metrics.push_back(thread_metric);
    }
    
    thread_metrics_ = metrics;
    return metrics;
}

std::vector<CommunicationPattern> ParallelInsights::analyzeCommunicationPatterns() {
    std::vector<CommunicationPattern> patterns;
    
    // Simulate communication pattern detection
    CommunicationPattern pattern1;
    pattern1.type = CommunicationPattern::POINT_TO_POINT;
    pattern1.frequency = 150.0; // messages per second
    pattern1.average_latency = 0.05; // ms
    pattern1.total_data_transferred = 1024.0 * 1024.0; // bytes
    pattern1.participating_threads = {0, 1};
    patterns.push_back(pattern1);
    
    CommunicationPattern pattern2;
    pattern2.type = CommunicationPattern::BROADCAST;
    pattern2.frequency = 25.0;
    pattern2.average_latency = 0.2;
    pattern2.total_data_transferred = 512.0 * 1024.0;
    pattern2.participating_threads = {0, 1, 2, 3};
    patterns.push_back(pattern2);
    
    communication_patterns_ = patterns;
    return patterns;
}

LoadBalanceAnalysis ParallelInsights::analyzeLoadBalance() {
    LoadBalanceAnalysis analysis;
    
    // Simulate load balance analysis
    analysis.thread_workloads = {85.0, 92.0, 78.0, 88.0, 95.0, 82.0, 90.0, 85.0};
    analysis.thread_idle_times = {15.0, 8.0, 22.0, 12.0, 5.0, 18.0, 10.0, 15.0};
    
    // Calculate load balance factor
    auto min_max = std::minmax_element(analysis.thread_workloads.begin(), 
                                     analysis.thread_workloads.end());
    double workload_range = *min_max.second - *min_max.first;
    double avg_workload = std::accumulate(analysis.thread_workloads.begin(), 
                                        analysis.thread_workloads.end(), 0.0) / 
                         analysis.thread_workloads.size();
    
    analysis.load_balance_factor = workload_range / avg_workload;
    
    // Determine imbalance cause
    if (analysis.load_balance_factor > 0.3) {
        analysis.imbalance_cause = "Uneven work distribution - some threads have significantly more work";
    } else if (analysis.load_balance_factor > 0.2) {
        analysis.imbalance_cause = "Moderate load imbalance - minor work distribution issues";
    } else {
        analysis.imbalance_cause = "Good load balance across threads";
    }
    
    // Generate balancing suggestions
    analysis.balancing_suggestions = generateLoadBalancingSuggestions(analysis);
    
    load_balance_analysis_ = analysis;
    return analysis;
}

double ParallelInsights::calculateSynchronizationOverhead() {
    // Simulate synchronization overhead calculation
    double total_sync_time = 0.0;
    double total_execution_time = 0.0;
    
    for (const auto& thread_metric : thread_metrics_) {
        total_sync_time += thread_metric.synchronization_wait_time;
        total_execution_time += thread_metric.wall_time_ms;
    }
    
    if (total_execution_time > 0.0) {
        return (total_sync_time / total_execution_time) * 100.0;
    }
    
    return 5.0; // Default 5% synchronization overhead
}

std::vector<std::string> ParallelInsights::identifySynchronizationHotspots() {
    std::vector<std::string> hotspots;
    
    // Analyze synchronization patterns
    std::map<std::string, double> sync_object_times;
    
    for (const auto& thread_metric : thread_metrics_) {
        for (const auto& sync_obj : thread_metric.synchronization_objects) {
            sync_object_times[sync_obj] += thread_metric.synchronization_wait_time;
        }
    }
    
    // Identify objects with high contention
    for (const auto& pair : sync_object_times) {
        if (pair.second > 100.0) { // More than 100ms total wait time
            hotspots.push_back("High contention on " + pair.first + 
                             " (total wait: " + std::to_string(static_cast<int>(pair.second)) + "ms)");
        }
    }
    
    if (hotspots.empty()) {
        hotspots.push_back("No significant synchronization hotspots detected");
    }
    
    return hotspots;
}

std::map<std::string, double> ParallelInsights::analyzeLockContention() {
    std::map<std::string, double> contention_map;
    
    // Simulate lock contention analysis
    contention_map["mutex_1"] = 15.2; // 15.2% contention
    contention_map["condition_var_1"] = 8.5; // 8.5% contention
    contention_map["shared_lock_1"] = 3.1; // 3.1% contention
    
    return contention_map;
}

double ParallelInsights::predictScalability(int target_thread_count) {
    // Use Amdahl's law to predict scalability
    double serial_fraction = calculateSynchronizationOverhead() / 100.0;
    double parallel_fraction = 1.0 - serial_fraction;
    
    // Amdahl's law: Speedup = 1 / (serial_fraction + parallel_fraction / n)
    double current_speedup = calculateAmdahlsLaw(serial_fraction, thread_metrics_.size());
    double target_speedup = calculateAmdahlsLaw(serial_fraction, target_thread_count);
    
    return target_speedup / current_speedup;
}

std::vector<std::string> ParallelInsights::identifyScalabilityBottlenecks() {
    std::vector<std::string> bottlenecks;
    
    double sync_overhead = calculateSynchronizationOverhead();
    double comm_overhead = calculateCommunicationOverhead();
    
    if (sync_overhead > 20.0) {
        bottlenecks.push_back("High synchronization overhead (" + 
                            std::to_string(static_cast<int>(sync_overhead)) + 
                            "%) will limit scalability");
    }
    
    if (comm_overhead > 15.0) {
        bottlenecks.push_back("High communication overhead (" + 
                            std::to_string(static_cast<int>(comm_overhead)) + 
                            "%) will limit scalability");
    }
    
    if (load_balance_analysis_.load_balance_factor > 0.3) {
        bottlenecks.push_back("Load imbalance will become worse with more threads");
    }
    
    // Check for sequential bottlenecks
    int sequential_sections = 0;
    for (const auto& pattern : communication_patterns_) {
        if (pattern.type == CommunicationPattern::POINT_TO_POINT && 
            pattern.participating_threads.size() == 2) {
            sequential_sections++;
        }
    }
    
    if (sequential_sections > 3) {
        bottlenecks.push_back("Too many sequential communication patterns detected");
    }
    
    if (bottlenecks.empty()) {
        bottlenecks.push_back("No major scalability bottlenecks detected");
    }
    
    return bottlenecks;
}

double ParallelInsights::analyzeCacheEfficiency() {
    // Simulate cache efficiency analysis
    double total_cache_misses = 0.0;
    double total_cache_accesses = 0.0;
    
    for (const auto& thread_metric : thread_metrics_) {
        total_cache_misses += thread_metric.cache_misses;
        total_cache_accesses += thread_metric.cache_misses * 20; // Assume 20:1 ratio
    }
    
    if (total_cache_accesses > 0.0) {
        return 1.0 - (total_cache_misses / total_cache_accesses);
    }
    
    return 0.85; // Default 85% cache efficiency
}

double ParallelInsights::analyzeMemoryAccessPatterns() {
    // Simulate memory access pattern analysis
    // This would typically analyze memory access locality and patterns
    return 0.75; // 75% efficient memory access patterns
}

std::vector<std::string> ParallelInsights::suggestMemoryOptimizations() {
    std::vector<std::string> suggestions;
    
    double cache_efficiency = analyzeCacheEfficiency();
    double memory_patterns = analyzeMemoryAccessPatterns();
    
    if (cache_efficiency < 0.8) {
        suggestions.push_back("Improve cache efficiency by optimizing data structures for locality");
        suggestions.push_back("Consider loop tiling/blocking to improve cache reuse");
    }
    
    if (memory_patterns < 0.7) {
        suggestions.push_back("Optimize memory access patterns for better spatial locality");
        suggestions.push_back("Consider data structure reorganization (AoS vs SoA)");
    }
    
    suggestions.push_back("Use prefetching for predictable memory access patterns");
    suggestions.push_back("Align data structures to cache line boundaries");
    
    return suggestions;
}

double ParallelInsights::calculateThreadEfficiency() {
    if (thread_metrics_.empty()) {
        return 0.0;
    }
    
    double total_efficiency = 0.0;
    for (const auto& thread_metric : thread_metrics_) {
        double efficiency = thread_metric.cpu_time_ms / thread_metric.wall_time_ms;
        total_efficiency += std::min(1.0, efficiency);
    }
    
    return total_efficiency / thread_metrics_.size();
}

std::vector<std::string> ParallelInsights::identifyIdleThreads() {
    std::vector<std::string> idle_threads;
    
    for (const auto& thread_metric : thread_metrics_) {
        if (thread_metric.cpu_utilization < 30.0) {
            idle_threads.push_back("Thread " + std::to_string(thread_metric.thread_id) + 
                                 " has low utilization (" + 
                                 std::to_string(static_cast<int>(thread_metric.cpu_utilization)) + "%)");
        }
    }
    
    if (idle_threads.empty()) {
        idle_threads.push_back("No significantly idle threads detected");
    }
    
    return idle_threads;
}

std::vector<std::string> ParallelInsights::suggestThreadOptimizations() {
    std::vector<std::string> suggestions;
    
    double thread_efficiency = calculateThreadEfficiency();
    auto idle_threads = identifyIdleThreads();
    
    if (thread_efficiency < 0.7) {
        suggestions.push_back("Thread efficiency is low - consider reducing thread count or improving work distribution");
    }
    
    if (idle_threads.size() > 1 && idle_threads[0] != "No significantly idle threads detected") {
        suggestions.push_back("Multiple idle threads detected - consider dynamic work assignment");
        suggestions.push_back("Implement work stealing to better utilize idle threads");
    }
    
    double avg_context_switches = 0.0;
    for (const auto& metric : thread_metrics_) {
        avg_context_switches += metric.context_switches;
    }
    avg_context_switches /= thread_metrics_.size();
    
    if (avg_context_switches > 1000) {
        suggestions.push_back("High context switch rate detected - consider reducing thread contention");
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> ParallelInsights::suggestCommunicationOptimizations() {
    std::vector<OptimizationSuggestion> suggestions;
    
    double comm_overhead = calculateCommunicationOverhead();
    
    if (comm_overhead > 10.0) {
        OptimizationSuggestion suggestion;
        suggestion.title = "Reduce Communication Overhead";
        suggestion.description = "High communication overhead detected (" + 
                               std::to_string(static_cast<int>(comm_overhead)) + 
                               "%). Consider optimizing inter-thread communication.";
        suggestion.potential_improvement = 15.0;
        suggestion.priority = 2;
        suggestion.implementation_steps = {
            "Batch multiple small messages into larger ones",
            "Use shared memory instead of message passing where possible",
            "Implement producer-consumer patterns with efficient queues",
            "Consider using memory pools for frequent data exchange"
        };
        suggestions.push_back(suggestion);
    }
    
    // Analyze communication patterns
    for (const auto& pattern : communication_patterns_) {
        if (pattern.average_latency > 1.0) { // High latency communication
            OptimizationSuggestion suggestion;
            suggestion.title = "Optimize High-Latency Communication";
            suggestion.description = "High-latency communication pattern detected. "
                                   "Consider optimizing data transfer mechanisms.";
            suggestion.potential_improvement = 12.0;
            suggestion.priority = 3;
            suggestion.implementation_steps = {
                "Use asynchronous communication to hide latency",
                "Implement data prefetching for predictable patterns",
                "Consider using different communication primitives",
                "Optimize data serialization/deserialization"
            };
            suggestions.push_back(suggestion);
            break; // Only suggest this once
        }
    }
    
    return suggestions;
}

double ParallelInsights::calculateCommunicationOverhead() {
    double total_comm_time = 0.0;
    double total_execution_time = 0.0;
    
    for (const auto& pattern : communication_patterns_) {
        total_comm_time += pattern.frequency * pattern.average_latency;
    }
    
    for (const auto& thread_metric : thread_metrics_) {
        total_execution_time += thread_metric.wall_time_ms;
    }
    
    if (total_execution_time > 0.0) {
        return (total_comm_time / total_execution_time) * 100.0;
    }
    
    return 3.0; // Default 3% communication overhead
}

void ParallelInsights::startThreadMonitoring() {
    // Start thread monitoring - would initialize profiling tools
}

void ParallelInsights::stopThreadMonitoring() {
    // Stop thread monitoring and collect final data
}

void ParallelInsights::collectThreadData() {
    // Collect thread performance data from system
}

void ParallelInsights::collectSynchronizationData() {
    // Collect synchronization performance data
}

void ParallelInsights::collectCommunicationData() {
    // Collect inter-thread communication data
}

double ParallelInsights::calculateAmdahlsLaw(double serial_fraction, int thread_count) {
    // Amdahl's Law: Speedup = 1 / (serial_fraction + (1 - serial_fraction) / n)
    return 1.0 / (serial_fraction + (1.0 - serial_fraction) / thread_count);
}

double ParallelInsights::calculateGustafsonLaw(double parallel_fraction, int thread_count) {
    // Gustafson's Law: Speedup = serial_fraction + parallel_fraction * n
    return (1.0 - parallel_fraction) + parallel_fraction * thread_count;
}

CommunicationPattern::Type ParallelInsights::detectCommunicationType(const std::vector<int>& participants) {
    if (participants.size() == 2) {
        return CommunicationPattern::POINT_TO_POINT;
    } else if (participants.size() > 2) {
        // Simple heuristic - could be more sophisticated
        return CommunicationPattern::BROADCAST;
    }
    return CommunicationPattern::POINT_TO_POINT;
}

std::string ParallelInsights::identifyLoadImbalanceCause(const std::vector<double>& workloads) {
    if (workloads.empty()) {
        return "No workload data available";
    }
    
    auto min_max = std::minmax_element(workloads.begin(), workloads.end());
    double range = *min_max.second - *min_max.first;
    double avg = std::accumulate(workloads.begin(), workloads.end(), 0.0) / workloads.size();
    
    if (range / avg > 0.5) {
        return "Significant workload variation - consider dynamic load balancing";
    } else if (range / avg > 0.3) {
        return "Moderate workload variation - fine-tune work distribution";
    } else {
        return "Well-balanced workload distribution";
    }
}

std::vector<std::string> ParallelInsights::generateLoadBalancingSuggestions(const LoadBalanceAnalysis& analysis) {
    std::vector<std::string> suggestions;
    
    if (analysis.load_balance_factor > 0.3) {
        suggestions.push_back("Implement dynamic work distribution (work stealing)");
        suggestions.push_back("Use smaller, more granular work units");
        suggestions.push_back("Consider round-robin or cyclic work assignment");
    } else if (analysis.load_balance_factor > 0.2) {
        suggestions.push_back("Fine-tune work partitioning strategy");
        suggestions.push_back("Monitor and adjust work distribution based on runtime performance");
    } else {
        suggestions.push_back("Current load balancing is acceptable");
    }
    
    return suggestions;
}

std::vector<std::string> ParallelInsights::generateSynchronizationSuggestions() {
    std::vector<std::string> suggestions;
    
    auto contention_map = analyzeLockContention();
    
    for (const auto& pair : contention_map) {
        if (pair.second > 10.0) {
            suggestions.push_back("High contention on " + pair.first + 
                                " - consider fine-grained locking or lock-free alternatives");
        }
    }
    
    if (suggestions.empty()) {
        suggestions.push_back("Synchronization appears to be well-optimized");
    }
    
    return suggestions;
}

} // namespace perf_analyzer