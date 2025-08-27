#pragma once

#include "performance_analyzer.h"
#include <vector>
#include <map>

namespace perf_analyzer {

// GPU-specific metrics
struct GPUMetrics {
    int device_count;
    std::vector<std::string> device_names;
    std::vector<double> gpu_utilization;
    std::vector<double> memory_utilization;
    std::vector<double> memory_bandwidth_usage;
    std::vector<double> compute_capability;
    std::vector<int> active_kernels;
    std::vector<double> kernel_execution_times;
    double total_gpu_memory_gb;
    double used_gpu_memory_gb;
    bool cuda_available;
    bool opencl_available;
};

// Kernel performance data
struct KernelProfile {
    std::string name;
    double execution_time_ms;
    double occupancy;
    int grid_size;
    int block_size;
    double memory_throughput;
    double compute_throughput;
    std::vector<std::string> optimization_hints;
};

class GPUAnalyzer {
public:
    GPUAnalyzer();
    ~GPUAnalyzer();

    // Main GPU analysis methods
    bool initializeGPUProfiling();
    GPUMetrics analyzeGPUPerformance();
    std::vector<KernelProfile> profileKernels();
    
    // GPU capability detection
    bool isCUDAAvailable() const;
    bool isOpenCLAvailable() const;
    int getDeviceCount() const;
    std::vector<std::string> getDeviceProperties() const;
    
    // Performance analysis
    double calculateGPUEfficiency(const GPUMetrics& metrics) const;
    bool detectMemoryBottlenecks(const GPUMetrics& metrics) const;
    bool detectComputeBottlenecks(const GPUMetrics& metrics) const;
    
    // GPU-specific optimization suggestions
    std::vector<OptimizationSuggestion> suggestGPUOptimizations(const GPUMetrics& metrics) const;
    std::vector<std::string> analyzeKernelLaunchConfiguration(const KernelProfile& profile) const;
    
    // Memory analysis
    double analyzeMemoryCoalescing() const;
    double analyzeMemoryBandwidthUtilization() const;
    std::vector<std::string> suggestMemoryOptimizations() const;
    
    // Configuration
    void setProfilingDuration(double seconds) { profiling_duration_ = seconds; }
    void enableDetailedProfiling(bool enable) { detailed_profiling_ = enable; }

private:
    // GPU detection and initialization
    bool initializeCUDA();
    bool initializeOpenCL();
    
    // Profiling methods
    void startProfiling();
    void stopProfiling();
    std::vector<KernelProfile> collectKernelData();
    
    // Analysis helpers
    double calculateOccupancy(const KernelProfile& profile) const;
    double calculateMemoryThroughput(const KernelProfile& profile) const;
    double calculateComputeThroughput(const KernelProfile& profile) const;
    
    // Optimization pattern matching
    std::vector<std::string> matchOptimizationPatterns(const KernelProfile& profile) const;
    
    // Configuration
    double profiling_duration_;
    bool detailed_profiling_;
    bool cuda_initialized_;
    bool opencl_initialized_;
    
    // GPU state
    GPUMetrics current_metrics_;
    std::vector<KernelProfile> kernel_profiles_;
};

} // namespace perf_analyzer