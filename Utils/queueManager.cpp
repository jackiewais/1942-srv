#include "queueManager.h"

#include <sys/msg.h>
#include <sys/errno.h>

#include <string.h>
#include <stdio.h>

extern int errno;

using namespace queueManager;


namespace queueManager{
bool sendQueueMessage(int msgqid, msg_buf msg){
	int rc;

	rc = msgsnd(msgqid, &msg, sizeof(msg.minfo), 0);
	if (rc < 0) {
		perror(strerror(errno));
		printf("sendQueueMessage | ERROR: failed sending message.\n");
		return false;
	}

	return true;
}

bool getQueue(int &queue){

	queue = msgget(IPC_PRIVATE, 0666|IPC_CREAT|IPC_EXCL);
	if (queue < 0) {
	  perror(strerror(errno));
	  printf("getQueue | ERROR: failed to create message queue \n");
	  return false;
	}
	return true;
}

bool receiveQueueMessage(int queue, msg_buf &msg){
	int rc;

	rc = msgrcv(queue, &msg, sizeof(msg.minfo), 0, 0);
	if (rc < 0) {
		perror( strerror(errno) );
		printf("receiveQueueMessage | ERROR: failed to receive message \n");
		return false;
	}

	return true;
}
}
