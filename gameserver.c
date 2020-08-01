#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#define PORT_NUMBER 9201

char data[1000] = {0};
// Helper function for error messages (not required)
void error(const char *msg)
{
    perror(msg);
    exit(1);
}



int main(int argc, char *argv[])
{	srand(time(NULL));
	int playercounter = 0; //intialize to 0.
	int numplay = 5;
	int sockfd, newsockfd, portno = PORT_NUMBER;
	socklen_t clilen;
	char buffer1[1000];
	struct sockaddr_in serv_addr, cli_addr;
	int n; // Number of bytes written/read
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	printf("Listening for clients...\n"); fflush(stdout);
	
	 //repeatedly listening for players connecting
		while(1){
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		printf("client connected\n");
		fflush(stdout);
		if (newsockfd < 0) 
			error("ERROR on accept");
		bzero(buffer1, sizeof(buffer1));
		sprintf(buffer1, "Welcome to the guessing game! In this game, you will guess a number from 1-50 three times each time trying to guess a randomly chosen number. You will be playing against 4 other players, each round one player with the worst guess will be eliminated from the game and their guess will be displayed to the remaining players. The player will the closest guess will win. \nPlease wait while other players connect... \n");
								
		n = write(newsockfd, buffer1, strlen(buffer1));
		//increase player count by 1. 
		data[playercounter] = newsockfd; //save client address to data table
		playercounter++;
		if (playercounter % numplay==0){
			pid_t pid = fork();
			int playercount = numplay;
			if(pid==0){
				int r = rand() % 50 + 1; //random int between 1-50
				//printi("rand = %d\n",r);fflush(stdout);
				int i;
				 
				int status[] = {1,1,1,1,1}; //keeps track of if the player is still in the game. 
				int guesses[] = {0,0,0,0,0}; 
				int dist[] = {0,0,0,0,0};
				//parallel for to deal with each player at the same time
				int j;
				for(j=0;j<3;j++){
					#pragma omp parallel	
					for(i=0;i<playercount;i++){
					char buffer[1000];
					if(status[i]!=0){
							if(j==0){
							bzero(buffer,sizeof(buffer));
							sprintf(buffer,"All players have joined the game, please enter a number from 1-50:");	
							n = write(data[i+playercounter-numplay], buffer, strlen(buffer));sleep(1);
						}
						else{
							bzero(buffer,sizeof(buffer));
                                                        sprintf(buffer,"Round %d, please enter a number from 1-50:",j+1);
                                                        n = write(data[i+playercounter-numplay], buffer, strlen(buffer));sleep(1);

						}
						// Receive answer from client
						bzero(buffer, sizeof(buffer));
						n = read(data[i+playercounter-numplay], buffer, sizeof(buffer));
						int guess = 0;
						sscanf(buffer, "%d", &guess);
						guesses[i] = guess;
						//printf("%d\n",guesses[i]);fflush(stdout);
                                                dist[i] = sqrt((guesses[i]-r)*(guesses[i]-r));
						//printf("%d\n",dist[i]);fflush(stdout);
						if(dist[i] == 0)
							status[i] = 2;
						bzero(buffer, sizeof(buffer));
                				sprintf(buffer,"Please wait while other players respond...\n");
						n = write(data[i+playercounter-numplay], buffer, strlen(buffer));sleep(1);
					}
					}
					//int m =0;
					//for(m = 0;m<playercount;m++){
					//	printf("%d",status[m]);
					//}
					//printf("\n");fflush(stdout);	
					int high = -1;
					int low = dist[1];
					int highloc = 0;
					int lowloc = 0;
					int k = 0;
					for(k=0;k<playercount;k++){
						if(status[k] == 2){
							//printf("pos:%d",k);fflush(stdout);
							int l =0;
							for(l = 0; l<playercount;l++){
								if(l!=k){
									status[l] = 0;
									//printf("pos %d changed to 0\n",l);
								}
							}
							break;
						}
						else if(status[k]!=0){
						
							if(high < dist[k]&&j!=2){
								high = dist[k];
								highloc = k;
							}
							if(low > dist[k]){
								low = dist[k];
								lowloc = k;
							}
						}
						
					}
					if(j==2){
						for(k=0;k<playercount;k++){
							if(k != lowloc)
								status[k] = 0;		
						}
					}
					else if(high != -1){
					status[highloc] = 0;
					}
					//for(m = 0;m<playercount;m++){
                                          //      printf("%d",status[m]);
                                        //}
					//printf("\n");fflush(stdout);

					#pragma omp parallel
					for(i=0;i<playercount;i++){
					char buffer2[1000];
						if(status[i] == 2){
							bzero(buffer2,sizeof(buffer2));
                                                        sprintf(buffer2,"Congratulations! You guessed the right number. You won! The actual number was %d.\n",r);
                                                        n = write(data[i+playercounter-numplay], buffer2, strlen(buffer2));
                                                        close(data[i+playercounter-numplay]);

						}
						else if (status[i] == 1){
							if(j!=2){
								bzero(buffer2,sizeof(buffer2));
                                                		sprintf(buffer2,"The worst guess was %d.\n",guesses[highloc]);
                                                		n = write(data[i+playercounter-numplay], buffer2, strlen(buffer2));sleep(1);
							}
							else{
								bzero(buffer2,sizeof(buffer2));
                                                                sprintf(buffer2,"Congratulations! You had the closest guess. You won! The actual number was %d.\n",r);
                                                                n = write(data[i+playercounter-numplay], buffer2, strlen(buffer2));
								close(data[i+playercounter-numplay]);
							}
						}
						else{
							bzero(buffer2,sizeof(buffer2));
                                                        sprintf(buffer2,"Game Over. Your guess was the worst of the round. The actual number was %d.\n",r);
                                                        n = write(data[i+playercounter-numplay], buffer2, strlen(buffer2));
							close(data[i+playercounter-numplay]);
							
							
						}
					}sleep(1);
				}

			}
			
		
		}
	}
	

	// Cleanup
    return 0; 
}
