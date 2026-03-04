#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Configuration
#define NUM_ACCOUNTS 2
#define NUM_THREADS 8
#define TRANSACTIONS_PER_THREAD 20
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

Account accounts[NUM_ACCOUNTS];

void deposit_unsafe(int account_id, double amount) {
    double current_balance = accounts[account_id].balance;
    usleep(1); // Simulate processing time to force race conditions
    double new_balance = current_balance + amount;
    accounts[account_id].balance = new_balance;
    accounts[account_id].transaction_count++;
}

// TODO 1: Implement withdrawl_unsafe()
void withdrawal_unsafe(int account_id, double amount) {
    // READ
    double current_balance = accounts[account_id].balance;
    // MODIFY
    usleep(1);
    double new_balance = current_balance - amount;
    // WRITE
    accounts[account_id].balance = new_balance;
    accounts[account_id].transaction_count++;
}

// TODO 2: Implement the thread function
void *teller_thread(void *arg) {
    int teller_id = *(int *)arg;

    // TODO 2a: Initialize thread-safe random seed
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self());

    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        // TODO 2b: Randomly select an account
        int account_idx = rand_r(&seed) % NUM_ACCOUNTS;

        // TODO 2c: Generate random amount (1-100)
        double amount = (rand_r(&seed) % 100) + 1;

        // TODO 2d: Randomly choose deposit (1) or withdrawal (0)
        int operation = rand_r(&seed) % 2;

        // TODO 2e: Call appropriate function
        if (operation == 1) {
            deposit_unsafe(account_idx, amount);
            printf("Teller %d: Deposited $%.2f to Account %d\n", teller_id, amount, account_idx);
        } else {
            withdrawal_unsafe(account_idx, amount);
            printf("Teller %d: Withdrew $%.2f from Account %d\n", teller_id, amount, account_idx);
        }
    }
    return NULL;
}

int main() {
    printf("=== Phase 1: Race Conditions Demo ===\n\n");

    // TODO 3a: Initialize all accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
    }

    printf("Initial State:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf(" Account %d: $%.2f\n", i, accounts[i].balance);
    }

    // TODO 3b: Calculate expected final balance
    double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE;

    // TODO 3c/d: Create threads
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // TODO 3e: Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // TODO 3f: Calculate and display results
    printf("\n=== Final Results ===\n");
    double actual_total = 0.0;
    int total_transactions = 0;

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf(" Account %d: $%.2f (%d transactions recorded)\n", i, accounts[i].balance, accounts[i].transaction_count);

        actual_total += accounts[i].balance;
        total_transactions += accounts[i].transaction_count;
    }

    printf("\nExpected Total Transactions %d\n", NUM_THREADS * TRANSACTIONS_PER_THREAD);
    printf("Actual Transactions Recorded: %d\n", total_transactions);
    printf("Expected Final Total: (Variable based on random ops)\n");
    printf("Actual Final Total: $%.2f\n", actual_total);

    // TODO 3g: Race condition detection
    printf("\nNote: Run this multiple times. You will notice 'lost' transactions\n");
    printf("where the transaction_count does not equal %d.\n", NUM_THREADS * TRANSACTIONS_PER_THREAD);

    return 0;
}

// Key Concepts
// What is a thread vs. a process?
// Process is a program with its own space of memory while a thread is a the smallest unit of execution within a process
// What is a race condition and why does it occur?
// Race condition is a software or hardware flaw it occurs due to lack of proper synchronization
// What is the read-modify-write problem?
// This is a race condidition occuring when a processor reads/modify a meomeory loacation and writes it back
// Why do we need thread synchronization?
// We need it because it allows access to shared resources, preventing data corruption, and unpredictable behavior

