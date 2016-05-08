#include "messages.h"

namespace queueManager{

		struct msg_buf {
		  int msocket;
		  struct gst** minfo;
		  int msgQty;
		};

		bool sendQueueMessage(int msgqid, msg_buf msg);
		bool getQueue(int &queue);
		bool receiveQueueMessage(int queue, msg_buf &msg);

};
