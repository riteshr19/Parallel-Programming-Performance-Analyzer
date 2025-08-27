#include "metrics_collector.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/sysinfo.h>
#include <chrono>
#include <thread>

namespace perf_analyzer {

MetricsCollector::MetricsCollector()
    : collecting_(false)
    , sampling_rate_hz_(100)
    , target_pid_(-1)
    , perf_counters_enabled_(false)
    , gpu_monitoring_enabled_(false)
    , collection_duration_(10.0)
    , perf_counters_initialized_(false)
    , gpu_monitoring_initialized_(false)
    , gpu_context_(nullptr) {
}

MetricsCollector::~MetricsCollector() {
    if (collecting_.load()) {
        stopCollection();
    }
    cleanupPerformanceCounters();
}

bool MetricsCollector::startCollection() {
    if (collecting_.load()) {
        return false; // Already collecting
    }
    
    // Initialize performance counters if enabled
    if (perf_counters_enabled_) {
        initializePerformanceCounters();
    }
    
    // Initialize GPU monitoring if enabled
    if (gpu_monitoring_enabled_) {
        initializeGPUMonitoring();
    }
    
    // Clear previous data
    metrics_history_.clear();
    system_metrics_history_.clear();
    process_metrics_history_.clear();
    
    // Start collection thread
    collecting_.store(true);
    collection_thread_ = std::thread(&MetricsCollector::collectingThread, this);
    
    return true;
}

void MetricsCollector::stopCollection() {
    if (!collecting_.load()) {
        return;
    }
    
    collecting_.store(false);
    
    if (collection_thread_.joinable()) {
        collection_thread_.join();
    }
}

PerformanceMetrics MetricsCollector::collectCurrentMetrics() {
    PerformanceMetrics metrics;
    
    // Initialize with default values
    metrics.execution_time_ms = 0.0;
    metrics.cpu_utilization = 0.0;
    metrics.memory_usage_mb = 0.0;
    metrics.gpu_utilization = 0.0;
    metrics.gpu_memory_usage_mb = 0.0;
    metrics.thread_count = 0;
    metrics.synchronization_overhead = 0.0;
    metrics.communication_overhead = 0.0;
    
    // Collect system metrics
    SystemMetrics sys_metrics = collectSystemMetrics();
    metrics.cpu_utilization = sys_metrics.cpu_usage_percent;
    metrics.memory_usage_mb = sys_metrics.memory_usage_mb;
    
    // Collect process-specific metrics if target PID is set
    if (target_pid_ > 0) {
        ProcessMetrics proc_metrics = collectProcessMetrics(target_pid_);
        metrics.cpu_utilization = proc_metrics.cpu_percent;
        metrics.memory_usage_mb = proc_metrics.memory_mb;
        metrics.thread_count = proc_metrics.thread_count;
        metrics.execution_time_ms = proc_metrics.cpu_time_user + proc_metrics.cpu_time_system;
    } else {
        // Use system-wide metrics
        metrics.thread_count = sys_metrics.active_threads;
    }
    
    // Collect GPU metrics if enabled
    if (gpu_monitoring_enabled_) {
        collectGPUMetrics(metrics);
    }
    
    // Calculate derived metrics
    metrics.synchronization_overhead = 5.0; // Simulated 5% sync overhead
    metrics.communication_overhead = 3.0;   // Simulated 3% comm overhead
    
    return metrics;
}

SystemMetrics MetricsCollector::collectSystemMetrics() {
    return readSystemMetrics();
}

ProcessMetrics MetricsCollector::collectProcessMetrics(int pid) {
    return readProcessMetrics(pid);
}

PerformanceCounters MetricsCollector::collectPerformanceCounters() {
    PerformanceCounters counters;
    
    if (perf_counters_initialized_ && target_pid_ > 0) {
        counters = readPerformanceCounters(target_pid_);
    } else {
        // Return simulated values
        counters.instructions_retired = 1000000000ULL;
        counters.cycles = 3000000000ULL;
        counters.cache_references = 50000000ULL;
        counters.cache_misses = 5000000ULL;
        counters.branch_instructions = 200000000ULL;
        counters.branch_misses = 10000000ULL;
        counters.page_faults = 1000ULL;
        counters.context_switches = 500ULL;
    }
    
    return counters;
}

void MetricsCollector::startContinuousMonitoring(int pid, double duration_seconds) {
    target_pid_ = pid;
    collection_duration_ = duration_seconds;
    
    startCollection();
    
    // Wait for collection to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(
        static_cast<int>(duration_seconds * 1000)));
    
    stopCollection();
}

std::vector<PerformanceMetrics> MetricsCollector::getContinuousMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_history_;
}

void MetricsCollector::collectGPUMetrics(PerformanceMetrics& metrics) {
    if (!gpu_monitoring_initialized_) {
        return;
    }
    
    // Simulate GPU metrics collection
    // In a real implementation, this would interface with NVIDIA ML or similar
    metrics.gpu_utilization = 65.0; // 65% GPU utilization
    metrics.gpu_memory_usage_mb = 2048.0; // 2GB GPU memory usage
}

