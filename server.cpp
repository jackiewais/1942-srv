#include <iostream>
#include <map>

#include <sys/socket.h>

#include "SDL2/SDL_thread.h"

#include <string>
#include <stdio.h>
#include <limits>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include "Elemento/Elemento.h"
#include "Logger/Log.h"
#include "Utils/queueManager.h"
#include "Utils/messages.h"
#include "Parser/Parser.h"

#define BUFLEN 1000

using namespace std;
using namespace Parser;
using namespace queueManager;

SDL_mutex *mutexQueue;
SDL_mutex *mutexCantClientes;
SDL_mutex *mutexClientState;
int cant_con, input_queue, cantJug, audit;
map<int,int> socket_queue;
map<int,bool> client_state;
map<int,Elemento*> elementos;
map<string,int> users;
Log slog;
bool quit=false;
type_Ventana ventana;
list<type_Sprite> sprites;
type_Escenario escenario;
type_Avion jugador;
string pathXMLEscenario = "escenario.xml";


//====================================================================================================


bool writeQueueMessage(int socket, struct gst** messages, int msgQty, bool socketQueue){
//When socketQueue = true writes on the socket queue. If its false, writes on the input queue
	msg_buf msg;
	int msgqid;

	// message to send
	msg.msocket = socket;
	msg.minfo = messages;
	msg.msgQty = msgQty;
	
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

void buscarPathSprite(Parser::spriteType idSprite, char path[pathl]) {
	list<type_Sprite>::iterator it;
	bool encontrado = false;

	for (it = sprites.begin(); it != sprites.end(); it ++) {
		if ((it)->id == idSprite) {
			encontrado = true;
			memset(path, '\0', pathl);
			memcpy(path, (it)->path, pathl);
		}
	}
	if (!encontrado)
		memcpy(path, "-", pathl);
}

int _processMsgs(struct gst** msgs, int socket, int msgQty, struct gst*** answerMsgs){

	//cout << "DEBUG entro a _processMsgs" << endl;

	int answerMsgsQty, tempId;
	Elemento* tempEl;
	bool newEvent = false, oldEvent = false, isEscenario = false;
	status event, prevStatus;

	for (int i = 0; i < msgQty; i++){

		if (msgs[i] -> type[0] == '2'){
			tempId = atoi(msgs[i]-> id);
			tempEl = elementos[tempId];
			//cout << "DEBUG _processMsgs esta queriendo actualizar" << endl;
			if (tempEl == NULL){
				cout << "recibido elemento inexistente " << endl;
				cout << "id = " << tempId << endl;
			}else {
				if ((msgs[i] -> info[0] == (char) status::START) ||
					(msgs[i] -> info[0] == (char) status::RESET) ||
					(msgs[i] -> info[0] == (char) status::PAUSA)){

					//cout << "_processMsgs DEBUG nuevoEvento = " << msgs[i] -> info[0] << endl;

					newEvent = true;
					event = (status) msgs[i] -> info[0];

					prevStatus = tempEl -> getEstado();
					tempEl-> update(msgs[i]);
					tempEl-> updateStatus(prevStatus);
				}

				else if ((tempEl -> getEstado() == status::START) ||
						(tempEl -> getEstado() == status::RESET) ||
						(tempEl -> getEstado() == status::PAUSA)){

					//cout << "_processMsgs DEBUG viejoEvento = " << (char)tempEl -> getEstado() << endl;
					oldEvent = true;
					event = tempEl -> getEstado();
					tempEl-> update(msgs[i]);

				}
				else{
					//cout << "_processMsgs DEBUG info = " << msgs[i] -> info[0] << endl;
					tempEl-> update(msgs[i]);
				}
				//cout << "DEBUG _processMsgs actualizo el elemento" << endl;

			}
		}
		else if (msgs[i] -> type[0] == '8'){
			if (msgs[i] -> info[0] == (char) command::DISCONNECT){
				if (SDL_LockMutex(mutexClientState) == 0) {
					client_state[socket] = false;
					SDL_UnlockMutex(mutexClientState);
				}
			}
			if (msgs[i] -> info[0] == (char) command::REQ_SCENARIO) {
				isEscenario = true;
			}
		}
		delete msgs[i];
	}
	if (isEscenario) {
		char pathFondo[pathl], pathElemento[pathl];
		memset(pathFondo, '=', pathl);
		memset(pathElemento, '=', pathl);
		int i = 2;
		buscarPathSprite(escenario.fondo.spriteId, pathFondo);

		answerMsgsQty = escenario.elementos.size() + 2;
		*answerMsgs = new struct gst*[answerMsgsQty];
		struct gst** answerIt = (*answerMsgs);
		answerIt[0] = genGstFromFondo(&escenario, pathFondo);
		answerIt[1] = genGstFromVentana(&ventana);
		list<type_Elemento>::iterator ite;
		for (ite = escenario.elementos.begin(); ite != escenario.elementos.end(); ite ++) {
			buscarPathSprite((ite) -> spriteId, pathElemento);
			answerIt[i] = genGstFromElemento(&(*ite), pathElemento);
			i++;
		}
	} else {
		answerMsgsQty = elementos.size();
		*answerMsgs = new struct gst*[answerMsgsQty];
		struct gst** answerIt = (*answerMsgs);
		map<int,Elemento*>::iterator elementosIt;
		elementosIt = elementos.begin();
		for (int i = 0; i < answerMsgsQty; i++){
	
			answerIt[i] = genUpdateGstFromElemento(elementosIt -> second);
	
			if ((newEvent || oldEvent) && (elementosIt -> first == tempId)){
				cout << "_processMsgs DEBUG evento ori = " << answerIt[i] -> info[0] << endl;
				answerIt[i] -> info[0] = (char) event;
				cout << "_processMsgs DEBUG evento mod = " << answerIt[i] -> info[0] << endl;
			}
			else if (newEvent){
				elementosIt -> second -> updateStatus(event);
			}
		
			elementosIt++;
		}
	}

	return answerMsgsQty;

}


static int processMessages (void *data) {
	msg_buf msg;

	bool finish = false;
	
	if (!getQueue(input_queue)) {
		return 1;
	}

	struct gst** answerMsgs;
	int msgQty;

	while(!finish){
		if (!receiveQueueMessage(input_queue, msg)) {
			return 1;
		}else{
			//slog.writeLine("processMessages | Processing input queue message: " + string(msg.minfo.data));
			//long response = (validate_message(msg.minfo))? 1 : 2;
			msgQty = _processMsgs(msg.minfo,msg.msocket, msg.msgQty, &answerMsgs);

			//Writes the message in the socket's queue
			writeQueueMessage(msg.msocket, answerMsgs, msgQty, true);
			//cout << "DEBUG processMessages se escribio en la queue de escritura" << endl;
		}
	}

	return 0;

}


bool doReadingError(int n, int sock, char* buffer){

	struct gst* msgExit = genAdminGst(0, command::DISCONNECT);
	bool finish=false;

	if (n < 0) {
		slog.writeErrorLine("doReadingError | ERROR reading from socket");
		writeQueueMessage(sock, &msgExit, 1, true);
		finish = true;

		client_state[sock] = false;
	}

	else if (n == 0){
		//Exit message
		slog.writeLine("doReadingError | Exit message received");
		writeQueueMessage(sock, &msgExit, 1, true);
		finish = true;
	}/*else{
		 if ((n == 1) & (string(buffer) == "q")){
			//Exit message
			slog.writeLine("doReading | Quit message received");
			writeQueueMessage(sock,99, msgExit, true);
			finish = true;
		 }else {
			slog.writeLine("doReading | Client send this message: " + string(buffer));
		}
	}*/

	return finish;

}

//put the message into mainqueuemessages ( with lock & unlock mutex)
bool insertingMessageQueue(int sock, char *buffer){
	struct gst** msgs;
	int msgQty;
	bool finish=false;
      
				
	//mutex lock.
	if (SDL_LockMutex(mutexQueue) == 0) {
		//slog.writeLine("insertingMessageQueue | Inserting message '" + string(buffer) + "' into clients queue");
		msgQty = decodeMessages(&msgs, buffer);
		finish = !writeQueueMessage(sock, msgs, msgQty, false);
	 //mutex unlock
	 SDL_UnlockMutex(mutexQueue);
	}
				
	return finish;
}



/*static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[BUFLEN];
   
   int sock = *(int*) sockfd;
   free(sockfd);
   char messageLength[3];
   int messageSize;
   char bufferingMessage[BUFLEN];
  
   //Receive a message from client
   while(!finish && !quit){

	   //cout << "DEBUG doReading sock = " << sock << endl;
	   //Read client's message
		bzero(bufferingMessage,BUFLEN);
		bzero(buffer,BUFLEN);
   		n = recv(sock, buffer, BUFLEN-1, 0);
		if (audit)
			cout << "AUDIT rcv: " << buffer << endl;
		//handle Error reading
		finish = doReadingError(n, sock, buffer);
	        	
		if (finish){
			if (SDL_LockMutex(mutexClientState) == 0) {
				client_state[sock] = false;
				SDL_UnlockMutex(mutexClientState);
			}

		}
		else{
			//getting the messageLength
			strncpy(messageLength,buffer,3);
			messageSize=atoi(messageLength);

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
			if (SDL_LockMutex(mutexClientState) == 0) {
				finish = !client_state[sock];
				SDL_UnlockMutex(mutexClientState);
				//cout << "DEBUG doReading finish = " << finish << endl;
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
}*/

static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[BUFLEN];

   int sock = *(int*) sockfd;
   free(sockfd);
   char messageLength[3];
   int messageSize;
   char bufferingMessage[BUFLEN];

   //Receive a message from client
   while(!finish && !quit){

       //cout << "DEBUG doReading sock = " << sock << endl;
       //Read client's message
        bzero(bufferingMessage,BUFLEN);
        bzero(buffer,BUFLEN);
           n = recv(sock, buffer, BUFLEN-1, 0);
           //cout << "DEBUG DOREADING n = " << n << endl;
        if (audit)
            cout << "AUDIT rcv: " << buffer << endl;
        //handle Error reading
        finish = doReadingError(n, sock, buffer);

        if (finish){
            if (SDL_LockMutex(mutexClientState) == 0) {
                client_state[sock] = false;
                SDL_UnlockMutex(mutexClientState);
            }

        }
        else{
            //getting the messageLength
            strncpy(messageLength,buffer,3);
            char* bufferIt = buffer;
            while (!(messageLength[0] >= '0' && messageLength[0] <= '9') &&
                   !(messageLength[1] >= '0' && messageLength[1] <= '9') &&
                   !(messageLength[2] >= '0' && messageLength[2] <= '9') ){
                bufferIt++;
                n--;
                strncpy(messageLength,bufferIt,3);
                //cout << "DEBUG DOREADING buffer = " << bufferIt << endl;
            }

            messageSize=atoi(messageLength);

            if (n>=messageSize){//full message received.
                finish=insertingMessageQueue(sock,bufferIt);
            }else{//message incomplete.
                int readed=n;
                cout << messageSize << endl;
                while ( readed !=messageSize){
                    n =recv(sock,bufferIt+readed,messageSize-readed,0);
                    cout << "loopeando dentro del doreading" << endl;
                    readed+=n;
                }
                finish=insertingMessageQueue(sock,bufferIt);
             }
            if (SDL_LockMutex(mutexClientState) == 0) {
                finish = !client_state[sock];
                SDL_UnlockMutex(mutexClientState);
                //cout << "DEBUG doReading finish = " << finish << endl;
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

   int sock = *(int*) sockfd;

   bool* isSocketActive;
   if (SDL_LockMutex(mutexClientState) == 0) {
	   isSocketActive = &client_state[sock];
	   SDL_UnlockMutex(mutexClientState);
   }
   //cout << "DEBUG doWriting sock = " << sock << endl;
   free(sockfd);

   //Receive a message from client
     while(!finish && !quit)
     {
		msgqid = socket_queue[sock];
		if (!receiveQueueMessage(msgqid, msg)) {
			return 1;
		}

		if (!*isSocketActive){
			//Exit message received
			//slog.writeLine("doWriting | Exit message received: " + string(msg.minfo.data));
			socket_queue.erase(sock);
			finish = true;
		}else{
			//slog.writeLine("doWriting | Received queue message: " + string(msg.minfo.data));
			//Respond to the client
			char* message;
			//cout << "DEBUG doWriting esta queriendo encodear los mensajes" << endl;
			int messageLen = encodeMessages(&message, msg.minfo, msg.msgQty);
			//cout << "DEBUG doWriting encodeo los mensajes" << endl;

			if(!quit){
				n = send(sock,message,messageLen,0);
				if (audit)
					cout << "AUDIT snd: " << message << endl;
				if (n < 0) {
					slog.writeErrorLine("doWriting | ERROR writing to socket");
					finish = true;
				}
				delete message;
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
	usleep(1000*1000);

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
	  SDL_LockMutex(mutexClientState);
	  for(auto const &it : socket_queue) {
		  close(it.first);
		  cant_con --;
	  }
	 

      SDL_DestroyMutex(mutexQueue);
      SDL_DestroyMutex(mutexCantClientes);
      SDL_DestroyMutex(mutexClientState);
	  slog.writeLine("Application closed.");

	  exit(0);
	  return 0;
}

void leerXMLEscenario() {
	type_DatosGraficos xml;

	xml = parseXMLServerMap(pathXMLEscenario.c_str(), &slog);

	ventana = xml.ventana;
	sprites = xml.sprites;
	escenario = xml.escenario;
	jugador = xml.avion;
	cantJug = xml.cantJug;
	slog.writeLine("Cantidad de jugadores: " + to_string(cantJug));
}

void leerXML(int &cantMaxClientes, int &puerto, int &audit){

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
	audit = xml.audit;
}



Elemento* genNewPlayer(int playerId, string username){
	int anchoPantalla = 640 / 4;
	int altura = 50;
	users[username] = playerId;
	Elemento* newPlayer = new Elemento(playerId, anchoPantalla * playerId, altura);
	elementos[playerId] = newPlayer;

	return newPlayer;
}


int main(int argc, char **argv)
{
	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in cli_addr;
   	mutexQueue = SDL_CreateMutex();
	mutexCantClientes = SDL_CreateMutex();
	mutexClientState = SDL_CreateMutex();
	cant_con = 0;

	//Log initialize
	slog.createFile(3);

	slog.writeLine("Starting...");

	leerXML(max_con,port_number, audit);
	slog.writeLine("Port: " + to_string(port_number));
	slog.writeLine("Max connections: " + to_string(max_con));
	slog.writeLine("Audit: " + to_string(audit));

	slog.writeLine("Leyendo XML de configuracion del juego...");
	leerXMLEscenario();

	createAndDetachThread(exitManager,"exitManager", 0);
	sockfd = openAndBindSocket(port_number);

	createAndDetachThread(processMessages,"processMessages", 0);

	cli_len = sizeof(cli_addr);

	struct gst* conMsg[2];
	char* buffer;
	int playerId = 1, bufferLen, newId;
	string newUsername;

	 while (1) {

		 cout << "Current connections: " << cant_con << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);

		 struct timeval timeout;
		 timeout.tv_sec = 120;
		 timeout.tv_usec = 0;

		 if (setsockopt (newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			 slog.writeErrorLine("ERROR setting socket rcv timeout");

		 if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		 	slog.writeErrorLine("ERROR setting socket snd timeout");

		 if (newsockfd < 0) {
			slog.writeErrorLine("ERROR on accept");
		 }else{


			 conMsg[0] = genAdminGst(playerId, command::CON_SUCCESS);
			 bufferLen = encodeMessages(&buffer, conMsg, 1);
			 send(newsockfd, buffer, bufferLen ,0);
			 delete buffer;

			 //Leo el nombre
			 buffer = new char[BUFLEN];
			 bufferLen = recv(newsockfd, buffer, bufferLen, 0);

			 newUsername = string(buffer);
			 newId = users[newUsername];

			if ((cant_con == max_con)&&(!newId)){

				conMsg[0] = genAdminGst(0, command::CON_FAIL);
				bufferLen = encodeMessages(&buffer, conMsg, 1);
				send(newsockfd, buffer, bufferLen ,0);
				close(newsockfd);
				delete buffer;
			}else{

				if (!newId){
					conMsg[0] = genAdminGst(playerId, command::CON_SUCCESS);
					conMsg[1] = genUpdateGstFromElemento(genNewPlayer(playerId, newUsername));
				}
				else{
					conMsg[0] = genAdminGst(newId, command::CON_SUCCESS);
					conMsg[1] = genUpdateGstFromElemento(elementos[newId]);
				}

				bufferLen = encodeMessages(&buffer, conMsg, 2);
				if (audit)
					cout << "AUDIT snd: " << buffer << endl;
				send(newsockfd, buffer, bufferLen, 0);
				if (SDL_LockMutex(mutexCantClientes) == 0) {
					cant_con++;
					SDL_UnlockMutex(mutexCantClientes);
				}
				if (SDL_LockMutex(mutexClientState) == 0) {
				   	client_state[newsockfd] = true;
				   	SDL_UnlockMutex(mutexClientState);
				}
				manageNewConnection(newsockfd);
				cout << "El usuario " << newUsername << " se ha conectado corretamente." << endl;
				delete buffer;
				playerId++;

			}
		}
	}
    return 1;
}
