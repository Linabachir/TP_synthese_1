#define _POSIX_C_SOURCE 199309L
#include <unistd.h> // For EXIT_SUCCESS/FAILURE
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // For strlen
#include <fcntl.h>  // For open/creat
#include <sys/wait.h>
#include <time.h>   // For clock_gettime
#include <signal.h> // For signal handling

#define MAX_INPUT_SIZE 256

// File Descriptors
int terminal = STDOUT_FILENO; // Sending arguments to terminal
int fd_input = STDIN_FILENO;  // Getting arguments

int status; // Variable pour le code de retour

// Input variable
char input[MAX_INPUT_SIZE];
int bytesRead;

char waitingPrompt[MAX_INPUT_SIZE] = "";

// Time control (ici on n'utilise pas le temps pour cette question spécifique)
struct timespec starttime, endtime;

int exit_signal_status; // Code de retour ou signal

void shellDisplay(void) {
    // Informational Messages
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    write(terminal, welcomeMessage, strlen(welcomeMessage));
}

void command(char input[], int bytesRead) {
    if (strcmp(input, "exit") == 0 || bytesRead == 0) { // Entrée "exit" ou commande vide
        exit(EXIT_SUCCESS); // Quitter le shell
    }

    pid_t pid = fork();

    if (pid < 0) { // Échec du fork
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { 
        //printf("Child process PID: %d\n", getpid()); //display PID and simulate a delay
        //fflush(stdout);
        //sleep (80) ;// Processus enfant
        // Essaye d'exécuter la commande
        if (execlp(input, input, NULL) == -1) {
            perror(input); // Affiche l'erreur ("command not found")
            exit(127);     // Retourne 127 pour signaler une commande introuvable
        }
    } else { // Processus parent
        wait(&status); // Attendre que le processus enfant se termine
    }
}

void return_code(void) {
    // Vérification du code de retour ou du signal de la dernière commande
    if (WIFEXITED(status)) {
        // Si le programme s'est terminé normalement
        exit_signal_status = WEXITSTATUS(status);
        sprintf(waitingPrompt, "enseash [exit:%d] %% ", exit_signal_status);
    } else if (WIFSIGNALED(status)) {
        // Si le programme a été tué par un signal
        exit_signal_status = WTERMSIG(status);
        sprintf(waitingPrompt, "enseash [sign:%d] %% ", exit_signal_status);
    }
}

int main(int argc, char **argv) {
    shellDisplay();

    while (1) {
        return_code(); // Met à jour le prompt avec le code de retour de la dernière commande
        write(terminal, waitingPrompt, strlen(waitingPrompt)); // Affiche le prompt

        bytesRead = read(fd_input, input, sizeof(input) - 1); // Lire l'entrée
        if (bytesRead <= 0) { // Gestion de l'EOF ou erreur de lecture
            write(terminal, "End of ShellENSEA\nBye bye...\n", 30);
            break;
        }

        input[bytesRead - 1] = '\0'; // Supprimer le '\n' à la fin
        command(input, bytesRead); // Exécuter la commande
    }

    return EXIT_SUCCESS;
}