/*
 * DefaultErrorHandler.cpp
 *
 *  Created on: Apr 10, 2016
 *      Author: bigfatpancha
 */

#include "DefaultErrorHandler.h"
#include <iostream>

DefaultErrorHandler::DefaultErrorHandler() {
}

DefaultErrorHandler::~DefaultErrorHandler(){

}

void DefaultErrorHandler::warning(const xercesc_3_1::SAXParseException& exc){
	std::cout << "WARNING en la validacion del XML";
}
void DefaultErrorHandler::error(const xercesc_3_1::SAXParseException& exc){
	std::cout << "ERROR de validacion. XML con formato incorrecto";
}
void DefaultErrorHandler::fatalError(const xercesc_3_1::SAXParseException& exc){
	std::cout << "FATAL ERROR de validacion. XML con formato incorrecto";
}
void DefaultErrorHandler::resetErrors(){

}



