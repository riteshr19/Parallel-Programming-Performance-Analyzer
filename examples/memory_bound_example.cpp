// Example: Memory-bound program with poor cache locality
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>

class MemoryIntensiveTask {
private:
    std::vector<std::vector<double>> data_;
    std::mutex mutex_;
    
public:
    MemoryIntensiveTask(size_t rows, size_t cols) : data_(rows, std::vector<double>(cols)) {
        // Initialize with random data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (auto& row : data_) {
            for (auto& val : row) {
                val = dis(gen);
            }
        }
    }
    
    void process_data_badly(int thread_id, int start_row, int end_row) {
        std::cout << "Thread " << thread_id << " processing rows " << start_row << " to " << end_row << std::endl;
        
        double sum = 0.0;
        
        // Poor memory access pattern - column-wise access (cache unfriendly)
        for (size_t col = 0; col < data_[0].size(); ++col) {
            for (int row = start_row; row < end_row && row < static_cast<int>(data_.size()); ++row) {
                sum += data_[row][col];
                
                // Add some unnecessary synchronization overhead
                if ((row + col) % 100 == 0) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    // Simulate some shared resource access
                    volatile double temp = sum * 1.001;
                    (void)temp;
                }
            }
        }
        
        std::cout << "Thread " << thread_id << " sum: " << sum << std::endl;
    }
    
    void process_data_better(int thread_id, int start_row, int end_row) {
        std::cout << "Thread " << thread_id << " processing rows " << start_row << " to " << end_row << std::endl;
        
        double sum = 0.0;
        
        // Better memory access pattern - row-wise access (cache friendly)
        for (int row = start_row; row < end_row && row < static_cast<int>(data_.size()); ++row) {
            for (size_t col = 0; col < data_[row].size(); ++col) {
                sum += data_[row][col];
            }
        }
        
        std::cout << "Thread " << thread_id << " sum: " << sum << std::endl;
    }
};

int main(int argc, char* argv[]) {
    size_t matrix_rows = 2000;
    size_t matrix_cols = 2000;
    int num_threads = std::thread::hardware_concurrency();
    bool use_bad_pattern = true;
    
    if (argc > 1) {
        matrix_rows = std::atoi(argv[1]);
    }
    if (argc > 2) {
        matrix_cols = std::atoi(argv[2]);
    }
    if (argc > 3) {
        num_threads = std::atoi(argv[3]);
    }
    if (argc > 4) {
        use_bad_pattern = (std::atoi(argv[4]) == 1);
    }
    
    std::cout << "Memory-intensive parallel processing example" << std::endl;
    std::cout << "Matrix size: " << matrix_rows << "x" << matrix_cols << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Using " << (use_bad_pattern ? "cache-unfriendly" : "cache-friendly") << " memory pattern" << std::endl;
    
    MemoryIntensiveTask task(matrix_rows, matrix_cols);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    int rows_per_thread = matrix_rows / num_threads;
    
    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        int start_row = i * rows_per_thread;
        int end_row = (i == num_threads - 1) ? matrix_rows : (i + 1) * rows_per_thread;
        
        if (use_bad_pattern) {
            threads.emplace_back(&MemoryIntensiveTask::process_data_badly, &task, i, start_row, end_row);
        } else {
            threads.emplace_back(&MemoryIntensiveTask::process_data_better, &task, i, start_row, end_row);
        }
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Processing completed in " << duration.count() << " ms" << std::endl;
    
    return 0;
}