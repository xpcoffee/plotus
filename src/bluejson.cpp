#include "include/bluejson.h"


/// Public
/// -------

BlueJSON::BlueJSON(string text):
    m_Text(text),
    m_ValueStart(0),
    flag_InString(false),
    m_Token("")
{
    m_it = m_Text.begin();
}


//	setters
//	--------

void BlueJSON::setText(string text)
{
    m_Text = text;
    m_it = m_Text.begin();
}

void BlueJSON::readInFile(string filename)
{
    if (filename.empty())
        return;
// read in file
    string token;
    string file = "";
    ifstream iss(filename.c_str());
    if (!iss.is_open())
        return;
    while (getline(iss, token)){
        file += token + "\n";
    }
    iss.close();
    setText(file);
}

void BlueJSON::setPosition(int pos){
    if ( (m_Text.empty()) || (pos > static_cast<int>(m_Text.size()-1)) )
        return;
    m_it = m_Text.begin() + pos;
}


//	getters
//	--------

int BlueJSON::getCurrentPos(){ return m_it - m_Text.begin(); }


// 	features
//	---------

bool BlueJSON::getNextKeyValue(string key, string &value, int pos)
{
    if (m_Text.empty() || key.empty())
        return false;

    if (pos != -1){
        setPosition(pos);
    }

    flag_KeyFound = false;
    flag_InString = false;
    stringstream buffer;

    for (; m_it != m_Text.end(); m_it++){
        char c = *m_it;

        // find key
        if (!flag_KeyFound){
            if (isStringDelimiter(c)){
                if (buffer.str() == key){	// key found
                    flag_KeyFound = true;
                    flag_InString = false;
                    m_NestLevel = 0;
                }
                buffer.str("");
            } else {
                buffer << c;
            }
        }

        // find value
        else {
            if (isValueStart(c)){
               // start of value
               if (m_NestLevel == 0){
                    m_ValueStart = m_it - m_Text.begin() + 1;
               }
               if (c != ':') { m_NestLevel++; }
            }
            if (isValueEnd(c)){
               if (c != ',' && m_NestLevel != 0) { m_NestLevel--; }
               if (m_NestLevel == 0){	// end of value
                    unsigned int value_end = m_it - m_Text.begin();
                    value = m_Text.substr(m_ValueStart, value_end - m_ValueStart);	// return value
                    m_Token = value;
                    return true;
               }
            }
        }
    }
    // key-value not found
    return false;
}

bool BlueJSON::getNextKeyValue(vector<string> keys, string &value, int &closest_key)
{
    if (m_Text.empty() || keys.empty())
        return false;

//    if (pos != -1){
//        setPosition(pos);
//    }

    flag_KeyFound = false;
    flag_InString = false;
    stringstream buffer;

    for (; m_it != m_Text.end(); m_it++){
        char c = *m_it;

        // find key
        if (!flag_KeyFound){
            if (isStringDelimiter(c)){
                for (unsigned int i = 0; i < keys.size(); i++){
                    if (buffer.str() == keys[i]){	// key found
                        flag_KeyFound = true;
                        flag_InString = false;
                        m_NestLevel = 0;
                        if (closest_key != -1) // if integer was passed
                            closest_key = i;
                    }
                }
                buffer.str("");
            } else {
                buffer << c;
            }
        }

        // find value
        else {
            if (isValueStart(c)){
               // start of value
               if (m_NestLevel == 0){
                    m_ValueStart = m_it - m_Text.begin() + 1;
               }
               if (c != ':') { m_NestLevel++; }
            }
            if (isValueEnd(c)){
               if (c != ',' && m_NestLevel != 0) { m_NestLevel--; }
               if (m_NestLevel == 0){	// end of value
                    unsigned int value_end = m_it - m_Text.begin();
                    value = m_Text.substr(m_ValueStart, value_end - m_ValueStart);	// return value
                    m_Token = value;
                    return true;
               }
            }
        }
    }
    // key-value not found
    return false;
}

bool BlueJSON::getStringToken(string &token)
{
    if (m_Token.empty() || m_Token.size() < 2)
        return false;
    if ((*m_Token.begin() != '"') || (*(m_Token.end()-1) != '"'))
        return false;
    token = m_Token.substr(1,m_Token.size()-2);
    return true;
}

bool BlueJSON::getIntToken(int &token)
{
    stringstream ss;
    if (m_Token.empty())
        return false;
    ss << m_Token;
    if(ss >> token)
        return true;
    return false;
}

bool BlueJSON::getDoubleToken(double &token)
{
    stringstream ss;
    if (m_Token.empty())
        return false;
    ss << m_Token;
    if(ss >> token)
        return true;
    return false;
}

bool BlueJSON::getBoolToken(bool &token)
{
    if (m_Token.empty())
        return false;
    if (m_Token == "false"){
        token = true;
        return true;
    }
    if (m_Token == "true"){
        token = false;
        return true;
    }
    return false;
}


///	Private
///	--------

//	parsing
//	--------

bool BlueJSON::isValueStart(char c)
{
    if (flag_InString)
        return false;
    return (c == '{') || (c == '[') || (c == ':');
}

 bool BlueJSON::isValueEnd(char c)
{
     if (flag_InString)
         return false;
     return (c == '}') || (c == ']') || (c == ',');
}

bool BlueJSON::isStringDelimiter(char c)
{
    if (c == '"'){
        flag_InString = !flag_InString;
        return true;
    }
    return false;
}
