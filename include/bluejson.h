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
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <limits>
#include <iostream>

///	Typedef
///	========

typedef std::numeric_limits<double> precDouble;


/// Enumerated Types
/// =================

enum JSONFormat{
    MultiLine = 0,
    Flat
};


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

    //	utility

    static string jsonObject(string json);
    static string jsonObject(vector<string> properties, JSONFormat format = MultiLine);

    template <typename T>
    static string jsonArray(vector<T> values, JSONFormat format = MultiLine);
    static string jsonArray(string json, JSONFormat format = MultiLine);

    template <typename T>
    static string jsonKeyValue(string key, T value);

    template <typename T>
    static string jsonValue(T value);
    static string jsonValue(string value);
    static string jsonValue(char value);


};

///	Templates
/// ==========

template <typename T>
string BlueJSON::jsonValue(T value)
{
    stringstream buffer;
    buffer.precision(precDouble::digits10);
    buffer << value;
    return buffer.str();
}

template <typename T>
string BlueJSON::jsonArray(vector<T> values, JSONFormat format)
{
    stringstream buffer;
    unsigned int i = 0;

    buffer << "[";
    if (format == MultiLine) buffer << "\n";

    for (; i < values.size() - 1; i++){
        buffer << jsonValue(values[i]);
        buffer << ",";

        if (format == MultiLine) buffer << "\n";
    }

    buffer << jsonValue(values[i]);
    if (format == MultiLine) buffer << "\n";
    buffer << "]";

    return buffer.str();
}

template <typename T>
string BlueJSON::jsonKeyValue(string key, T value)
{
    return "\"" + key + "\":" + jsonValue(value);
}

#endif // BLUEJSON_H
