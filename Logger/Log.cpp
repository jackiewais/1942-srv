#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <time.h>
#include <list>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

#include "Log.h"
#include "../Utils/Util.h"

string logName = "Log.txt";
string logPath =  "./Logs/";

Util util;
unsigned short level;

char const* getLogFile(){
	char const* fullPath = (logPath + logName).c_str();
	return fullPath;
}

void Log::writeLine(string line)
{
	printf("%s\n",line.c_str());
	if(level >= 3){
		ofstream log_file(getLogFile(), ios_base::out | ios_base::app );
		log_file << util.currentDateTime() + " || " << line << endl;
	    log_file.close();
	}
}

void Log::writeWarningLine(string line)
{
	if(level >= 2){
		printf("%s\n",line.c_str());
		ofstream log_file(getLogFile(), ios_base::out | ios_base::app );
		log_file << util.currentDateTime() + " || WARNING || " << line << endl;
	    log_file.close();
	}
}

void Log::writeErrorLine(string line)
{
	if(level >= 1){
		perror((line + "\n").c_str());
		ofstream log_file(getLogFile(), ios_base::out | ios_base::app );
		log_file << util.currentDateTime() + " || ERROR || " << line << endl;
	    log_file.close();
	}
}


void Log::deleteLine()
{
	// TODO: De momento no tiene un uso determinado.-
}

void Log::writeBlock(list<string> lineList)
{
    std::ofstream log_file(getLogFile(), std::ios_base::out | std::ios_base::app );
    list<string>::iterator pos;
    pos = lineList.begin();
    while( pos != lineList.end())
    {
    	cout << *pos;
    	log_file << std::endl;
    	log_file << util.currentDateTime() + " || " << *pos << std::endl;
    	pos++;
    }
    log_file.close();
}

void Log::deleteBlock()
{
	// TODO: De momento no tiene un uso determinado.-
}

// Chequeamos que el archivo exista.
inline bool file_exists (const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

void Log::createFile(unsigned short  newLevel)
{
	// Actualizamos el nivel de logueo.
	level = newLevel;

	// Definimos un nombre para el Log de manera que sea UNIVOCO.-
	string fileDate = "(" + util.currentDateTime() + ") " + logName;
	logName = fileDate.c_str();

	// Creamos e inicializamos nunestro nuevo archivos de log.
	std::ofstream outfile (getLogFile());
	outfile << "Archivo de Log inicializado: " << util.currentDateTime() << std::endl;
	outfile << std::endl;
	outfile.close();

	//Chequeamos que haya sido creado exitosamente.
	if(!file_exists(getLogFile())){
		perror(("Missing folder " + logPath).c_str());
		exit(1);
	}
}

void Log::deleteFile()
{
	// TODO: De momento no tiene un uso determinado.-
}
