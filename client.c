/*
    Program for a chat client
    It uses sockets, threads (paralelism), signals and __moreinfohere__

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
*/

/*
    Client program to access the accounts in the bank
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
#include "codes.h"

#define BUFFER_SIZE 1024

char KEY_VAL[12];

///// FUNCTION DECLARATIONS
void usage(char *);
void chatOperations(int);

///// MAIN FUNCTION
int main(int argc, char *argv[]){
	int connection_fd;
	
	// Check the correct arguments
	if (argc != 3)
		usage(argv[0]);
	
	
	// Start the server connection
	connection_fd = connectSocket(argv[1], argv[2]);
	
	// Use the bank operations available
	printf("\n=== CHAT CLIENT PROGRAM ===\n");
	
	chatOperations(connection_fd);
	// Close the socket
	close(connection_fd);
	
	return 0;
}

///// FUNCTION DEFINITIONS
/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char *program){
	printf("Usage:\n");
	printf("\t%s {server_address} {port_number}\n", program);
	exit(EXIT_FAILURE);
}

/*
    Main menu with the options available to the user
*/
void chatOperations(int connection_fd) {
	char buffer[BUFFER_SIZE];
	char name[50];
	char *topics[3];
	int chat_room = 4;
	char opt;
	int operation = NAME;
	
	// Start interaction with user's client
	printf("Enter your display name: ");
	if( fgets(name, 50, stdin) == -1 ) {
		printf("Error reading the name");
		exit(EXIT_FAILURE);
	}
	
	sprintf(buffer, "%d %s", operation, name);

	sendString(connection_fd, buffer);

	// Recieve response of the user creation
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
        printf("The server got lost while finding the topics\n");
        exit(EXIT_FAILURE);
    }
    sscanf(buffer, "%d %d", &operation, &chat_room);
	

	if(operation == OK) {
		printf("hola %s", name);
		// Recieve the topics if the name creating was OK
		if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
	        printf("The server got lost while finding the topics\n");
	        exit(EXIT_FAILURE);
	    }
	    

	    sscanf(buffer, "%s %s %s", topics[0], topics[1], topics[2]);
	    // Ask user what topics does he want to join, save in chat_room
	    printf("-+-+-+-+- Select a topic -+-+-+-+-");
	    printf("\t 0. %s", topics[0]);
	    printf("\t 1. %s", topics[1]);
	    printf("\t 2. %s\n", topics[2]);
	    scanf(" %d", &chat_room);
	    if(chat_room < 0 && chat_room > 2) {
	    	printf("Invalid topic option");
	    	exit(EXIT_FAILURE);
	    }
	} else {
		printf("Something went wrong creating your user. Try again");
		exit(EXIT_FAILURE);
	}
	
	// Request server a list of topics
	operation = TOPIC;
	sprintf(buffer, "%d %d", operation, chat_room);
	sendString(connection_fd, buffer);
	
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
        printf("The server had problems connecting you with the room\n");
        exit(EXIT_FAILURE);
    }
    
    sscanf(buffer, "%d", &operation);
    if(operation == KEY) {
    	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
	        printf("The server had problems with the ids\n");
	        exit(EXIT_FAILURE);
	    }
	    sscanf(buffer, "%s", KEY_VAL);
    } else {
    	printf("An error happend with the key");
    	exit(EXIT_FAILURE);
    }
    
	while(opt != 'c') {
		printf("-*-*-* ChatRoom Menu -*-*-*");
		printf("\ta. Write Message");
		printf("\tb. Refresh");
		printf("\tc. Exit");
		scanf("%c", &opt);
		switch(opt) {
			case 'a':
				// Write new message
				operation = SEND;
				break;
			case 'b':
				// TODO: Ask server for all the messages in the group;
				operation = SHOW;
				break;
			case 'c':
				// Exit program
				operation = EXIT;
				break;
			default:
	            printf("Invalid option. Try again ...\n");
	            // This skips the rest of the while
	            continue;
		}
		// TODO: Prepare the message to the server
		// sprintf(buffer, "%d %d %f", operation, account, amount);
		sendString(connection_fd, buffer);
	}
	
}
