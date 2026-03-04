#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 8
#define TRANSACTIONS_PER_THREAD 100000 // Increased to mesaure performace difference
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

// Safe functions with mutexes

void deposit_safe(int account_id, double amount) {
    pthread_mutex_lock(&accounts[account_id].lock);
    // Critical Section
    accounts[account_id].balance += amount;
    accounts[account_id]. transaction_count++;
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// TODO 1: Implement withdrawal_safe()
void withdrawal_safe(int account_id, double amount) {
    pthread_mutex_lock(&accounts[account_id].lock);
    // Critical Section
    accounts[account_id].balance -= amount;
    accounts[account_id].transaction_count++;
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// TODO 2: Updated teller_thread
void *teller_thread_safe(void *arg) {
    int teller_id = *(int *)arg;
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self());

    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        int account_idx = rand_r(&seed) % NUM_ACCOUNTS;
        double amount = 1.0;
        int operation = rand_r(&seed) % 2;

        if (operation == 1) {
            deposit_safe(account_idx, amount);
        } else {
            withdrawal_safe(account_idx, amount);
        }
    }
    return NULL;
}

// Initialization and cleanup

void initialize_accounts() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

// TODO 4: Mutex cleanup
void cleanup_mutexes() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

int main() {
    struct timespec start, end;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    initialize_accounts();

    printf("=== Phase 2: Mutex Protection Demo ===\n");

    // TODO 5: performance timing
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread_safe, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Results dispaly
    int total_actual_transactions = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d Final Balance: $%.2f (Trans: %d)\n", i, accounts[i].balance, accounts[i].transaction_count);
        total_actual_transactions += accounts[i].transaction_count;
    }

    int expected_transactions = NUM_THREADS * TRANSACTIONS_PER_THREAD;
    printf("\nTotal Transactions Expected: %d\n", expected_transactions);
    printf("Total Transactions Recorded: %d\n", total_actual_transactions);
    printf("Execution Time: %.6f seconds\n", time_taken);

    if (total_actual_transactions == expected_transactions) {
        printf("\nRESULT: SUCCESS - No transactions lost!\n");
    } else {
        printf("\nRESULT: FAILURE - Race conditions still present.\n");
    }

    cleanup_mutexes();
    return 0;
}

// Key Concepts
// What is mutual exclusion?
// Is a computing concept ensuring that only one thread or process accesses a shared resource at a time
// What is critical section?
// Is code in a program that access shared resources that should not be executed by more than one thread simultaneously
// How do mutex locks work?
// A thread must lock the mutex to gain ownership, forcing other threads to wait until the holder calls unlock
// Performance impact of synchronization
// It causes threads or processes to wait for shared resources, creating reduce parallel scalability
