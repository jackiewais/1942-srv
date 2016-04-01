#include <iostream>
#include <map>

#include <sys/socket.h>

#include "SDL2/SDL_thread.h"

#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <unistd.h>

#include "queueManager.h"

using namespace std;

using namespace queueManager;


int cant_con, input_queue;
map<int,int> socket_queue;


bool writeQueueMessage(int socket, int msgid, char* message, bool socketQueue){
//When socketQueue = true writes on the socket queue. If its false, writes on the input queue

	msg_buf msg;
	int msgqid;

	// message to send
	msg.mtype = msgid;
	msg.minfo.msocket = socket;
	sprintf (msg.minfo.mtext, "%s\n", message);

	msgqid = (socketQueue) ? socket_queue[socket] : input_queue;

	return sendQueueMessage(msgqid,msg);
}

static int processMessages (void *data) {
	msg_buf msg;
	bool finish = false;

	if (!getQueue(input_queue)) {
		return 1;
	}

	while(!finish){
		if (!receiveQueueMessage(input_queue, msg)) {
			return 1;
		}else{
			//Validate Message
			//Process Message

			printf("processMessages | Processing input queue messsage: %s\n", msg.minfo.mtext);
			//Writes the message in the socket's queue
			writeQueueMessage(msg.minfo.msocket,msg.mtype, msg.minfo.mtext, true);
		}
	}

	return 0;

}

static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[256];

   int sock = *(int*)sockfd;

   //Receive a message from client
   while(!finish){
	   //Read client's message
		bzero(buffer,256);
		n = recv(sock,buffer,255,0 );
		if (n < 0) {
			perror("doReading | ERROR reading from socket \n");
			finish = true;
		}

		if (n == 0){
			//Exit message
			printf("doReading | Exit message received \n");
			writeQueueMessage(sock,99, (char*) "Client Connection Closed", true);
			finish = true;
			cant_con--;
		}else{
			printf("doReading | Client send this message: %s \n",buffer);

			//Respond OK to the client
			n = send(sock,"I got your message \n",21,0);
			if (n < 0) {
				perror("doReading | ERROR writing to socket \n");
				finish = true;
			}

			if (!finish){
				//Puts the message in the input queue
				printf("doReading | Inserting message '%s' into clients queue \n",buffer);
				finish = !writeQueueMessage(sock,1, buffer,false);
			}
		}
     }

	free(sockfd);
	close(sock);
	return 0;
}

static int doWriting (void *sockfd) {
   int msgqid, n;
   bool finish = false;
   msg_buf msg;

   int sock = *(int*)sockfd;

   //Receive a message from client
     while(!finish)
     {
		msgqid = socket_queue[sock];
		if (!receiveQueueMessage(msgqid, msg)) {
			return 1;
		}

		if (msg.mtype == 99){
			//Exit message received
			printf("doWriting | Exit message received: %s\n", msg.minfo.mtext);
			socket_queue.erase(sock);
			finish = true;
		}else{
			printf("doWriting | Received queue message: %s\n", msg.minfo.mtext);
			//Respond to the client
			n = send(sock,"I processed your message \n",26,0);
			if (n < 0) {
			  perror("doWriting | ERROR writing to socket \n");
			  finish = true;
			}
		}
	}

	return 0;
}

void createAndDetachThread(SDL_ThreadFunction fn, const char *name, void *data){
	SDL_Thread *thread;

	thread = SDL_CreateThread(fn,name, data);
	if (NULL == thread) {
		printf("\nSDL_CreateThread failed: %s", SDL_GetError());
	}
	SDL_DetachThread(thread);

}

void manageNewConnection(int newsockfd){

	int *newsock_dir;
	int msgqid;

	//Create the output messages queue for the socket
	if (getQueue(msgqid)) {
		socket_queue[newsockfd] = msgqid;

		newsock_dir = (int *)malloc(sizeof(int));
		*newsock_dir = newsockfd;

		createAndDetachThread(doReading,"doReading", newsock_dir);
		createAndDetachThread(doWriting,"doWriting", newsock_dir);
	}
}

int openAndBindSocket(int port_number){
	int sockfd;
	struct sockaddr_in serv_addr;

	// Opening the Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	 perror("openAndBindSocket | ERROR opening socket");
	 exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);

	// Binding server address with socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("openAndBindSocket | ERROR on binding");
	  exit(1);
	}

	//Listen through the socket
	listen(sockfd,5);

	return sockfd;
}

int main()
{
	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in cli_addr;

	cant_con = 0;

	//Parameter definition
	port_number = 5001;
	max_con = 2;


	printf("Starting... \n");

	sockfd = openAndBindSocket(port_number);

	createAndDetachThread(processMessages,"processMessages", NULL);

	cli_len = sizeof(cli_addr);

	 while (1) {

		printf("Current connections: %i \n", cant_con);

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);
		 if (newsockfd < 0) {
			perror("ERROR on accept");
		 }else{

			if (cant_con == max_con){
				send(newsockfd,"ERROR: The Server has exceeded max number of connections. \n",60,0);
				close(newsockfd);
			}else{
				cant_con++;
				manageNewConnection(newsockfd);
			}
		}
	}

    return 1;
}
