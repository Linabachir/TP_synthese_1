#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_INPUT_SIZE 256

int terminal = STDOUT_FILENO;  // Descripteur de fichier pour afficher sur le terminal
int fd_input = STDIN_FILENO;   // Descripteur de fichier pour lire depuis stdin
int status;
char input[MAX_INPUT_SIZE];
int bytesRead;

char waitingPrompt[MAX_INPUT_SIZE] = "";
char exitSucesss[] = "\nEnd of ShellENSEA\nBye bye...\n";

// Variables pour la gestion du temps d'exécution
struct timespec starttime, endtime;
int exit_signal_status;
double time_elapsed;

// Fonction pour gérer les redirections
void handle_redirection(char *input) {
    char *args[MAX_INPUT_SIZE];
    char *input_file = NULL;
    char *output_file = NULL;
    int i = 0;

    // Séparer la commande par espaces
    char *token = strtok(input, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");  // Récupère le fichier d'entrée
            input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");  // Récupère le fichier de sortie
            output_file = token;
        } else {
            args[i++] = token;  // Ajouter l'argument à la commande
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Terminer la liste des arguments

    // Gérer la redirection vers le fichier d'entrée (<)
    if (input_file != NULL) {
        fd_input = open(input_file, O_RDONLY);
        if (fd_input == -1) {
            perror("Erreur d'ouverture du fichier d'entrée");
            exit(EXIT_FAILURE);
        }
        dup2(fd_input, STDIN_FILENO);  // Redirige stdin vers le fichier d'entrée
        close(fd_input);
    }

    // Gérer la redirection vers le fichier de sortie (>)
    if (output_file != NULL) {
        int fd_output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_output == -1) {
            perror("Erreur d'ouverture du fichier de sortie");
            exit(EXIT_FAILURE);
        }
        dup2(fd_output, STDOUT_FILENO);  // Redirige stdout vers le fichier de sortie
        close(fd_output);
    }

    // Exécuter la commande
    execvp(args[0], args);
}

// Fonction pour exécuter la commande
void command(char input[], int bytesRead) {
    if (strcmp(input, "exit") == 0 || bytesRead == 0) {
        write(terminal, exitSucesss, sizeof(exitSucesss));
        exit(EXIT_SUCCESS);
    }

    pid_t pid = fork();

    if (pid < 0) {
        close(fd_input);
        close(terminal);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        handle_redirection(input);  // Gère la redirection et exécute la commande
        close(fd_input);
        close(terminal);
        exit(EXIT_SUCCESS);
    } else {
        wait(&status);
    }
}

// Fonction pour afficher le code de retour et le temps d'exécution
void return_code(void) {
    time_elapsed = (endtime.tv_sec - starttime.tv_sec) * 1000 + (endtime.tv_nsec - starttime.tv_nsec) / 1e6;

    if (WIFEXITED(status)) {
        exit_signal_status = WEXITSTATUS(status);
        sprintf(waitingPrompt, "enseash [exit:%d|%.0f ms] %% ", exit_signal_status, time_elapsed);
    } else if (WIFSIGNALED(status)) {
        exit_signal_status = WTERMSIG(status);
        sprintf(waitingPrompt, "enseash [sign:%d|%.0f ms] %% ", exit_signal_status, time_elapsed);
    }
}

// Fonction d'affichage initiale
void shellDisplay(void) {
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    write(terminal, welcomeMessage, strlen(welcomeMessage));
}

int main(int argc, char **argv) {
    shellDisplay();

    while (1) {
        return_code();
        write(terminal, waitingPrompt, sizeof(waitingPrompt) - 1);

        bytesRead = read(fd_input, input, sizeof(input));
        input[bytesRead - 1] = '\0';  // Retirer le '\n' de la fin
        clock_gettime(CLOCK_MONOTONIC, &starttime);
        command(input, bytesRead);
        clock_gettime(CLOCK_MONOTONIC, &endtime);
    }

    return EXIT_SUCCESS;
}
