#pragma once

#include "performance_analyzer.h"
#include <vector>
#include <memory>

namespace perf_analyzer {

class BottleneckDetector {
public:
    BottleneckDetector();
    ~BottleneckDetector();

    // Main detection methods
    std::vector<Bottleneck> detectBottlenecks(const PerformanceMetrics& metrics);
    
    // Specific bottleneck detection algorithms
    bool isCPUBound(const PerformanceMetrics& metrics) const;
    bool isMemoryBound(const PerformanceMetrics& metrics) const;
    bool isGPUBound(const PerformanceMetrics& metrics) const;
    bool hasSynchronizationIssues(const PerformanceMetrics& metrics) const;
    bool hasCommunicationBottleneck(const PerformanceMetrics& metrics) const;
    bool hasLoadImbalance(const PerformanceMetrics& metrics) const;
    
    // Configuration
    void setCPUThreshold(double threshold) { cpu_threshold_ = threshold; }
    void setMemoryThreshold(double threshold) { memory_threshold_ = threshold; }
    void setGPUThreshold(double threshold) { gpu_threshold_ = threshold; }
    void setSyncThreshold(double threshold) { sync_threshold_ = threshold; }
    void setLoadBalanceThreshold(double threshold) { load_balance_threshold_ = threshold; }

private:
    // Thresholds for bottleneck detection
    double cpu_threshold_;
    double memory_threshold_;
    double gpu_threshold_;
    double sync_threshold_;
    double load_balance_threshold_;
    
    // Analysis methods
    double calculateCPUSeverity(const PerformanceMetrics& metrics) const;
    double calculateMemorySeverity(const PerformanceMetrics& metrics) const;
    double calculateGPUSeverity(const PerformanceMetrics& metrics) const;
    double calculateSyncSeverity(const PerformanceMetrics& metrics) const;
    double calculateCommSeverity(const PerformanceMetrics& metrics) const;
    double calculateLoadImbalanceSeverity(const PerformanceMetrics& metrics) const;
    
    std::string generateBottleneckDescription(Bottleneck::Type type, 
                                            const PerformanceMetrics& metrics) const;
};

} // namespace perf_analyzer