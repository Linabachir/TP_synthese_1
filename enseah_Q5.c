#define _POSIX_C_SOURCE 199309L
#include <unistd.h> // For EXIT_SUCCESS/FAILURE
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // For strlen
#include <fcntl.h>  // For open/creat
#include <sys/wait.h>
#include <time.h>   // For clock_gettime

#define MAX_INPUT_SIZE 256

// File Descriptors
int terminal = STDOUT_FILENO; // Sending arguments to terminal
int fd_input = STDIN_FILENO;  // Getting arguments

int status;

// Input variable
char input[MAX_INPUT_SIZE];
int bytesRead;

char waitingPrompt[MAX_INPUT_SIZE] = "";
char exitSuccess[] = "\nEnd of ShellENSEA\nBye bye...\n";

// Time control
struct timespec starttime, endtime;

int exit_signal_status;
double time_elapsed;

void shellDisplay(void) {
    // Informational Messages
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    write(terminal, welcomeMessage, strlen(welcomeMessage));
}

void command(char input[], int bytesRead) {
    if (strcmp(input, "exit") == 0 || bytesRead == 0) { // Enter in if exit or ctrl+d
        write(terminal, exitSuccess, strlen(exitSuccess));
        exit(EXIT_SUCCESS);
    }

    pid_t pid = fork();

    if (pid < 0) { // Échec du fork
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Processus enfant
        // Essaye d'exécuter la commande
        if (execlp(input, input, NULL) == -1) {
            perror(input); // Affiche l'erreur ("blabla: command not found")
            exit(127);     // Retourne 127 pour signaler une commande introuvable
        }
    } else { // Processus parent
        wait(&status);
    }
}

void return_code(void) {
    // Time conversion takes into account seconds and nanoseconds
    time_elapsed = (endtime.tv_sec - starttime.tv_sec) * 1000 +
                   (endtime.tv_nsec - starttime.tv_nsec) / 1e6;

    // Return code for exit and signal
    if (WIFEXITED(status)) {
        exit_signal_status = WEXITSTATUS(status);
        sprintf(waitingPrompt, "enseash [exit:%d|%.0f ms] %% ", exit_signal_status, time_elapsed);
    } else if (WIFSIGNALED(status)) {
        exit_signal_status = WTERMSIG(status);
        sprintf(waitingPrompt, "enseash [sign:%d|%.0f ms] %% ", exit_signal_status, time_elapsed);
    }
}

int main(int argc, char **argv) {
    shellDisplay();

    while (1) {
        return_code();
        write(terminal, waitingPrompt, strlen(waitingPrompt));

        bytesRead = read(fd_input, input, sizeof(input) - 1); // Laisse de la place pour '\0'
        if (bytesRead <= 0) { // Gestion des erreurs de lecture ou EOFb
            write(terminal, exitSuccess, strlen(exitSuccess));
            break;
        }

        input[bytesRead - 1] = '\0'; // Removing the '\n' at the end
        clock_gettime(CLOCK_MONOTONIC, &starttime);
        command(input, bytesRead);
        clock_gettime(CLOCK_MONOTONIC, &endtime);
    }

    return EXIT_SUCCESS;
}