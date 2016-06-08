#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "messages.h"
#include "../Elemento/Elemento.h"

int msg1len = typel + idl + posl + posl,
	msg2len = typel + idl + posl + posl + infol,
	msg3len = typel + idl,
	msg8len = typel + idl + infol,
	msg5len = typel + idl + pathl,
	msg0lin = typel + idl + posl + posl + posl + posl + pathl;

void intToFixedChar(int n, char* p, int length){
	
	for (char* it = p + length - 1; it >= p; it--){
		*it = '0' + (n % 10);
		n = n/10;
	}

}

void cleanGst(struct gst* msg){
	memset(msg->type,0,typel +1);
	memset(msg->id,0,idl +1);
	memset(msg->ancho,0,posl +1);
	memset(msg->alto,0,posl +1);
	memset(msg->posx,0,posl +1);
	memset(msg->posy,0,posl +1);
	memset(msg->info,0,infol +1);
	memset(msg->path,0,pathl +1);

}

int decodeMessages(struct gst*** msgs, char* msgsChar){
	int nOfMsgs;

	char nOfMsgsChar[nOfMsgsl + 1],
		  *charIdx = msgsChar;

	charIdx += lengthl;
	nOfMsgsChar[nOfMsgsl] = 0;
	memcpy(nOfMsgsChar, charIdx, nOfMsgsl);
	nOfMsgs = atoi(nOfMsgsChar);
	charIdx += nOfMsgsl;
	*msgs = new struct gst*[nOfMsgs];
	struct gst** msgIdx = *msgs;

	for (int i = 0; i < nOfMsgs; i++){

		*msgIdx = new struct gst;
		cleanGst(*msgIdx);

		memcpy((*msgIdx) -> type, charIdx, typel);
		charIdx += typel;

		memcpy((*msgIdx) -> id, charIdx, idl);
		charIdx += idl;

		if ((*msgIdx) -> type[0] == '2'){
			memcpy((*msgIdx) -> posx, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posy, charIdx, posl);
			charIdx += posl;
		}

		if ((*msgIdx) -> type[0] == '2' || (*msgIdx) -> type[0] == '8'){
			memcpy((*msgIdx) -> info, charIdx, infol);
			charIdx += infol;
		}

		else if ((*msgIdx) -> type[0] == '5'){
			memcpy((*msgIdx) -> path, charIdx, pathl);
			charIdx += pathl;
		}

		else if ((*msgIdx) -> type[0] == '0'){
			memcpy((*msgIdx) -> ancho, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> alto, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posx, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posy, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> path, charIdx, pathl);
			charIdx += pathl;
		}

		else if ((*msgIdx) -> type[0] == '1'){
			memcpy((*msgIdx) -> alto, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posx, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> posy, charIdx, posl);
			charIdx += posl;
			memcpy((*msgIdx) -> info, charIdx, infol);
			charIdx += infol;
		}

		else if ((*msgIdx) -> type[0] != '3'){
			return -1;
		}

		msgIdx++;
	}

	return nOfMsgs;
}




