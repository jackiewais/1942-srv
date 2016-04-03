namespace queueManager{

		struct msg_buf {
		  long mtype;
		  struct msg_content {
			  int msocket;
			  char mtext[255];
		  } minfo;
		};

		bool sendQueueMessage(int msgqid, msg_buf msg);
		bool getQueue(int &queue);
		bool receiveQueueMessage(int queue, msg_buf &msg);

};
