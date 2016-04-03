#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <list>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#include "Log.h"
#include "../Utils/Util.h"

const char* logName = "log_file.txt";
const char* logPath =  "./Logger/Logs/";

Util util;

char const* getLogFile(){
	return (string(logPath) + string(logName)).c_str();
}
void Log::writeLine(string line)
{
	printf("%s\n",line.c_str());
    ofstream log_file(getLogFile(), ios_base::out | ios_base::app );
    log_file << util.currentDateTime() + " || " << line << endl;
}

void Log::writeErrorLine(string line)
{
	perror((line + "\n").c_str());
    ofstream log_file(getLogFile(), ios_base::out | ios_base::app );

    log_file << util.currentDateTime() + " || " << line << endl;
}


void Log::deleteLine()
{
	// TODO: De momento no tiene un uso determinado.-
}

void Log::writeBlock(list<string> lineList)
{
    ofstream log_file(getLogFile(), ios_base::out | ios_base::app );
    list<string>::iterator pos;
    pos = lineList.begin();
    while( pos != lineList.end())
    {
    	cout << *pos;
    	log_file << endl;
    	log_file << util.currentDateTime() + " || " << *pos << endl;
    	pos++;
    }
}

void Log::deleteBlock()
{
	// TODO: De momento no tiene un uso determinado.-
}

// Chequeamos que el archivo exista.
inline bool file_exists (const string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

void Log::createFile()
{
	// Hacemos un backup del anterior archivo de Log.
	string bName =  string(logPath) + "(" + util.currentDateTime() + ")" + logName;
	const char* backupName = bName.c_str();
	if(file_exists(getLogFile()))
		rename(getLogFile(), backupName);

	// Creamos e inicializamos nunestro nuevo archivos de log.
	ofstream outfile (getLogFile());
	outfile << "Archivo de Log inicializado: " << util.currentDateTime() << endl;
	outfile << endl;
	outfile.close();

	if(!file_exists(getLogFile())){
		perror(("Missing folder " + string(logPath)).c_str());
		exit(1);
	}

}

void Log::deleteFile()
{
	// TODO: De momento no tiene un uso determinado.-
}
