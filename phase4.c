#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

// TODO: Implement safe_transfer_ordered
void safe_transfer_ordered(int from_id, int to_id, double amount) {
    // Step 1: Identify which account ID is lower/higher
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    printf("Thread %lu: Requesting locks in order: %d, then %d\n", (unsigned long)pthread_self(), first, second);

    // Step 2 and 3: lock in consistent order and lower id
    pthread_mutex_lock(&accounts[first].lock);
    usleep(100000); // Simulate processing delay
    pthread_mutex_lock(&accounts[second].lock);

    // Step 4: Perdorm transfer
    if (accounts[from_id].balance >= amount) {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        printf("Thread %lu: Successfully transferred $%.2f from %d to %d\n", (unsigned long)pthread_self(), amount, from_id, to_id);
    }

    //Step 5: Unlock
    pthread_mutex_unlock(&accounts[second].lock);
    pthread_mutex_unlock(&accounts[first].lock);
}

void* teller_safe(void* arg) {
    int direction = *(int*)arg;
    if (direction == 0) {
        safe_transfer_ordered(0, 1, 50.0);
    } else {
        safe_transfer_ordered(1, 0, 50.0);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    int d1 = 0, d2 = 1;

    // Initialize
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("=== Phase 4: Deadloack Resoultion (Lock Ordering) ===\n");

    // Create threads that previously deadloacked
    pthread_create(&t1, NULL, teller_safe, &d1);
    pthread_create(&t2, NULL, teller_safe, &d2);

    // Join threads
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\nFinal Account Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%.2f\n", i, accounts[i].balance);
    }

    // Cleanup
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}


// Key Concepts
// Breaking circular wait
// By enforcing a strict ordering of resource acquisition, ensuring processes can't form a closed loop of dependencies
// Lock ordering strategies
// Gloabl ordering, hash based ordering, resource hierarchy, and token based ordering are some lock ordering strategies
// Timeout mechanisms
// Is a software tool that terminates an operation if it does not complete within a predefined timeframe.


