#include "performance_analyzer.h"
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <cstdlib>

using namespace perf_analyzer;

void printUsage(const char* program_name) {
    std::cout << "Parallel Programming Performance Analyzer\n";
    std::cout << "Advanced tool for analyzing parallel program performance\n\n";
    std::cout << "Usage: " << program_name << " [OPTIONS] [PROGRAM] [ARGS...]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -p, --pid PID           Analyze running process with given PID\n";
    std::cout << "  -d, --duration SECS     Analysis duration in seconds (default: 10.0)\n";
    std::cout << "  -g, --gpu               Enable GPU analysis\n";
    std::cout << "  -v, --verbose           Enable verbose output\n";
    std::cout << "  -o, --output FILE       Output report to file\n";
    std::cout << "  --no-gpu                Disable GPU analysis\n";
    std::cout << "  --sampling-rate RATE    Set sampling rate in Hz (default: 100)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " ./my_parallel_program\n";
    std::cout << "  " << program_name << " -p 1234 -d 30.0 -g\n";
    std::cout << "  " << program_name << " -o report.txt ./parallel_app arg1 arg2\n\n";
    std::cout << "Features:\n";
    std::cout << "  • Identifies performance bottlenecks in parallel programs\n";
    std::cout << "  • Analyzes GPU utilization and CUDA/OpenCL performance\n";
    std::cout << "  • Provides optimization suggestions\n";
    std::cout << "  • Detects load balancing issues\n";
    std::cout << "  • Analyzes thread synchronization overhead\n";
    std::cout << "  • Measures communication patterns and efficiency\n";
}

int main(int argc, char* argv[]) {
    // Configuration variables
    std::string program_path;
    std::vector<std::string> program_args;
    int target_pid = -1;
    double duration = 10.0;
    bool gpu_analysis = true;
    bool verbose = false;
    std::string output_file;
    int sampling_rate = 100;
    
    // Parse command line arguments
    static struct option long_options[] = {
        {"help",         no_argument,       0, 'h'},
        {"pid",          required_argument, 0, 'p'},
        {"duration",     required_argument, 0, 'd'},
        {"gpu",          no_argument,       0, 'g'},
        {"verbose",      no_argument,       0, 'v'},
        {"output",       required_argument, 0, 'o'},
        {"no-gpu",       no_argument,       0, 1001},
        {"sampling-rate", required_argument, 0, 1002},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "hp:d:gvo:", long_options, nullptr)) != -1) {
        switch (c) {
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'p':
                target_pid = std::atoi(optarg);
                break;
            case 'd':
                duration = std::atof(optarg);
                break;
            case 'g':
                gpu_analysis = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 1001:  // --no-gpu
                gpu_analysis = false;
                break;
            case 1002:  // --sampling-rate
                sampling_rate = std::atoi(optarg);
                break;
            case '?':
                std::cerr << "Unknown option. Use -h for help.\n";
                return 1;
            default:
                break;
        }
    }
    
    // Get program path and arguments if not analyzing existing process
    if (target_pid == -1) {
        if (optind >= argc) {
            std::cerr << "Error: No program specified and no PID given.\n";
            std::cerr << "Use -h for help.\n";
            return 1;
        }
        
        program_path = argv[optind];
        for (int i = optind + 1; i < argc; i++) {
            program_args.push_back(argv[i]);
        }
    }
    
    // Validate arguments
    if (duration <= 0) {
        std::cerr << "Error: Duration must be positive.\n";
        return 1;
    }
    
    if (sampling_rate <= 0 || sampling_rate > 10000) {
        std::cerr << "Error: Sampling rate must be between 1 and 10000 Hz.\n";
        return 1;
    }
    
    try {
        // Initialize the performance analyzer
        PerformanceAnalyzer analyzer;
        analyzer.setAnalysisDuration(duration);
        analyzer.enableGPUAnalysis(gpu_analysis);
        analyzer.setVerboseOutput(verbose);
        
        if (verbose) {
            std::cout << "Parallel Programming Performance Analyzer v1.0\n";
            std::cout << "==============================================\n\n";
            
            if (target_pid != -1) {
                std::cout << "Analyzing process PID: " << target_pid << "\n";
            } else {
                std::cout << "Analyzing program: " << program_path << "\n";
                if (!program_args.empty()) {
                    std::cout << "Arguments: ";
                    for (const auto& arg : program_args) {
                        std::cout << arg << " ";
                    }
                    std::cout << "\n";
                }
            }
            
            std::cout << "Analysis duration: " << duration << " seconds\n";
            std::cout << "GPU analysis: " << (gpu_analysis ? "enabled" : "disabled") << "\n";
            std::cout << "Sampling rate: " << sampling_rate << " Hz\n\n";
        }
        
        // Run the analysis
        bool success = false;
        if (target_pid != -1) {
            success = analyzer.analyzeRunningProcess(target_pid);
        } else {
            success = analyzer.analyzeProgram(program_path, program_args);
        }
        
        if (!success) {
            std::cerr << "Error: Failed to analyze the target program/process.\n";
            return 1;
        }
        
        // Display results
        if (verbose) {
            std::cout << "\nAnalysis Complete!\n";
            std::cout << "==================\n\n";
        }
        
        analyzer.printSummary();
        
        // Generate detailed report if requested
        if (!output_file.empty()) {
            analyzer.generateReport(output_file);
            std::cout << "\nDetailed report saved to: " << output_file << "\n";
        }
        
        // Provide performance insights
        const auto& bottlenecks = analyzer.getBottlenecks();
        const auto& suggestions = analyzer.getSuggestions();
        
        if (!bottlenecks.empty()) {
            std::cout << "\n🔍 Performance Bottlenecks Detected:\n";
            std::cout << "====================================\n";
            for (size_t i = 0; i < bottlenecks.size() && i < 5; ++i) {
                const auto& bottleneck = bottlenecks[i];
                std::cout << (i + 1) << ". " << bottleneck.description 
                          << " (Severity: " << (bottleneck.severity_score * 100) << "%)\n";
            }
        }
        
        if (!suggestions.empty()) {
            std::cout << "\n💡 Optimization Suggestions:\n";
            std::cout << "============================\n";
            for (size_t i = 0; i < suggestions.size() && i < 5; ++i) {
                const auto& suggestion = suggestions[i];
                std::cout << (i + 1) << ". " << suggestion.title;
                if (suggestion.potential_improvement > 0) {
                    std::cout << " (Potential improvement: " 
                              << suggestion.potential_improvement << "%)";
                }
                std::cout << "\n   " << suggestion.description << "\n\n";
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}