#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>

#define PORT_NUMBER 9201

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	// Prepare for socket communication
  
	int sockfd, portno = PORT_NUMBER, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1000];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

	// Receive dialogue from server
	n = read(sockfd, buffer, sizeof(buffer));
	if (n < 0) error("ERROR reading from socket");
	printf("%s\n", buffer);
	int i = 0;
	for(i = 0;i<3;i++){
	
		// Receive more dialogue from client
		//printf("recieving from server\n"); fflush(stdout);
		bzero(buffer, sizeof(buffer));
		n = read(sockfd, buffer, sizeof(buffer));
		printf("%s\n", buffer); fflush(stdout);
	
		// Get user's answer and send to server
		//printf("sending to server\n"); fflush(stdout);
		bzero(buffer, sizeof(buffer));
		gets(buffer);
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) error("ERROR writing to socket");	

		// Receive more dialogue from client
		//printf("recieving from server\n"); fflush(stdout);
        	bzero(buffer, sizeof(buffer));
        	n = read(sockfd, buffer, sizeof(buffer));
        	printf("%s\n", buffer); fflush(stdout);
		
		//printf("recieving from server\n"); fflush(stdout);
		bzero(buffer, sizeof(buffer));
                n = read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer); 

		
		if(strstr(buffer, "Game Over") != NULL) {
    			fflush(stdout);
			close(sockfd);
			return 0;
		}
		if(strstr(buffer, "Congratulations") != NULL) {
                        fflush(stdout);
                        close(sockfd);
                        return 0;
                }


	//There are no special data structures here or variables,
	//This program will recieve dialogue and send dialogue to the server until the player quits or loses. The above code will be repeated to account for all the dialogue sent by the server.
	//Additionally there will be code to send the player's guesses. 

	//the program will recieve information on win/loss from the game and will write those statistics to a stats.txt file and include last result, largest win streak and current win streak. 
	
	}
	// Cleanup
	
    close(sockfd);
    return 0;
}
