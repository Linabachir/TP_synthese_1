# TP_synthese_1
# Objective  
The main objective of this practical task is to develop a micro shell that can execute commands and display details about their execution.  
# Step 1 - Displaying a welcome message, followed by a simple prompt  
To output a message to the terminal, a file descriptor is used:  
`int terminal = STDOUT_FILENO;`  
We then use the 'write()' function, taking the file descriptor and the string we want to display.  
`write(terminal,welcomeMessage,strlen(welcomeMessage));`  
Don't forget to close the file descriptors.  
`close(terminal);`  

