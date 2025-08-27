// Example: Load imbalanced parallel program
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>

void unbalanced_work(int thread_id, int work_amount) {
    std::cout << "Thread " << thread_id << " starting with work amount: " << work_amount << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Simulate work that takes different amounts of time per thread
    for (int i = 0; i < work_amount; ++i) {
        // Some computational work
        volatile double result = 0.0;
        for (int j = 0; j < 1000; ++j) {
            result += std::sin(i * j * 0.001) * std::cos(i * j * 0.001);
        }
        
        // Add some variable delay to simulate real-world unpredictable work
        if (i % 100 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(thread_id * 10));
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Thread " << thread_id << " completed in " << duration.count() << " ms" << std::endl;
}

void producer_consumer_example(int producer_id, std::vector<int>& shared_queue, std::mutex& queue_mutex, bool is_producer) {
    if (is_producer) {
        std::cout << "Producer " << producer_id << " starting..." << std::endl;
        
        for (int i = 0; i < 1000; ++i) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                shared_queue.push_back(i);
            }
            
            // Simulate some work between productions
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        std::cout << "Producer " << producer_id << " finished" << std::endl;
    } else {
        std::cout << "Consumer " << producer_id << " starting..." << std::endl;
        
        int consumed = 0;
        while (consumed < 500) { // Each consumer processes 500 items
            int item = -1;
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (!shared_queue.empty()) {
                    item = shared_queue.back();
                    shared_queue.pop_back();
                    consumed++;
                }
            }
            
            if (item != -1) {
                // Simulate processing
                volatile double result = std::sin(item) * std::cos(item);
                (void)result;
            } else {
                // No work available, sleep briefly
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }
        
        std::cout << "Consumer " << producer_id << " finished, consumed " << consumed << " items" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    int example_type = 1; // 1 = load imbalance, 2 = producer-consumer
    int num_threads = std::thread::hardware_concurrency();
    
    if (argc > 1) {
        example_type = std::atoi(argv[1]);
    }
    if (argc > 2) {
        num_threads = std::atoi(argv[2]);
    }
    
    if (example_type == 1) {
        std::cout << "Load Imbalance Example" << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        
        std::vector<std::thread> threads;
        std::vector<int> work_amounts;
        
        // Create highly unbalanced work distribution
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 10000);
        
        for (int i = 0; i < num_threads; ++i) {
            int work = dis(gen);
            // Make some threads have much more work
            if (i % 3 == 0) {
                work *= 3; // Triple the work for every third thread
            }
            work_amounts.push_back(work);
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Create threads with unbalanced work
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(unbalanced_work, i, work_amounts[i]);
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Load imbalanced execution completed in " << duration.count() << " ms" << std::endl;
        
        // Show work distribution
        std::cout << "\nWork distribution:" << std::endl;
        for (int i = 0; i < num_threads; ++i) {
            std::cout << "Thread " << i << ": " << work_amounts[i] << " work units" << std::endl;
        }
        
    } else if (example_type == 2) {
        std::cout << "Producer-Consumer Example with Synchronization Overhead" << std::endl;
        
        std::vector<int> shared_queue;
        std::mutex queue_mutex;
        std::vector<std::thread> threads;
        
        int num_producers = std::max(1, num_threads / 3);
        int num_consumers = num_threads - num_producers;
        
        std::cout << "Producers: " << num_producers << ", Consumers: " << num_consumers << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Create producer threads
        for (int i = 0; i < num_producers; ++i) {
            threads.emplace_back(producer_consumer_example, i, std::ref(shared_queue), 
                               std::ref(queue_mutex), true);
        }
        
        // Create consumer threads
        for (int i = 0; i < num_consumers; ++i) {
            threads.emplace_back(producer_consumer_example, i + num_producers, std::ref(shared_queue), 
                               std::ref(queue_mutex), false);
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Producer-consumer execution completed in " << duration.count() << " ms" << std::endl;
        std::cout << "Remaining items in queue: " << shared_queue.size() << std::endl;
    }
    
    return 0;
}