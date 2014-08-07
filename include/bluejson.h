/*!	Author(s):	Emerick Bosch
    Build:		0.1
    Date:		July 2014

    bluejson.h
    -------------

    Description
    ============
    JSON parser with the ability of finding the
    next value of one (or many) key(s).
*/

#ifndef BLUEJSON_H
#define BLUEJSON_H


///	Includes
///	=========

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cassert>


///	Namespace
///	==========

using namespace std;


///	Class
/// ======

class BlueJSON
{
private:
    // member variables
    string m_text;
    string::iterator m_it;
    unsigned int m_valueStart;
    unsigned int m_nestLevel;
    bool flag_keyFound;
    bool flag_inString;
    string m_token;

    // parsing
    bool isValueStart(char c);
    bool isValueEnd(char c);
    bool isStringDelimiter(char c);

public:
    //	class
    BlueJSON(string text = "");

    //	setters
    void setText(string text);
    void readInFile(string filename);
    void setPosition(int pos);

    //	getters
    int getCurrentPos();

    //	features
    bool getNextKeyValue(string key, string &value, int pos = -1);
    bool getNextKeyValue(vector<string> keys, string &value, int &closest_key);
    bool getStringToken(string &token);
    bool getIntToken(int &token);
    bool getDoubleToken(double &token);
    bool getBoolToken(bool &token);


};

#endif // BLUEJSON_H
