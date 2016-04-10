namespace queueManager{

		struct msg_buf {
		  long mtype;
		  struct msg_content {
			  int msocket;
			  char data[256];
		          char id[10];
		          char type[1];
		  } minfo;
		};

		bool sendQueueMessage(int msgqid, msg_buf msg);
		bool getQueue(int &queue);
		bool receiveQueueMessage(int queue, msg_buf &msg);

};
