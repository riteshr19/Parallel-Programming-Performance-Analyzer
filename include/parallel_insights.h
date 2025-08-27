#pragma once

#include "performance_analyzer.h"
#include <vector>
#include <map>
#include <chrono>

namespace perf_analyzer {

// Thread-specific metrics
struct ThreadMetrics {
    int thread_id;
    double cpu_time_ms;
    double wall_time_ms;
    double cpu_utilization;
    int context_switches;
    int cache_misses;
    double synchronization_wait_time;
    std::vector<std::string> synchronization_objects;
};

// Communication pattern analysis
struct CommunicationPattern {
    enum Type {
        POINT_TO_POINT,
        BROADCAST,
        SCATTER_GATHER,
        ALL_TO_ALL,
        REDUCTION
    };
    
    Type type;
    double frequency;
    double average_latency;
    double total_data_transferred;
    std::vector<int> participating_threads;
};

// Load balancing analysis
struct LoadBalanceAnalysis {
    double load_balance_factor;  // 0.0 (perfectly balanced) to 1.0 (completely unbalanced)
    std::vector<double> thread_workloads;
    std::vector<double> thread_idle_times;
    std::string imbalance_cause;
    std::vector<std::string> balancing_suggestions;
};

class ParallelInsights {
public:
    ParallelInsights();
    ~ParallelInsights();

    // Main analysis methods
    std::vector<ThreadMetrics> analyzeThreadPerformance();
    std::vector<CommunicationPattern> analyzeCommunicationPatterns();
    LoadBalanceAnalysis analyzeLoadBalance();
    
    // Synchronization analysis
    double calculateSynchronizationOverhead();
    std::vector<std::string> identifySynchronizationHotspots();
    std::map<std::string, double> analyzeLockContention();
    
    // Scalability analysis
    double predictScalability(int target_thread_count);
    std::vector<std::string> identifyScalabilityBottlenecks();
    
    // Memory access pattern analysis
    double analyzeCacheEfficiency();
    double analyzeMemoryAccessPatterns();
    std::vector<std::string> suggestMemoryOptimizations();
    
    // Thread efficiency analysis
    double calculateThreadEfficiency();
    std::vector<std::string> identifyIdleThreads();
    std::vector<std::string> suggestThreadOptimizations();
    
    // Communication optimization
    std::vector<OptimizationSuggestion> suggestCommunicationOptimizations();
    double calculateCommunicationOverhead();
    
    // Configuration
    void setAnalysisPeriod(double seconds) { analysis_period_ = seconds; }
    void enableDetailedThreadAnalysis(bool enable) { detailed_thread_analysis_ = enable; }
    void setSamplingRate(int rate_hz) { sampling_rate_hz_ = rate_hz; }

private:
    // Data collection methods
    void startThreadMonitoring();
    void stopThreadMonitoring();
    void collectThreadData();
    void collectSynchronizationData();
    void collectCommunicationData();
    
    // Analysis algorithms
    double calculateAmdahlsLaw(double serial_fraction, int thread_count);
    double calculateGustafsonLaw(double parallel_fraction, int thread_count);
    
    // Pattern detection
    CommunicationPattern::Type detectCommunicationType(const std::vector<int>& participants);
    std::string identifyLoadImbalanceCause(const std::vector<double>& workloads);
    
    // Optimization suggestions
    std::vector<std::string> generateLoadBalancingSuggestions(const LoadBalanceAnalysis& analysis);
    std::vector<std::string> generateSynchronizationSuggestions();
    
    // Configuration
    double analysis_period_;
    bool detailed_thread_analysis_;
    int sampling_rate_hz_;
    
    // Collected data
    std::vector<ThreadMetrics> thread_metrics_;
    std::vector<CommunicationPattern> communication_patterns_;
    LoadBalanceAnalysis load_balance_analysis_;
    std::map<std::string, double> synchronization_overhead_;
};

} // namespace perf_analyzer