#include <iostream>
#include <map>

#include <sys/socket.h>

#include "SDL2/SDL_thread.h"

#include <string.h>
#include <stdio.h>
#include <limits>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include "Logger/Log.h"
#include "Utils/queueManager.h"
#include "Parser/Parser.h"


using namespace std;
using namespace Parser;
using namespace queueManager;

SDL_mutex *mutexQueue;
SDL_mutex *mutexCantClientes;
SDL_mutex *mutexQueueCliente;
int cant_con, input_queue;
map<int,int> socket_queue;
Log slog;
bool quit=false;


//====================================================================================================

struct bufferMessage structMessage(char *buffer){

 	 bufferMessage msg;

 	 int OffsetId = lengthMessage;
 	 int OffsetType= lengthMessage + lengthId ;
 	 int OffsetData= lengthMessage + lengthId + lengthType;

	 bzero(msg.id,lengthId+1);
	 bzero(msg.type,lengthType+1);
	 bzero(msg.data,lengthData+1);

	 strncpy(msg.id,buffer+OffsetId,lengthId);
	 strncpy(msg.type,buffer+OffsetType,lengthType);
	 strncpy(msg.data,buffer+OffsetData,lengthData);

 return msg;
}

bool writeQueueMessage(int socket, int msgid, bufferMessage message, bool socketQueue){
//When socketQueue = true writes on the socket queue. If its false, writes on the input queue
	msg_buf msg;
	int msgqid;

	// message to send
	msg.mtype = msgid;
	msg.msocket = socket;
	
	sprintf (msg.minfo.data, "%s", message.data);
	sprintf (msg.minfo.id, "%s", message.id);
	sprintf (msg.minfo.type, "%s", message.type);
	
	msgqid = (socketQueue) ? socket_queue[socket] : input_queue;


	return sendQueueMessage(msgqid,msg);

}

bool check_integer(const char* value) {
  for(unsigned int i = 0; i < strlen(value); ++i) {
    if(!isdigit(value[i])){
	if((value[i]!='-')or (i!=0)){
	return false;
		}	
	}
      
  }
  return true;
}

bool check_double(const char* value) {
	char *p;
	strtod(value, & p );
	return (* p == 0 );
}

bool check_char(const char* value) {
	return strlen(value) == 1;
}

bool validate_message(bufferMessage message ){

	switch(message.type[0]){
		case 'i':
			slog.writeLine("validate_message | Validating Integer");
			return check_integer(message.data);
			break;
		case 'd':
			slog.writeLine("validate_message | Validating Double");
			return check_double(message.data);
			break;
		case 'c':
			slog.writeLine("validate_message | Validating Char");
			return check_char(message.data);
			break;
		case 's':
			slog.writeLine("validate_message | Validating String");
			return true;
			break;
		default:
			return false;
		}
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
			slog.writeLine("processMessages | Processing input queue message: " + string(msg.minfo.data));
			
			long response = (validate_message(msg.minfo))? 1 : 2;
			//Writes the message in the socket's queue
			writeQueueMessage(msg.msocket,response, msg.minfo, true);
		}
	}

	return 0;

}


bool doReadingError(int n, int sock, char* buffer){
   bufferMessage msgExit;
   sprintf (msgExit.data, "%s", "Client Connection Closed");	
   sprintf (msgExit.type, "%s", "N");	
   sprintf (msgExit.id, "%s", "null");	
   bool finish=false;
		if (n < 0) {
			slog.writeErrorLine("doReadingError | ERROR reading from socket");
			writeQueueMessage(sock,99, msgExit, true);
			finish = true;
	   	}else if (n == 0){
			//Exit message
			slog.writeLine("doReadingError | Exit message received");
			writeQueueMessage(sock,99, msgExit, true);
			finish = true;
	   	}else{
			 if ((n == 1) & (string(buffer) == "q")){
				//Exit message
				slog.writeLine("doReading | Quit message received");
				writeQueueMessage(sock,99, msgExit, true);
				finish = true;
			 }else {
				slog.writeLine("doReading | Client send this message: " + string(buffer));				
			}
		}

		return finish;

}

//put the message into mainqueuemessages ( with lock & unlock mutex)
bool insertingMessageQueue(int sock, char *buffer){
	bufferMessage msg;
	bool finish=false;
      
				
	//mutex lock.
	if (SDL_LockMutex(mutexQueue) == 0) {
		slog.writeLine("insertingMessageQueue | Inserting message '" + string(buffer) + "' into clients queue");
		msg = structMessage(buffer);
		finish = !writeQueueMessage(sock,1, msg,false);
	 //mutex unlock
	 SDL_UnlockMutex(mutexQueue);
	}
				
	return finish;
}



static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[999];
   
   int sock = *(int*)sockfd;
   free(sockfd);
   char messageLength[3];
   int messageSize;
   char bufferingMessage[999];
  
   //Receive a message from client
   while(!finish && !quit){
	   //Read client's message
		bzero(bufferingMessage,999);
		bzero(buffer,999);
   		n = recv(sock,buffer,998,0);
		//handle conection Exit client,Error reading
		finish=doReadingError(n,sock,buffer);
	        	
		if (!finish){
			//getting the messageLength
			strncpy(messageLength,buffer,3);
			messageSize=stoi(messageLength,nullptr,10);

			if (n==messageSize){//full message received.
				finish=insertingMessageQueue(sock,buffer);
			}else{//message incomplete.
				int readed=n;
				cout << messageSize << endl;
				while ( readed !=messageSize){
					n =recv(sock,buffer+readed,messageSize-readed,0);
					cout << "loopeando dentro del doreading" << endl;					
					readed+=n;
				}
				finish=insertingMessageQueue(sock,buffer);
			 }
		}

   	} // END WHILE
	if (!quit){
	if (SDL_LockMutex(mutexCantClientes) == 0) {
         cant_con--;
 	 SDL_UnlockMutex(mutexCantClientes);	
	}

	close(sock);
}
	return 0;
}

