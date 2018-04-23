/*
    Program for a chat server
    It uses sockets, threads (paralelism), signals and __moreinfohere__

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
*/

/*
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Ivan Aram Gonzalez Su
    A01022584
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// FUNCTION DECLARATIONS
void usage(char *);
void setupHandlers();
void interruptionHandler(int);
void waitForConnections(int);
void *attentionThread(void *);

///// MAIN FUNCTION
int main(int argc, char *argv[]){
    int server_fd;

    // Check the correct arguments
    if (argc != 2){
        usage(argv[0]);
    }
    printf("\n=== SIMPLE BANK SERVER ===\n");
    // Configure the handler to catch SIGINT
    setupHandlers();

	  // Show the IPs assigned to this computer
	  printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	  // Listen for connections from the clients
    waitForConnections(server_fd);
    // Close the socket
    close(server_fd);

    return 0;
}

///// FUNCTION DEFINITIONS
/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char *program){
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers(){
    struct sigaction new_action;

    // Handle Ctrl-c
    new_action.sa_handler = interruptionHandler;
    new_action.sa_flags = SA_RESETHAND;

    // Apply the handler
    sigaction(SIGINT, &new_action, (struct sigaction *)NULL);
}

/*
    Function to handle Ctrl-C (interruption)
*/
void interruptionHandler(int sig){

}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd){
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    int thread_status;
    int poll_response;
	  int timeout = 500; // Time in milliseconds (0.5 seconds)

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1){
		    //// POLL
        // Create a structure array to hold the file descriptors to poll
        struct pollfd test_fds[1];
        // Fill in the structure
        test_fds[0].fd = server_fd;
        test_fds[0].events = POLLIN;    // Check for incomming data
        // Check if there is any incomming communication
        poll_response = poll(test_fds, 1, timeout);

		    // Error when polling
        if (poll_response == -1){
            // Test if the error was caused by an interruption
            if (errno == EINTR){
              // INTERRUPTION
              //printf("Poll did not finish. The program was interrupted");
            }
            else{
                fatalError("ERROR: poll");
            }
        }
		    // There is something ready at the socket
        else{
            // Check the type of event detected
            if (test_fds[0].revents & POLLIN){
        				// ACCEPT
        				// Wait for a client connection
        				client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
        				if (client_fd == -1){
        					fatalError("ERROR: accept");
        				}

        				// Get the data from the client
        				inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
        				printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);
        				// Prepare the structure to send to the thread

        				// Create thread
                pthread_t client_thread;
                thread_status = pthread_create(&client_thread, NULL, &attentionThread, NULL);
                if(thread_status != 0){
                  perror("ERROR: pthread_create");
                  exit(EXIT_FAILURE);
                }
            }
        }
    }
}

/*
    Hear the request from the client and send an answer
*/
void *attentionThread(void *arg){
  pthread_exit(NULL);
}
