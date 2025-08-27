#include "optimization_engine.h"
#include <functional>

namespace perf_analyzer {

OptimizationEngine::OptimizationEngine() {
    initializeOptimizationPatterns();
}

OptimizationEngine::~OptimizationEngine() = default;

std::vector<OptimizationSuggestion> OptimizationEngine::generateSuggestions(
    const PerformanceMetrics& metrics, 
    const std::vector<Bottleneck>& bottlenecks) {
    
    std::vector<OptimizationSuggestion> suggestions;
    
    // Generate suggestions based on detected bottlenecks
    for (const auto& bottleneck : bottlenecks) {
        switch (bottleneck.type) {
            case Bottleneck::CPU_BOUND: {
                auto cpu_suggestions = suggestCPUOptimizations(metrics);
                suggestions.insert(suggestions.end(), cpu_suggestions.begin(), cpu_suggestions.end());
                break;
            }
            case Bottleneck::MEMORY_BOUND: {
                auto memory_suggestions = suggestMemoryOptimizations(metrics);
                suggestions.insert(suggestions.end(), memory_suggestions.begin(), memory_suggestions.end());
                break;
            }
            case Bottleneck::GPU_BOUND: {
                auto gpu_suggestions = suggestGPUOptimizations(metrics);
                suggestions.insert(suggestions.end(), gpu_suggestions.begin(), gpu_suggestions.end());
                break;
            }
            case Bottleneck::SYNCHRONIZATION: {
                auto sync_suggestions = suggestSynchronizationOptimizations(metrics);
                suggestions.insert(suggestions.end(), sync_suggestions.begin(), sync_suggestions.end());
                break;
            }
            case Bottleneck::LOAD_IMBALANCE: {
                auto balance_suggestions = suggestLoadBalancingStrategies(metrics);
                suggestions.insert(suggestions.end(), balance_suggestions.begin(), balance_suggestions.end());
                break;
            }
            case Bottleneck::COMMUNICATION: {
                auto parallel_suggestions = suggestParallelizationImprovements(metrics);
                suggestions.insert(suggestions.end(), parallel_suggestions.begin(), parallel_suggestions.end());
                break;
            }
        }
    }
    
    // Apply optimization patterns
    for (const auto& pattern : optimization_patterns_) {
        if (pattern.condition(metrics)) {
            suggestions.push_back(pattern.suggestion_generator(metrics));
        }
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestCPUOptimizations(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    if (shouldRecommendVectorization(metrics)) {
        suggestions.push_back(createSuggestion(
            "Enable SIMD/Vectorization",
            "Use SIMD instructions or vectorization to process multiple data elements simultaneously. "
            "Modern compilers can auto-vectorize loops, or you can use intrinsics for manual optimization.",
            15.0, // potential improvement
            2,    // priority
            {
                "Add compiler flags: -O3 -march=native -ftree-vectorize",
                "Restructure loops to be vectorization-friendly",
                "Consider using libraries like Eigen or Intel MKL",
                "Use OpenMP SIMD directives for parallel loops"
            }
        ));
    }
    
    suggestions.push_back(createSuggestion(
        "Compiler Optimizations",
        "Enable aggressive compiler optimizations to improve CPU performance through better code generation.",
        8.0,
        2,
        {
            "Use -O3 optimization level",
            "Add -march=native for target-specific optimizations",
            "Enable link-time optimization with -flto",
            "Consider profile-guided optimization (PGO)"
        }
    ));
    
    if (metrics.cpu_utilization > 90.0) {
        suggestions.push_back(createSuggestion(
            "Algorithm Optimization",
            "High CPU utilization suggests the need for algorithmic improvements to reduce computational complexity.",
            25.0,
            1,
            {
                "Analyze algorithm complexity and look for O(n²) → O(n log n) optimizations",
                "Implement caching for expensive computations",
                "Consider approximation algorithms for non-critical calculations",
                "Use more efficient data structures (e.g., hash tables vs linear search)"
            }
        ));
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestMemoryOptimizations(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    if (shouldRecommendCacheOptimization(metrics)) {
        suggestions.push_back(createSuggestion(
            "Cache-Friendly Memory Access",
            "Optimize memory access patterns to improve cache performance and reduce memory latency.",
            20.0,
            2,
            {
                "Ensure data structures fit in cache lines (64 bytes)",
                "Use row-major order for multi-dimensional arrays",
                "Implement loop tiling/blocking for better cache locality",
                "Consider data structure reorganization (AoS vs SoA)"
            }
        ));
    }
    
    suggestions.push_back(createSuggestion(
        "Memory Pool Allocation",
        "Use custom memory allocators to reduce allocation overhead and improve memory locality.",
        12.0,
        3,
        {
            "Implement object pools for frequently allocated/deallocated objects",
            "Use stack allocators for temporary objects",
            "Consider memory-mapped files for large datasets",
            "Reduce dynamic allocation in hot paths"
        }
    ));
    
    if (metrics.memory_usage_mb > 4096) {
        suggestions.push_back(createSuggestion(
            "Memory Usage Reduction",
            "High memory usage detected. Optimize data structures and algorithms to reduce memory footprint.",
            15.0,
            2,
            {
                "Use bit packing for boolean arrays",
                "Implement compression for large data structures",
                "Stream processing instead of loading all data in memory",
                "Use memory-efficient algorithms (e.g., external sorting)"
            }
        ));
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestGPUOptimizations(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    if (shouldRecommendGPUOffloading(metrics)) {
        suggestions.push_back(createSuggestion(
            "GPU Kernel Optimization",
            "Optimize GPU kernels for better performance and occupancy.",
            30.0,
            1,
            {
                "Optimize block and grid dimensions for target GPU",
                "Minimize divergent branching in GPU kernels",
                "Use shared memory to reduce global memory access",
                "Coalesce global memory accesses"
            }
        ));
    }
    
    if (metrics.gpu_utilization > 80.0) {
        suggestions.push_back(createSuggestion(
            "GPU Memory Bandwidth Optimization",
            "High GPU utilization suggests memory bandwidth optimization opportunities.",
            20.0,
            2,
            {
                "Use texture memory for read-only data with spatial locality",
                "Implement double buffering for data transfers",
                "Overlap computation with memory transfers",
                "Consider using unified memory for easier programming"
            }
        ));
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestParallelizationImprovements(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    if (metrics.communication_overhead > 10.0) {
        suggestions.push_back(createSuggestion(
            "Reduce Communication Overhead",
            "High communication overhead detected. Optimize inter-thread/process communication patterns.",
            18.0,
            2,
            {
                "Use shared memory instead of message passing where possible",
                "Batch communication operations to reduce frequency",
                "Implement lock-free data structures for producer-consumer patterns",
                "Consider using memory pools for inter-thread data exchange"
            }
        ));
    }
    
    if (shouldRecommendThreadPoolAdjustment(metrics)) {
        suggestions.push_back(createSuggestion(
            "Thread Pool Optimization",
            "Adjust thread pool size and work distribution for optimal performance.",
            12.0,
            3,
            {
                "Experiment with different thread pool sizes (typically 1-2x CPU cores)",
                "Implement work stealing for better load distribution",
                "Use thread-local storage to reduce contention",
                "Consider CPU affinity for performance-critical threads"
            }
        ));
    }
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestSynchronizationOptimizations(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    if (shouldRecommendLockOptimization(metrics)) {
        suggestions.push_back(createSuggestion(
            "Lock-Free Programming",
            "Reduce synchronization overhead by implementing lock-free algorithms and data structures.",
            25.0,
            1,
            {
                "Use atomic operations instead of locks for simple operations",
                "Implement lock-free queues for producer-consumer patterns",
                "Consider read-copy-update (RCU) for read-heavy workloads",
                "Use fine-grained locking instead of coarse-grained locks"
            }
        ));
    }
    
    suggestions.push_back(createSuggestion(
        "Synchronization Pattern Optimization",
        "Optimize synchronization patterns to reduce thread contention and waiting time.",
        15.0,
        2,
        {
            "Replace barriers with more efficient synchronization primitives",
            "Use condition variables instead of busy waiting",
            "Implement double-checked locking pattern where appropriate",
            "Consider using reader-writer locks for read-heavy workloads"
        }
    ));
    
    return suggestions;
}

std::vector<OptimizationSuggestion> OptimizationEngine::suggestLoadBalancingStrategies(const PerformanceMetrics& metrics) const {
    std::vector<OptimizationSuggestion> suggestions;
    
    suggestions.push_back(createSuggestion(
        "Dynamic Work Distribution",
        "Implement dynamic load balancing to ensure even work distribution across threads.",
        20.0,
        2,
        {
            "Implement work stealing queues",
            "Use dynamic scheduling with OpenMP (schedule=dynamic)",
            "Break large tasks into smaller, more granular work units",
            "Monitor thread utilization and redistribute work accordingly"
        }
    ));
    
    if (!metrics.thread_load_balance.empty()) {
        auto min_max = std::minmax_element(metrics.thread_load_balance.begin(), 
                                         metrics.thread_load_balance.end());
        double imbalance = (*min_max.second - *min_max.first) / *min_max.second * 100.0;
        
        if (imbalance > 30.0) {
            suggestions.push_back(createSuggestion(
                "Work Partitioning Strategy",
                "High load imbalance detected. Revise work partitioning strategy for better balance.",
                18.0,
                1,
                {
                    "Use round-robin or cyclic distribution instead of block distribution",
                    "Implement adaptive partitioning based on actual processing times",
                    "Consider using a centralized work queue with work stealing",
                    "Profile individual tasks to identify and split heavy computations"
                }
            ));
        }
    }
    
    return suggestions;
}

OptimizationSuggestion OptimizationEngine::createSuggestion(
    const std::string& title,
    const std::string& description,
    double potential_improvement,
    int priority,
    const std::vector<std::string>& steps) const {
    
    OptimizationSuggestion suggestion;
    suggestion.title = title;
    suggestion.description = description;
    suggestion.potential_improvement = potential_improvement;
    suggestion.priority = priority;
    suggestion.implementation_steps = steps;
    
    return suggestion;
}

bool OptimizationEngine::shouldRecommendVectorization(const PerformanceMetrics& metrics) const {
    return metrics.cpu_utilization > 70.0 && metrics.thread_count <= 4;
}

bool OptimizationEngine::shouldRecommendCacheOptimization(const PerformanceMetrics& metrics) const {
    return metrics.cpu_utilization < 60.0 && metrics.execution_time_ms > 500.0;
}

bool OptimizationEngine::shouldRecommendGPUOffloading(const PerformanceMetrics& metrics) const {
    return metrics.gpu_utilization < 50.0 && metrics.cpu_utilization > 80.0;
}

bool OptimizationEngine::shouldRecommendThreadPoolAdjustment(const PerformanceMetrics& metrics) const {
    return metrics.thread_count > 16 || (metrics.thread_count < 4 && metrics.cpu_utilization < 70.0);
}

bool OptimizationEngine::shouldRecommendLockOptimization(const PerformanceMetrics& metrics) const {
    return metrics.synchronization_overhead > 15.0;
}

void OptimizationEngine::initializeOptimizationPatterns() {
    // Initialize patterns that can be applied based on metrics
    optimization_patterns_.clear();
    
    // Example pattern: High thread count with low efficiency
    optimization_patterns_.push_back({
        "Thread Efficiency Pattern",
        [](const PerformanceMetrics& m) { 
            return m.thread_count > 8 && m.cpu_utilization < 60.0; 
        },
        [this](const PerformanceMetrics& m) {
            return createSuggestion(
                "Reduce Thread Contention",
                "High thread count with low CPU utilization suggests thread contention or over-parallelization.",
                15.0, 2,
                {
                    "Reduce the number of worker threads",
                    "Analyze critical sections for contention",
                    "Consider using thread pools instead of creating threads dynamically",
                    "Profile lock usage and optimize hot locks"
                }
            );
        }
    });
}

} // namespace perf_analyzer