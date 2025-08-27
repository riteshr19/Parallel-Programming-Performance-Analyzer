#pragma once

#include "performance_analyzer.h"
#include <chrono>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

namespace perf_analyzer {

// System resource metrics
struct SystemMetrics {
    double cpu_usage_percent;
    double memory_usage_mb;
    double memory_usage_percent;
    double swap_usage_mb;
    double disk_io_read_mbps;
    double disk_io_write_mbps;
    double network_io_recv_mbps;
    double network_io_send_mbps;
    int active_processes;
    int active_threads;
};

// Process-specific metrics
struct ProcessMetrics {
    int pid;
    double cpu_percent;
    double memory_mb;
    double memory_percent;
    int thread_count;
    int file_descriptors;
    double cpu_time_user;
    double cpu_time_system;
    std::chrono::system_clock::time_point timestamp;
};

// Performance counters
struct PerformanceCounters {
    uint64_t instructions_retired;
    uint64_t cycles;
    uint64_t cache_references;
    uint64_t cache_misses;
    uint64_t branch_instructions;
    uint64_t branch_misses;
    uint64_t page_faults;
    uint64_t context_switches;
};

class MetricsCollector {
public:
    MetricsCollector();
    ~MetricsCollector();

    // Collection control
    bool startCollection();
    void stopCollection();
    bool isCollecting() const { return collecting_.load(); }
    
    // Main metrics collection
    PerformanceMetrics collectCurrentMetrics();
    SystemMetrics collectSystemMetrics();
    ProcessMetrics collectProcessMetrics(int pid);
    PerformanceCounters collectPerformanceCounters();
    
    // Continuous monitoring
    void startContinuousMonitoring(int pid, double duration_seconds);
    std::vector<PerformanceMetrics> getContinuousMetrics() const;
    
    // GPU metrics collection (if available)
    void collectGPUMetrics(PerformanceMetrics& metrics);
    
    // Configuration
    void setSamplingRate(int rate_hz) { sampling_rate_hz_ = rate_hz; }
    void setTargetProcess(int pid) { target_pid_ = pid; }
    void enablePerformanceCounters(bool enable) { perf_counters_enabled_ = enable; }
    void enableGPUMonitoring(bool enable) { gpu_monitoring_enabled_ = enable; }
    
    // Utility methods
    double calculateCPUUtilization(const std::vector<ProcessMetrics>& samples);
    double calculateMemoryEfficiency(const std::vector<ProcessMetrics>& samples);
    std::vector<double> calculateThreadLoadBalance();

private:
    // Collection methods
    void collectingThread();
    SystemMetrics readSystemMetrics();
    ProcessMetrics readProcessMetrics(int pid);
    PerformanceCounters readPerformanceCounters(int pid);
    
    // System interface methods
    bool initializePerformanceCounters();
    void cleanupPerformanceCounters();
    double getCPUUsage();
    double getMemoryUsage();
    std::vector<int> getProcessThreads(int pid);
    
    // GPU monitoring (if available)
    bool initializeGPUMonitoring();
    void updateGPUMetrics(PerformanceMetrics& metrics);
    
    // Thread synchronization and control
    std::atomic<bool> collecting_;
    std::thread collection_thread_;
    mutable std::mutex metrics_mutex_;
    
    // Configuration
    int sampling_rate_hz_;
    int target_pid_;
    bool perf_counters_enabled_;
    bool gpu_monitoring_enabled_;
    double collection_duration_;
    
    // Collected data
    std::vector<PerformanceMetrics> metrics_history_;
    std::vector<SystemMetrics> system_metrics_history_;
    std::vector<ProcessMetrics> process_metrics_history_;
    
    // Performance counters state
    bool perf_counters_initialized_;
    std::map<std::string, int> perf_counter_fds_;
    
    // GPU monitoring state
    bool gpu_monitoring_initialized_;
    void* gpu_context_;  // Opaque pointer for GPU context
};

} // namespace perf_analyzer