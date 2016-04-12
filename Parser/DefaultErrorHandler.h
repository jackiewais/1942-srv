/*
 * DefaultErrorHandler.h
 *
 *  Created on: Apr 10, 2016
 *      Author: bigfatpancha
 */

#ifndef DEFAULTERRORHANDLER_H_
#define DEFAULTERRORHANDLER_H_

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

class DefaultErrorHandler: public xercesc_3_1::ErrorHandler {
public:
	DefaultErrorHandler();
	~DefaultErrorHandler();

	void warning(const xercesc_3_1::SAXParseException& exc);
	void error(const xercesc_3_1::SAXParseException& exc);
	void fatalError(const xercesc_3_1::SAXParseException& exc);
	void resetErrors();
};

#endif /* DEFAULTERRORHANDLER_H_ */
