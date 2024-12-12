#include <unistd.h> //For EXIT_SUCCESS/FAILURE
#include <stdlib.h>
#include <stdio.h>
#include <string.h> //For strlen
#include <fcntl.h> //For open/creat

//Files descriptor
int terminal = STDOUT_FILENO; //Sending arguments to terminal

void shellDisplay(void) {
    //Informational Messages
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    char waitingPrompt[] = "enseash %\n";

    //Displaying
    write(terminal,welcomeMessage,strlen(welcomeMessage));
    write(terminal,waitingPrompt,strlen(waitingPrompt));

    close(terminal);

}

void main(int argc,char** argv){

    shellDisplay();

    close(terminal);
    exit(EXIT_SUCCESS);

}