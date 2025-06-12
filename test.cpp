#include <stdio.h>
#include <string.h>
#include <time.h>

#define PASSWORD_LENGTH 13
#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define CHARSET_SIZE 62

const char *target_password = "Toiohomai1234";

unsigned long long guess_count = 0; // Global counter
unsigned long long guesses_100k = 0;
time_t last_report_time;            // Time of last 100k report

int main() {
    char attempt[PASSWORD_LENGTH + 1];
    int indices[PASSWORD_LENGTH] = {0};
    unsigned long long guess_count = 0;
    time_t last_report_time = time(NULL);

    printf("Starting brute-force...\n");

    attempt[PASSWORD_LENGTH] = '\0';

    while (1) {
        // Build the attempt string from current indices
        for (int i = 0; i < PASSWORD_LENGTH; i++) {
            attempt[i] = CHARSET[indices[i]];
        }

        guess_count++;

        // Check progress
        if (guess_count == 1000000000) {
            guesses_100k++;
            time_t current_time = time(NULL);
            double elapsed = difftime(current_time, last_report_time);
            printf("1B guesses: %llu (%.0f seconds since last 1B)\n", guesses_100k, elapsed);
            last_report_time = current_time;
            guess_count = 0;
        }

        // Check if password matches
        if (strcmp(attempt, target_password) == 0) {
            printf("Password found: %s\n1B guesses: %llu\n+ %llu guesses\n", attempt,guesses_100k, guess_count);
            break;
        }

        // Increment base-CHARSET_SIZE counter
        int pos = PASSWORD_LENGTH - 1;
        while (pos >= 0) {
            indices[pos]++;
            if (indices[pos] < CHARSET_SIZE) {
                break;
            } else {
                indices[pos] = 0;
                pos--;
            }
        }

        // If we've wrapped around past the first digit, we’re done
        if (pos < 0) {
            printf("Password not found.\n");
            break;
        }
    }

    return 0;
}