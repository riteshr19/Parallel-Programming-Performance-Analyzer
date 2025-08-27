// Example: CPU-bound parallel program with potential optimization opportunities
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>

void cpu_intensive_work(int thread_id, int iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        // Simulate CPU-intensive computation
        double x = dis(gen);
        result += std::sin(x) * std::cos(x) * std::exp(x);
        
        // Add some cache-unfriendly memory access patterns
        if (i % 1000 == 0) {
            volatile double temp = result * 2.0;
            (void)temp; // Prevent optimization
        }
    }
    
    std::cout << "Thread " << thread_id << " completed with result: " << result << std::endl;
}

int main(int argc, char* argv[]) {
    int num_threads = std::thread::hardware_concurrency();
    int iterations_per_thread = 1000000;
    
    if (argc > 1) {
        num_threads = std::atoi(argv[1]);
    }
    if (argc > 2) {
        iterations_per_thread = std::atoi(argv[2]);
    }
    
    std::cout << "Starting CPU-intensive parallel computation..." << std::endl;
    std::cout << "Threads: " << num_threads << ", Iterations per thread: " << iterations_per_thread << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(cpu_intensive_work, i, iterations_per_thread);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Parallel computation completed in " << duration.count() << " ms" << std::endl;
    
    return 0;
}