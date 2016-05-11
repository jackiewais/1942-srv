/*
 * Elemento.h
 *
 *  Created on: 2 de may. de 2016
 *      Author: pablomd
 */

#ifndef ELEMENTO_H_
#define ELEMENTO_H_

#include "../Utils/messages.h"

enum status:char{
	VIVO = 'a',
	DESTRUIDO = 'b',
	DISPARANDO = 'c',
	DESCONECTADO = 'd',
	TRUCO = 'e',
	PAUSA = 'f',
	NO_PAUSA = 'h',
	START = 's',
	RESET = 'r'

	};

class Elemento {
private:
	int id;
	int posX;
	int posY;
	status estado;
public:
	Elemento(int idEl, int x, int y);
	Elemento(struct gst*);
//	virtual ~Elemento();
	void update(int x, int y, status estado);
	void update(struct gst*);
	void updateStatus(status estado);
	void updatePos(int x, int y);
	int getId();
	int getPosX();
	int getPosY();
	status getEstado();
};

#endif /* ELEMENTO_ELEMENTO_H_ */
