#pragma once

#include "performance_analyzer.h"
#include <vector>
#include <functional>

namespace perf_analyzer {

class OptimizationEngine {
public:
    OptimizationEngine();
    ~OptimizationEngine();

    // Main optimization suggestion generation
    std::vector<OptimizationSuggestion> generateSuggestions(
        const PerformanceMetrics& metrics, 
        const std::vector<Bottleneck>& bottlenecks);
    
    // Specific optimization strategies
    std::vector<OptimizationSuggestion> suggestCPUOptimizations(const PerformanceMetrics& metrics) const;
    std::vector<OptimizationSuggestion> suggestMemoryOptimizations(const PerformanceMetrics& metrics) const;
    std::vector<OptimizationSuggestion> suggestGPUOptimizations(const PerformanceMetrics& metrics) const;
    std::vector<OptimizationSuggestion> suggestParallelizationImprovements(const PerformanceMetrics& metrics) const;
    std::vector<OptimizationSuggestion> suggestSynchronizationOptimizations(const PerformanceMetrics& metrics) const;
    std::vector<OptimizationSuggestion> suggestLoadBalancingStrategies(const PerformanceMetrics& metrics) const;

private:
    // Helper methods for creating suggestions
    OptimizationSuggestion createSuggestion(
        const std::string& title,
        const std::string& description,
        double potential_improvement,
        int priority,
        const std::vector<std::string>& steps) const;
    
    // Analysis helpers
    bool shouldRecommendVectorization(const PerformanceMetrics& metrics) const;
    bool shouldRecommendCacheOptimization(const PerformanceMetrics& metrics) const;
    bool shouldRecommendGPUOffloading(const PerformanceMetrics& metrics) const;
    bool shouldRecommendThreadPoolAdjustment(const PerformanceMetrics& metrics) const;
    bool shouldRecommendLockOptimization(const PerformanceMetrics& metrics) const;
    
    // Optimization knowledge base
    void initializeOptimizationPatterns();
    
    struct OptimizationPattern {
        std::string name;
        std::function<bool(const PerformanceMetrics&)> condition;
        std::function<OptimizationSuggestion(const PerformanceMetrics&)> suggestion_generator;
    };
    
    std::vector<OptimizationPattern> optimization_patterns_;
};

} // namespace perf_analyzer