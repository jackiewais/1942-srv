#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "messages.h"
#include "../Elemento/Elemento.h"

int msg1len = typel + idl + posl + posl,
	msg2len = typel + idl + posl + posl + infol,
	msg3len = typel + idl,
	msg8len = typel + idl + infol;

void intToFixedChar(int n, char* p, int length){
	
	for (char* it = p + length - 1; it >= p; it--){
		*it = '0' + (n % 10);
		n = n/10;
	}

}	

int decodeMessages(struct gst*** msgs, char* msgsChar){

	int nOfMsgs;

	char nOfMsgsChar[nOfMsgsl],
		  *charIdx = msgsChar;

	charIdx += lengthl;
	memcpy(nOfMsgsChar, charIdx, nOfMsgsl);
	nOfMsgs = atoi(nOfMsgsChar);
	charIdx += nOfMsgsl;

	*msgs = new struct gst*[nOfMsgs];
	struct gst** msgIdx = *msgs;

	for (int i = 0; i < nOfMsgs; i++){

		*msgIdx = new struct gst;

		memcpy((*msgIdx) -> type, charIdx, typel);
		charIdx += typel;

		memcpy((*msgIdx) -> id, charIdx, idl);
		charIdx += idl;

		if ((*msgIdx) -> type[0] == '1' || (*msgIdx) -> type[0] == '2'){
			memcpy((*msgIdx) -> posx, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posy, charIdx, posl);
			charIdx += posl;
		}

		if ((*msgIdx) -> type[0] == '2' || (*msgIdx) -> type[0] == '8'){
			memcpy((*msgIdx) -> info, charIdx, infol);
			charIdx += infol;
		}
		else if ((*msgIdx) -> type[0] != '3')
			return -1;

		msgIdx++;
	}

	return nOfMsgs;
}




int encodeMessages(char** msgsChar, struct gst** msgs, int qty){

	int pkglen = lengthl + nOfMsgsl;

	*msgsChar = new char[999];
	char* buffer = *msgsChar;
	char* bufferIt = buffer + lengthl;
	
	//copio la cantidad de mensajes
	intToFixedChar(qty, bufferIt, nOfMsgsl);
	bufferIt += nOfMsgsl;
	
	for (int i = 0; i < qty; i++){
		*bufferIt++ = (*msgs) -> type[0];
		
		memcpy(bufferIt, (*msgs) -> id, idl);
		bufferIt += idl;
		
		pkglen += typel + idl;

		if ((*msgs) -> type[0] == '1' || (*msgs) -> type[0] == '2'){
			memcpy(bufferIt, (*msgs) -> posx, posl);
			bufferIt += posl;
			memcpy(bufferIt, (*msgs) -> posy, posl);
			bufferIt += posl;

			pkglen += posl + posl;
		}

		if ((*msgs) -> type[0] == '2' || (*msgs) -> type[0] == '8'){
			memcpy(bufferIt, (*msgs) -> info, infol);
			bufferIt += infol;
	
			pkglen += infol;
		}

		msgs++;
		
	}


	intToFixedChar(pkglen, buffer, lengthl);
	return pkglen;
}

struct gst* genUpdateGstFromElemento(Elemento* elemento){

	struct gst* newMsg = new struct gst;

	newMsg -> type[0] = (char) msgType::UPDATE;

	char charId[idl], charPos[posl];
	intToFixedChar(elemento->getId(), charId, idl);
	memcpy(newMsg -> id, charId, idl);

	intToFixedChar(elemento -> getPosX(), charPos, posl);
	memcpy(newMsg -> posx, charPos, posl);

	intToFixedChar(elemento -> getPosY(), charPos, posl);
	memcpy(newMsg -> posy, charPos, posl);

	newMsg -> info[0] = (char) elemento -> getEstado();
	return newMsg;
}

struct gst* genAdminGst(int clientId, command comando){

	struct gst* newMsg = new struct gst;

	newMsg -> type[0] = (char) msgType::CONTROL;

	char charId[idl];
	intToFixedChar(clientId, charId, idl);
	memcpy(newMsg -> id, charId, idl);

	newMsg -> info[0] = (char) comando;
	return newMsg;
}
