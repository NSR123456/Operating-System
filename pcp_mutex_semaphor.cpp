#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <mutex>  // For std::mutex

const int BUFFER_SIZE = 5; // Bounded buffer size
int buffer[BUFFER_SIZE];   // Shared buffer
int in = 0;                // Index for producer to insert
int out = 0;               // Index for consumer to remove

// Declare semaphores
HANDLE empty_slots;
HANDLE filled_slots;
HANDLE mutex_buffer;      // Mutex for exclusive access to the buffer

// Declare mutex for synchronized console output
std::mutex output_mutex;

// Producer function
void producer(int id) {
    // Produce data as 0, 1, 2, ..., 4
    int data = id - 1; // Adjust ID to produce 0, 1, 2, ..., 4

    // Wait for empty slot in the buffer
    WaitForSingleObject(empty_slots, INFINITE);
    WaitForSingleObject(mutex_buffer, INFINITE);  // Exclusive access to the buffer

    // Produce data and insert into the buffer
    buffer[in] = data;

    // Synchronized console output to avoid jumbled messages
    {
        std::lock_guard<std::mutex> lock(output_mutex);  // Lock the mutex for printing
        std::cout << "Producer " << id << " produced " << data << " at index " << in << std::endl;
    }

    in = (in + 1) % BUFFER_SIZE;

    // Release mutex and signal that there is new data available
    ReleaseSemaphore(filled_slots, 1, NULL);
    ReleaseSemaphore(mutex_buffer, 1, NULL);  // Release the mutex for buffer access

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate work
}

// Consumer function
void consumer(int id) {
    // Wait for filled slot in the buffer
    WaitForSingleObject(filled_slots, INFINITE);
    WaitForSingleObject(mutex_buffer, INFINITE);  // Exclusive access to the buffer

    // Consume data from the buffer
    int data = buffer[out];

    // Synchronized console output to avoid jumbled messages
    {
        std::lock_guard<std::mutex> lock(output_mutex);  // Lock the mutex for printing
        std::cout << "Consumer " << id << " consumed " << data << " at index " << out << std::endl;
    }

    out = (out + 1) % BUFFER_SIZE;

    // Release mutex and signal that there is space in the buffer
    ReleaseSemaphore(empty_slots, 1, NULL);
    ReleaseSemaphore(mutex_buffer, 1, NULL);  // Release the mutex for buffer access

    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate work
}

int main() {
    srand(time(0));  // Seed the random number generator

    // Initialize semaphores
    empty_slots = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL); // Initially, all slots are empty
    filled_slots = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);          // Initially, no slots are filled
    mutex_buffer = CreateSemaphore(NULL, 1, 1, NULL);                    // Mutex to protect buffer

    // Create producer and consumer threads
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Start producer threads (5 producers)
    for (int i = 1; i <= 5; ++i) {
        producers.push_back(std::thread(producer, i)); // Producer IDs 1 to 5
    }

    // Start consumer threads (5 consumers)
    for (int i = 1; i <= 5; ++i) {
        consumers.push_back(std::thread(consumer, i)); // Consumer IDs 1 to 5
    }

    // Wait for all threads to finish (Producers and Consumers each do their task once)
    for (auto &p : producers) {
        p.join();
    }

    for (auto &c : consumers) {
        c.join();
    }

    // Clean up semaphores
    CloseHandle(empty_slots);
    CloseHandle(filled_slots);
    CloseHandle(mutex_buffer);

    return 0;
}