double MetricsCollector::calculateCPUUtilization(const std::vector<ProcessMetrics>& samples) {
    if (samples.size() < 2) {
        return 0.0;
    }
    
    // Calculate CPU utilization from samples
    double total_cpu_time = 0.0;
    double total_wall_time = 0.0;
    
    for (size_t i = 1; i < samples.size(); ++i) {
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            samples[i].timestamp - samples[i-1].timestamp).count();
        
        double cpu_diff = (samples[i].cpu_time_user + samples[i].cpu_time_system) -
                         (samples[i-1].cpu_time_user + samples[i-1].cpu_time_system);
        
        total_cpu_time += cpu_diff;
        total_wall_time += time_diff;
    }
    
    if (total_wall_time > 0.0) {
        return (total_cpu_time / total_wall_time) * 100.0;
    }
    
    return 0.0;
}

double MetricsCollector::calculateMemoryEfficiency(const std::vector<ProcessMetrics>& samples) {
    if (samples.empty()) {
        return 0.0;
    }
    
    // Simple memory efficiency based on memory usage stability
    double avg_memory = 0.0;
    for (const auto& sample : samples) {
        avg_memory += sample.memory_mb;
    }
    avg_memory /= samples.size();
    
    // Lower variance indicates better memory management
    double variance = 0.0;
    for (const auto& sample : samples) {
        variance += (sample.memory_mb - avg_memory) * (sample.memory_mb - avg_memory);
    }
    variance /= samples.size();
    
    // Return efficiency score (0-100%)
    return std::max(0.0, 100.0 - (variance / avg_memory) * 100.0);
}

std::vector<double> MetricsCollector::calculateThreadLoadBalance() {
    // Simulate thread load balance calculation
    std::vector<double> load_balance = {85.0, 92.0, 78.0, 88.0, 95.0, 82.0, 90.0, 85.0};
    return load_balance;
}

void MetricsCollector::collectingThread() {
    auto interval = std::chrono::milliseconds(1000 / sampling_rate_hz_);
    
    while (collecting_.load()) {
        auto start_time = std::chrono::steady_clock::now();
        
        // Collect metrics
        PerformanceMetrics metrics = collectCurrentMetrics();
        SystemMetrics sys_metrics = collectSystemMetrics();
        
        // Store metrics
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_history_.push_back(metrics);
            system_metrics_history_.push_back(sys_metrics);
            
            if (target_pid_ > 0) {
                ProcessMetrics proc_metrics = collectProcessMetrics(target_pid_);
                process_metrics_history_.push_back(proc_metrics);
            }
        }
        
        // Sleep until next sampling interval
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = end_time - start_time;
        
        if (elapsed < interval) {
            std::this_thread::sleep_for(interval - elapsed);
        }
    }
}

SystemMetrics MetricsCollector::readSystemMetrics() {
    SystemMetrics metrics;
    
    // Read CPU usage
    metrics.cpu_usage_percent = getCPUUsage();
    
    // Read memory usage
    metrics.memory_usage_mb = getMemoryUsage();
    
    // Read system info
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        metrics.memory_usage_percent = ((double)(info.totalram - info.freeram) / info.totalram) * 100.0;
        metrics.swap_usage_mb = (double)info.totalswap / (1024 * 1024);
        metrics.active_processes = info.procs;
    }
    
    // Simulate other metrics
    metrics.disk_io_read_mbps = 50.0;
    metrics.disk_io_write_mbps = 25.0;
    metrics.network_io_recv_mbps = 10.0;
    metrics.network_io_send_mbps = 5.0;
    metrics.active_threads = 50;
    
    return metrics;
}

ProcessMetrics MetricsCollector::readProcessMetrics(int pid) {
    ProcessMetrics metrics;
    
    metrics.pid = pid;
    metrics.timestamp = std::chrono::system_clock::now();
    
    // Read from /proc/pid/stat
    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(stat_path);
    
    if (stat_file.is_open()) {
        std::string line;
        std::getline(stat_file, line);
        std::istringstream iss(line);
        
        std::vector<std::string> fields;
        std::string field;
        while (iss >> field) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 24) {
            // Extract CPU times (fields 13, 14)
            metrics.cpu_time_user = std::stod(fields[13]) / sysconf(_SC_CLK_TCK) * 1000.0; // ms
            metrics.cpu_time_system = std::stod(fields[14]) / sysconf(_SC_CLK_TCK) * 1000.0; // ms
            
            // Extract memory usage (field 23 - virtual memory size)
            metrics.memory_mb = std::stod(fields[22]) / (1024 * 1024); // Convert to MB
            
            // Extract thread count (field 19)
            metrics.thread_count = std::stoi(fields[19]);
        }
    }
    
    // Read additional metrics from /proc/pid/status
    std::string status_path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream status_file(status_path);
    
    if (status_file.is_open()) {
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                // Physical memory usage
                std::istringstream iss(line);
                std::string label, value, unit;
                iss >> label >> value >> unit;
                metrics.memory_mb = std::stod(value) / 1024.0; // Convert kB to MB
                break;
            }
        }
    }
    
    // Calculate CPU percentage (simplified)
    static std::map<int, double> last_cpu_time;
    static std::map<int, std::chrono::system_clock::time_point> last_time;
    
    double current_cpu_time = metrics.cpu_time_user + metrics.cpu_time_system;
    auto current_time = metrics.timestamp;
    
    if (last_cpu_time.find(pid) != last_cpu_time.end()) {
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - last_time[pid]).count();
        double cpu_diff = current_cpu_time - last_cpu_time[pid];
        
        if (time_diff > 0) {
            metrics.cpu_percent = (cpu_diff / time_diff) * 100.0;
        }
    }
    
    last_cpu_time[pid] = current_cpu_time;
    last_time[pid] = current_time;
    
    // Simulate other metrics
    metrics.memory_percent = (metrics.memory_mb / 8192.0) * 100.0; // Assume 8GB system
    metrics.file_descriptors = 25;
    
    return metrics;
}

