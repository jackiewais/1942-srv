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
	DESCONECTADO = 'd'
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
	int getId();
	int getPosX();
	int getPosY();
	status getEstado();
};

#endif /* ELEMENTO_ELEMENTO_H_ */