static int doWriting (void *sockfd) {
   int msgqid, n;
   bool finish = false;
   msg_buf msg;

   int sock = *(int*)sockfd;
   free(sockfd);

   //Receive a message from client
     while(!finish && !quit)
     {
		msgqid = socket_queue[sock];
		if (!receiveQueueMessage(msgqid, msg)) {
			return 1;
		}

		if (msg.mtype == 99){
			//Exit message received
			slog.writeLine("doWriting | Exit message received: " + string(msg.minfo.data));
			socket_queue.erase(sock);
			finish = true;
		}else{
			slog.writeLine("doWriting | Received queue message: " + string(msg.minfo.data));
			//Respond to the client
			const char* message = (msg.mtype == 1)?"Message is correct. I processed your message \n":"ERROR: Message is not correct. I can't process your message \n";
			if (msg.mtype == 1){
			slog.writeLine(message);
			}else{
			slog.writeErrorLine(message);
			}
			if(!quit){
			n = send(sock,message,strlen(message),0);
			if (n < 0) {
			  slog.writeErrorLine("doWriting | ERROR writing to socket");
			  finish = true;
			}
			}
		}
     }

     return 0;
}

void createAndDetachThread(SDL_ThreadFunction fn, const char *name, int data){
	SDL_Thread *thread;
	int *data_dir;

	data_dir = (int *)malloc(sizeof(int));
	*data_dir = data;

	thread = SDL_CreateThread(fn,name, data_dir);
	if (NULL == thread) {
		slog.writeLine("SDL_CreateThread failed: " +  string(SDL_GetError()) + "");
	}
	SDL_DetachThread(thread);

}

void manageNewConnection(int newsockfd){

	int msgqid;

	//Create the output messages queue for the socket
	if (getQueue(msgqid)) {
		socket_queue[newsockfd] = msgqid;
		createAndDetachThread(doReading,"doReading", newsockfd);
		createAndDetachThread(doWriting,"doWriting", newsockfd);
 	}
}

int openAndBindSocket(int port_number){
	int sockfd;
	struct sockaddr_in serv_addr;

	// Opening the Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		slog.writeErrorLine("openAndBindSocket | ERROR opening socket");
		slog.writeLine("Application closed.");
		exit(1);
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		slog.writeErrorLine("openAndBindSocket | ERROR configuring socket as reusable");

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);

	// Binding server address with socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  slog.writeErrorLine("openAndBindSocket | ERROR on binding");
	  slog.writeLine("Application closed.");
	  exit(1);
	}

	//Listen through the socket
	listen(sockfd,5);

	return sockfd;
}


static int exitManager (void *data) {
	  string input;
	  //bool quit = false;

	  cout << "Type 'quit' any time to exit \n";
	  while (!quit){
		  getline (cin, input);
		  quit = (input == "quit");
	  }

	  slog.writeLine("Exit signal received. Closing application...");
	  SDL_LockMutex(mutexCantClientes);
	  SDL_LockMutex(mutexQueue);
	  for(auto const &it : socket_queue) {
		  close(it.first);
		  cant_con --;
	  }
	 
	  
	  slog.writeLine("Application closed.");

	  exit(0);
	  return 0;
}

void leerXML(int &cantMaxClientes, int &puerto){

	type_datosServer xml;
	string path;

	cout << "Please enter the XML config path, or 'default' to use default file.:" << endl;
	cin >> path;
	if (path == "default"){
		slog.writeWarningLine("Se toma el XML de default");
		path = getDefaultNameServer();
	}

	while (!fileExists(path.c_str())){
		cout << "Invalid path. Pease enter the XML config path, or 'default' to use default file." << endl;
		cin >> path;
		if (path == "default"){
			slog.writeWarningLine("Se toma el XML de default");
			path = getDefaultNameServer();
		}

	}

	xml = parseXMLServer(path.c_str(), &slog);

	cantMaxClientes = xml.cantMaxClientes;
	puerto = xml.puerto;
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in cli_addr;
   	mutexQueue = SDL_CreateMutex();
	mutexCantClientes = SDL_CreateMutex();
	mutexQueueCliente = SDL_CreateMutex();
	cant_con = 0;

	//Log initialize
	slog.createFile(3);

	slog.writeLine("Starting...");

	leerXML(max_con,port_number);
	slog.writeLine("Port: " + to_string(port_number));
	slog.writeLine("Max connections: " + to_string(max_con));

	createAndDetachThread(exitManager,"exitManager", 0);
	sockfd = openAndBindSocket(port_number);

	createAndDetachThread(processMessages,"processMessages", 0);

	cli_len = sizeof(cli_addr);

	 while (1) {

		 cout << "Current connections: " << cant_con << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);
		 struct timeval timeout;
		 timeout.tv_sec = 10;
		 timeout.tv_usec = 0;

		 if (setsockopt (newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			 slog.writeErrorLine("ERROR setting socket rcv timeout");

		 if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		 	slog.writeErrorLine("ERROR setting socket snd timeout");

		 if (newsockfd < 0) {
			slog.writeErrorLine("ERROR on accept");
		 }else{
			if (cant_con == max_con){
				send(newsockfd,"ERROR: The Server has exceeded max number of connections. \n",60,0);
				close(newsockfd);
			}else{
				send(newsockfd,"Connected \n",12,0);
				cant_con++;
				manageNewConnection(newsockfd);
			}
		}
	}
        SDL_DestroyMutex(mutexQueue);
	SDL_DestroyMutex(mutexCantClientes);
    return 1;
}