PerformanceCounters MetricsCollector::readPerformanceCounters(int pid) {
    PerformanceCounters counters;
    
    // In a real implementation, this would read from perf_event_open()
    // For now, return simulated values
    counters.instructions_retired = 1000000000ULL + (pid * 1000000ULL);
    counters.cycles = 3000000000ULL + (pid * 3000000ULL);
    counters.cache_references = 50000000ULL + (pid * 50000ULL);
    counters.cache_misses = 5000000ULL + (pid * 5000ULL);
    counters.branch_instructions = 200000000ULL + (pid * 200000ULL);
    counters.branch_misses = 10000000ULL + (pid * 10000ULL);
    counters.page_faults = 1000ULL + (pid * 10ULL);
    counters.context_switches = 500ULL + (pid * 5ULL);
    
    return counters;
}

bool MetricsCollector::initializePerformanceCounters() {
    // In a real implementation, this would:
    // 1. Open performance event file descriptors using perf_event_open()
    // 2. Configure hardware performance counters
    // 3. Start monitoring
    
    perf_counters_initialized_ = true;
    return true;
}

void MetricsCollector::cleanupPerformanceCounters() {
    if (!perf_counters_initialized_) {
        return;
    }
    
    // Close performance counter file descriptors
    for (auto& pair : perf_counter_fds_) {
        if (pair.second >= 0) {
            close(pair.second);
        }
    }
    
    perf_counter_fds_.clear();
    perf_counters_initialized_ = false;
}

double MetricsCollector::getCPUUsage() {
    // Read from /proc/stat
    static unsigned long long last_idle = 0, last_total = 0;
    
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) {
        return 0.0;
    }
    
    std::string line;
    std::getline(stat_file, line);
    
    std::istringstream iss(line);
    std::string cpu_label;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    
    iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    
    unsigned long long current_idle = idle + iowait;
    unsigned long long current_total = user + nice + system + idle + iowait + irq + softirq + steal;
    
    unsigned long long idle_diff = current_idle - last_idle;
    unsigned long long total_diff = current_total - last_total;
    
    last_idle = current_idle;
    last_total = current_total;
    
    if (total_diff == 0) {
        return 0.0;
    }
    
    return 100.0 * (1.0 - (double)idle_diff / total_diff);
}

double MetricsCollector::getMemoryUsage() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) {
        return 0.0;
    }
    
    unsigned long long total_memory = 0, free_memory = 0, available_memory = 0;
    std::string line;
    
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string label, value, unit;
        iss >> label >> value >> unit;
        
        if (label == "MemTotal:") {
            total_memory = std::stoull(value);
        } else if (label == "MemFree:") {
            free_memory = std::stoull(value);
        } else if (label == "MemAvailable:") {
            available_memory = std::stoull(value);
        }
    }
    
    if (total_memory > 0) {
        unsigned long long used_memory = total_memory - (available_memory > 0 ? available_memory : free_memory);
        return (double)used_memory / 1024.0; // Convert kB to MB
    }
    
    return 0.0;
}

std::vector<int> MetricsCollector::getProcessThreads(int pid) {
    std::vector<int> thread_ids;
    
    std::string task_dir = "/proc/" + std::to_string(pid) + "/task";
    
    // In a real implementation, would read directory contents
    // For simulation, return some thread IDs
    for (int i = 0; i < 4; ++i) {
        thread_ids.push_back(pid + i);
    }
    
    return thread_ids;
}

bool MetricsCollector::initializeGPUMonitoring() {
    // In a real implementation, this would:
    // 1. Initialize NVIDIA ML library or similar
    // 2. Query available GPUs
    // 3. Set up monitoring
    
    gpu_monitoring_initialized_ = true;
    return true;
}

void MetricsCollector::updateGPUMetrics(PerformanceMetrics& metrics) {
    if (!gpu_monitoring_initialized_) {
        return;
    }
    
    // Simulate GPU metrics update
    metrics.gpu_utilization = 65.0;
    metrics.gpu_memory_usage_mb = 2048.0;
}

} // namespace perf_analyzer