int encodeMessages(char** msgsChar, struct gst** msgs, int qty){

	int pkglen = lengthl + nOfMsgsl;

	*msgsChar = new char[999];
	char* buffer = *msgsChar;
	memset(buffer, 0, 999);
	char* bufferIt = buffer + lengthl;
	
	//copio la cantidad de mensajes
	intToFixedChar(qty, bufferIt, nOfMsgsl);
	bufferIt += nOfMsgsl;
	
	for (int i = 0; i < qty; i++){
		*bufferIt++ = (*msgs) -> type[0];
		
		memcpy(bufferIt, (*msgs) -> id, idl);
		bufferIt += idl;
		
		pkglen += typel + idl;

		if ((*msgs) -> type[0] == '2'){
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

		else if ((*msgs) -> type[0] == '0'){
			memcpy(bufferIt, (*msgs) -> ancho, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> alto, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> posx, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> posy, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> path, pathl);
			bufferIt += pathl;

			pkglen += posl + posl + posl + posl + pathl;
		}

		else if ((*msgs) -> type[0] == '5'){
			memcpy(bufferIt, (*msgs) -> path, pathl);
			bufferIt += pathl;

			pkglen += pathl;
		}

		else if ((*msgs) -> type[0] == '1'){
			memcpy(bufferIt, (*msgs) -> alto, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> posx, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> posy, posl);
			bufferIt += posl;

			memcpy(bufferIt, (*msgs) -> info, infol);
			bufferIt += infol;

			pkglen +=  posl + posl + posl + infol;

		}

		delete (*msgs);
		msgs++;
		
	}


	intToFixedChar(pkglen, buffer, lengthl);
	return pkglen;
}

struct gst* genGstFromVelocidades(int velocidadDesplazamiento, int velocidadDisparos) {
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::OBJETO;

	char velocidad[posl];
	memcpy(newMsg -> id, "SP", idl);

	intToFixedChar(velocidadDesplazamiento, velocidad, posl);
	memcpy(newMsg -> ancho, velocidad, posl);

	intToFixedChar(velocidadDisparos, velocidad, posl);
	memcpy(newMsg -> alto, velocidad, posl);

	intToFixedChar(0, velocidad, posl);
	memcpy(newMsg -> posx, velocidad, posl);

	intToFixedChar(0, velocidad, posl);
	memcpy(newMsg -> posy, velocidad, posl);

	memset(newMsg -> path, '=', pathl);

	return newMsg;
}

struct gst* genGstFromCantJug(int cantJug) {
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::CANT_JUG;

	char cantidad[idl + 1];
	cantidad[idl] = 0;
	intToFixedChar(cantJug, cantidad, idl);
	memcpy(newMsg -> id, cantidad, idl);

	return newMsg;
}

struct gst* genGstFromFondo(Parser::type_Escenario * escenario, char path[pathl]) {
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::OBJETO;

	char charMedida[posl];
	memcpy(newMsg -> id, "FO", idl);

	intToFixedChar(escenario->ancho, charMedida, posl);
	memcpy(newMsg -> ancho, charMedida, posl);

	intToFixedChar(escenario->alto, charMedida, posl);
	memcpy(newMsg -> alto, charMedida, posl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> posx, charMedida, posl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> posy, charMedida, posl);

	memset(newMsg -> path, '=', pathl);
	memcpy(newMsg -> path, path, pathl);

	return newMsg;
}

struct gst* genGstFromElemento(Parser::type_Elemento * elem, char path[pathl]) {
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::OBJETO;

	char charMedida[posl];
	memcpy(newMsg -> id, "EL", idl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> ancho, charMedida, posl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> alto, charMedida, posl);

	intToFixedChar(elem->posicionX, charMedida, posl);
	memcpy(newMsg -> posx, charMedida, posl);

	intToFixedChar(elem->posicionY, charMedida, posl);
	memcpy(newMsg -> posy, charMedida, posl);

	memset(newMsg -> path, '=', pathl);
	memcpy(newMsg -> path, path, pathl);

	return newMsg;
}

struct gst* genGstFromVentana(Parser::type_Ventana* ventana){
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::OBJETO;

	char charMedida[posl];
	memcpy(newMsg -> id, "VE", idl);

	intToFixedChar(ventana->ancho, charMedida, posl);
	memcpy(newMsg -> ancho, charMedida, posl);

	intToFixedChar(ventana->alto, charMedida, posl);
	memcpy(newMsg -> alto, charMedida, posl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> posx, charMedida, posl);

	intToFixedChar(0, charMedida, posl);
	memcpy(newMsg -> posy, charMedida, posl);

	memset(newMsg -> path, '=', pathl);

	return newMsg;
}

struct gst* genGstFromSprite(Parser::type_Sprite * sprite) {
	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::SPRITE;

	intToFixedChar(sprite->id, newMsg -> id, idl);
	cout << "DEBUG genGstFromSprite " << newMsg -> id << endl;

	memset(newMsg -> path, '=', pathl);
	memcpy(newMsg -> path, sprite->path, pathl);

	return newMsg;
}

struct gst* genUpdateGstFromElemento(Elemento* elemento, int idc){

	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);
	newMsg -> type[0] = (char) msgType::UPDATE;

	char charId[idl], charPos[posl];
	intToFixedChar(elemento->getId(), charId, idl);
	memcpy(newMsg -> id, charId, idl);

	intToFixedChar(elemento -> getPosX(), charPos, posl);
	memcpy(newMsg -> posx, charPos, posl);

	intToFixedChar(elemento -> getPosY(), charPos, posl);
	memcpy(newMsg -> posy, charPos, posl);

	newMsg -> info[0] = (char) elemento -> getEstado(idc);

	return newMsg;
}

struct gst* genAdminGst(int clientId, command comando){

	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::CONTROL;

	char temp[idl + 1];
	temp[idl] = 0;

	intToFixedChar(clientId, temp, idl);
	memcpy(newMsg -> id, temp, idl);

	newMsg -> info[0] = (char) comando;
	return newMsg;
}

struct gst* genInitGst(int id, int progress, int posx, int posy, bool playing){

	struct gst* newMsg = new struct gst;
	cleanGst(newMsg);

	newMsg -> type[0] = (char) msgType::INIT;


	intToFixedChar(id, newMsg -> id, idl);
	intToFixedChar(progress , newMsg -> alto, posl);
	intToFixedChar(posx, newMsg -> posx, posl);
	intToFixedChar(posy, newMsg -> posy, posl);

	if (playing){
		newMsg -> info[0] = 'Y';
	}
	else{
		newMsg -> info[0] = 'N';
	}

	return newMsg;
}
