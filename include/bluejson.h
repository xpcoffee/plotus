#ifndef BLUEJSON_H
#define BLUEJSON_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cassert>

using namespace std;

class BlueJSON
{
private:
// member variables
    string m_text;
    string::iterator m_it;
    unsigned int m_value_start;
    unsigned int m_nest_level;
    bool flag_key_found;
    bool flag_in_string;
    string m_token;

// functions
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