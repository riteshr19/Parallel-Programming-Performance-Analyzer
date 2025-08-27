#include "bottleneck_detector.h"
#include <algorithm>
#include <cmath>

namespace perf_analyzer {

BottleneckDetector::BottleneckDetector() 
    : cpu_threshold_(80.0)
    , memory_threshold_(85.0)
    , gpu_threshold_(90.0)
    , sync_threshold_(10.0)
    , load_balance_threshold_(20.0) {
}

BottleneckDetector::~BottleneckDetector() = default;

std::vector<Bottleneck> BottleneckDetector::detectBottlenecks(const PerformanceMetrics& metrics) {
    std::vector<Bottleneck> bottlenecks;
    
    // Check for CPU bottlenecks
    if (isCPUBound(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::CPU_BOUND;
        bottleneck.severity_score = calculateCPUSeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::CPU_BOUND, metrics);
        bottleneck.location = "CPU cores";
        bottlenecks.push_back(bottleneck);
    }
    
    // Check for memory bottlenecks
    if (isMemoryBound(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::MEMORY_BOUND;
        bottleneck.severity_score = calculateMemorySeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::MEMORY_BOUND, metrics);
        bottleneck.location = "System memory";
        bottlenecks.push_back(bottleneck);
    }
    
    // Check for GPU bottlenecks
    if (isGPUBound(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::GPU_BOUND;
        bottleneck.severity_score = calculateGPUSeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::GPU_BOUND, metrics);
        bottleneck.location = "GPU device";
        bottlenecks.push_back(bottleneck);
    }
    
    // Check for synchronization issues
    if (hasSynchronizationIssues(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::SYNCHRONIZATION;
        bottleneck.severity_score = calculateSyncSeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::SYNCHRONIZATION, metrics);
        bottleneck.location = "Thread synchronization";
        bottlenecks.push_back(bottleneck);
    }
    
    // Check for communication bottlenecks
    if (hasCommunicationBottleneck(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::COMMUNICATION;
        bottleneck.severity_score = calculateCommSeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::COMMUNICATION, metrics);
        bottleneck.location = "Inter-thread communication";
        bottlenecks.push_back(bottleneck);
    }
    
    // Check for load imbalance
    if (hasLoadImbalance(metrics)) {
        Bottleneck bottleneck;
        bottleneck.type = Bottleneck::LOAD_IMBALANCE;
        bottleneck.severity_score = calculateLoadImbalanceSeverity(metrics);
        bottleneck.description = generateBottleneckDescription(Bottleneck::LOAD_IMBALANCE, metrics);
        bottleneck.location = "Thread workload distribution";
        bottlenecks.push_back(bottleneck);
    }
    
    return bottlenecks;
}

bool BottleneckDetector::isCPUBound(const PerformanceMetrics& metrics) const {
    return metrics.cpu_utilization > cpu_threshold_;
}

bool BottleneckDetector::isMemoryBound(const PerformanceMetrics& metrics) const {
    // Consider memory bound if high memory usage or if CPU utilization is low 
    // but execution time is high (indicating potential memory bottleneck)
    double memory_usage_percent = (metrics.memory_usage_mb / 8192.0) * 100.0; // Assume 8GB system
    return memory_usage_percent > memory_threshold_ || 
           (metrics.cpu_utilization < 50.0 && metrics.execution_time_ms > 1000.0);
}

bool BottleneckDetector::isGPUBound(const PerformanceMetrics& metrics) const {
    return metrics.gpu_utilization > gpu_threshold_;
}

bool BottleneckDetector::hasSynchronizationIssues(const PerformanceMetrics& metrics) const {
    return metrics.synchronization_overhead > sync_threshold_;
}

bool BottleneckDetector::hasCommunicationBottleneck(const PerformanceMetrics& metrics) const {
    return metrics.communication_overhead > 5.0; // 5% threshold for communication overhead
}

bool BottleneckDetector::hasLoadImbalance(const PerformanceMetrics& metrics) const {
    if (metrics.thread_load_balance.size() < 2) {
        return false;
    }
    
    auto min_max = std::minmax_element(metrics.thread_load_balance.begin(), 
                                     metrics.thread_load_balance.end());
    double load_variance = (*min_max.second - *min_max.first) / *min_max.second * 100.0;
    
    return load_variance > load_balance_threshold_;
}

double BottleneckDetector::calculateCPUSeverity(const PerformanceMetrics& metrics) const {
    // Severity increases non-linearly with CPU utilization
    double normalized = (metrics.cpu_utilization - cpu_threshold_) / (100.0 - cpu_threshold_);
    return std::min(1.0, std::max(0.0, normalized * normalized));
}

double BottleneckDetector::calculateMemorySeverity(const PerformanceMetrics& metrics) const {
    double memory_usage_percent = (metrics.memory_usage_mb / 8192.0) * 100.0;
    double normalized = (memory_usage_percent - memory_threshold_) / (100.0 - memory_threshold_);
    
    // Also consider if low CPU usage indicates memory bottleneck
    if (metrics.cpu_utilization < 50.0 && metrics.execution_time_ms > 1000.0) {
        normalized = std::max(normalized, 0.6);
    }
    
    return std::min(1.0, std::max(0.0, normalized));
}

double BottleneckDetector::calculateGPUSeverity(const PerformanceMetrics& metrics) const {
    double normalized = (metrics.gpu_utilization - gpu_threshold_) / (100.0 - gpu_threshold_);
    return std::min(1.0, std::max(0.0, normalized));
}

double BottleneckDetector::calculateSyncSeverity(const PerformanceMetrics& metrics) const {
    // Synchronization overhead becomes more severe exponentially
    return std::min(1.0, metrics.synchronization_overhead / 50.0);
}

double BottleneckDetector::calculateCommSeverity(const PerformanceMetrics& metrics) const {
    return std::min(1.0, metrics.communication_overhead / 30.0);
}

double BottleneckDetector::calculateLoadImbalanceSeverity(const PerformanceMetrics& metrics) const {
    if (metrics.thread_load_balance.size() < 2) {
        return 0.0;
    }
    
    auto min_max = std::minmax_element(metrics.thread_load_balance.begin(), 
                                     metrics.thread_load_balance.end());
    double load_variance = (*min_max.second - *min_max.first) / *min_max.second;
    
    return std::min(1.0, load_variance);
}

std::string BottleneckDetector::generateBottleneckDescription(Bottleneck::Type type, 
                                                            const PerformanceMetrics& metrics) const {
    switch (type) {
        case Bottleneck::CPU_BOUND:
            return "High CPU utilization (" + std::to_string(static_cast<int>(metrics.cpu_utilization)) + 
                   "%) indicates CPU-bound performance bottleneck. Consider optimizing algorithms, "
                   "enabling compiler optimizations, or reducing computational complexity.";
        
        case Bottleneck::MEMORY_BOUND:
            return "Memory subsystem appears to be limiting performance. High memory usage (" + 
                   std::to_string(static_cast<int>(metrics.memory_usage_mb)) + 
                   " MB) or poor cache utilization detected. Consider memory access pattern optimization.";
        
        case Bottleneck::GPU_BOUND:
            return "GPU utilization is very high (" + std::to_string(static_cast<int>(metrics.gpu_utilization)) + 
                   "%), indicating GPU compute or memory bandwidth saturation. Consider kernel optimization "
                   "or workload distribution.";
        
        case Bottleneck::SYNCHRONIZATION:
            return "Significant synchronization overhead (" + 
                   std::to_string(static_cast<int>(metrics.synchronization_overhead)) + 
                   "%) detected. Threads are spending too much time waiting for locks or barriers. "
                   "Consider reducing critical sections or using lock-free algorithms.";
        
        case Bottleneck::COMMUNICATION:
            return "High communication overhead (" + 
                   std::to_string(static_cast<int>(metrics.communication_overhead)) + 
                   "%) between threads or processes. Consider reducing data movement, using shared memory, "
                   "or optimizing communication patterns.";
        
        case Bottleneck::LOAD_IMBALANCE:
            if (!metrics.thread_load_balance.empty()) {
                auto min_max = std::minmax_element(metrics.thread_load_balance.begin(), 
                                                 metrics.thread_load_balance.end());
                return "Load imbalance detected across threads. Workload varies from " + 
                       std::to_string(static_cast<int>(*min_max.first)) + "% to " + 
                       std::to_string(static_cast<int>(*min_max.second)) + 
                       "%. Consider work stealing, dynamic scheduling, or better work partitioning.";
            }
            return "Load imbalance detected across threads. Consider better work distribution strategies.";
        
        default:
            return "Unknown bottleneck type detected.";
    }
}

} // namespace perf_analyzer