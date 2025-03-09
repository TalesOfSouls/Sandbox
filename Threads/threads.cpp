#include <windows.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

// Constants
const int NUM_ITERATIONS = 100; // Number of synchronization operations
const int NUM_THREADS = 2;         // Number of threads (one signaler, one waiter)

// Latency and CPU usage measurements
struct TestResult {
    double latency_ms; // Average latency per operation
};

// Test synchronization mechanisms
enum class SyncMechanism {
    EVENT,
    SPINLOCK,
    SEMAPHORE,
    CONDITION_VARIABLE,
    MUTEX
};

// Thread function for the waiter
void WaiterThread(SyncMechanism mechanism, HANDLE event, std::atomic<bool>& spinlock, HANDLE semaphore, CONDITION_VARIABLE& cv, CRITICAL_SECTION& cs, bool& ready) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        switch (mechanism) {
            case SyncMechanism::EVENT:
                WaitForSingleObject(event, INFINITE);
                break;
            case SyncMechanism::SPINLOCK:
                while (spinlock.exchange(true)) {} // Spin until lock is acquired
                spinlock.store(false); // Release the lock
                break;
            case SyncMechanism::SEMAPHORE:
                WaitForSingleObject(semaphore, INFINITE);
                break;
            case SyncMechanism::CONDITION_VARIABLE:
                EnterCriticalSection(&cs);
                while (!ready) {
                    SleepConditionVariableCS(&cv, &cs, INFINITE);
                }
                ready = false;
                LeaveCriticalSection(&cs);
                break;
            case SyncMechanism::MUTEX:
                EnterCriticalSection(&cs);
                LeaveCriticalSection(&cs);
                break;
        }
    }
}

// Thread function for the signaler
void SignalerThread(SyncMechanism mechanism, HANDLE event, std::atomic<bool>& spinlock, HANDLE semaphore, CONDITION_VARIABLE& cv, CRITICAL_SECTION& cs, bool& ready) {
    // Needs to be much larger value since the signaler thread runs so much faster
    for (int i = 0; i < NUM_ITERATIONS * 1000; ++i) {
        switch (mechanism) {
            case SyncMechanism::EVENT:
                SetEvent(event);
                break;
            case SyncMechanism::SPINLOCK:
                spinlock.store(false); // Release the lock
                break;
            case SyncMechanism::SEMAPHORE:
                ReleaseSemaphore(semaphore, 1, NULL);
                break;
            case SyncMechanism::CONDITION_VARIABLE:
                EnterCriticalSection(&cs);
                ready = true;
                WakeConditionVariable(&cv);
                LeaveCriticalSection(&cs);
                break;
            case SyncMechanism::MUTEX:
                EnterCriticalSection(&cs);
                LeaveCriticalSection(&cs);
                break;
        }
    }
}

// Test function for each synchronization mechanism
TestResult TestSynchronization(SyncMechanism mechanism) {
    // Synchronization objects
    HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
    std::atomic<bool> spinlock(false);
    HANDLE semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    CONDITION_VARIABLE cv;
    CRITICAL_SECTION cs;
    InitializeConditionVariable(&cv);
    InitializeCriticalSection(&cs);
    bool ready = false;

    // Measure latency
    auto start_time = std::chrono::high_resolution_clock::now();

    // Create threads
    std::thread waiter(WaiterThread, mechanism, event, std::ref(spinlock), semaphore, std::ref(cv), std::ref(cs), std::ref(ready));
    std::thread signaler(SignalerThread, mechanism, event, std::ref(spinlock), semaphore, std::ref(cv), std::ref(cs), std::ref(ready));

    // Wait for threads to finish
    waiter.join();
    signaler.join();

    // Measure latency
    auto end_time = std::chrono::high_resolution_clock::now();
    double latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count() / NUM_ITERATIONS;

    // Clean up
    CloseHandle(event);
    CloseHandle(semaphore);
    DeleteCriticalSection(&cs);

    return {latency_ms};
}

int main() {
    // Test each synchronization mechanism
    std::vector<std::pair<SyncMechanism, std::string>> mechanisms = {
        {SyncMechanism::EVENT, "Event"},
        {SyncMechanism::SPINLOCK, "Spinlock"},
        {SyncMechanism::SEMAPHORE, "Semaphore"},
        {SyncMechanism::CONDITION_VARIABLE, "Condition Variable"},
        {SyncMechanism::MUTEX, "Mutex"}
    };

    for (const auto& [mechanism, name] : mechanisms) {
        TestResult result = TestSynchronization(mechanism);
        std::cout << name << ":\n";
        std::cout << "  Latency: " << result.latency_ms << " ms per operation\n";
        std::cout << "-------------------------\n";
    }

    return 0;
}

/*
Event:
  Latency: 0.539111 ms per operation
-------------------------
Spinlock:
  Latency: 0.01303 ms per operation
-------------------------
Semaphore:
  Latency: 2.16932 ms per operation
-------------------------
Condition Variable:
  Latency: 0.028234 ms per operation
-------------------------
Mutex:
  Latency: 0.016222 ms per operation
-------------------------
*/