#include "bluejson.h"


/// Public
/// -------

// 	class
//	------

BlueJSON::BlueJSON(string text):
    m_text(text),
    m_value_start(0),
    flag_in_string(false),
    m_token("")
{
    m_it = m_text.begin();
}


//	setters
//	--------

void BlueJSON::setText(string text)
{
    m_text = text;
    m_it = m_text.begin();
}

void BlueJSON::setPosition(int pos){
    if ( (m_text.empty()) || (pos > m_text.size()-1) )
        return;
    cout << "init" << endl;
    m_it = m_text.begin() + pos;
}


//	getters
//	--------

int BlueJSON::getCurrentPos(){ return m_it - m_text.begin(); }


// 	features
//	---------

bool BlueJSON::getNextKeyValue(string key, string &value, int pos)
{
    if (m_text.empty() || key.empty())
        return false;

    if (pos != -1){
        setPosition(pos);
    }

    flag_key_found = false;
    flag_in_string = false;
    stringstream buffer;

    for (; m_it != m_text.end(); m_it++){
        char c = *m_it;

        // find key
        if (!flag_key_found){
            if (isStringDelimiter(c)){
                if (buffer.str() == key){	// key found
                    flag_key_found = true;
                    flag_in_string = false;
                    m_nest_level = 0;
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
               if (m_nest_level == 0){
                    m_value_start = m_it - m_text.begin() + 1;
               }
               if (c != ':') { m_nest_level++; }
            }
            if (isValueEnd(c)){
               if (c != ',' && m_nest_level != 0) { m_nest_level--; }
               if (m_nest_level == 0){	// end of value
                    unsigned int value_end = m_it - m_text.begin();
                    value = m_text.substr(m_value_start, value_end - m_value_start);	// return value
                    m_token = value;
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
    if (m_text.empty() || keys.empty())
        return false;

//    if (pos != -1){
//        setPosition(pos);
//    }

    flag_key_found = false;
    flag_in_string = false;
    stringstream buffer;

    for (; m_it != m_text.end(); m_it++){
        char c = *m_it;

        // find key
        if (!flag_key_found){
            if (isStringDelimiter(c)){
                for (int i = 0; i < keys.size(); i++){
                    if (buffer.str() == keys[i]){	// key found
                        flag_key_found = true;
                        flag_in_string = false;
                        m_nest_level = 0;
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
               if (m_nest_level == 0){
                    m_value_start = m_it - m_text.begin() + 1;
               }
               if (c != ':') { m_nest_level++; }
            }
            if (isValueEnd(c)){
               if (c != ',' && m_nest_level != 0) { m_nest_level--; }
               if (m_nest_level == 0){	// end of value
                    unsigned int value_end = m_it - m_text.begin();
                    value = m_text.substr(m_value_start, value_end - m_value_start);	// return value
                    m_token = value;
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
    if (m_token.empty() || m_token.size() < 2)
        return false;
    if ((*m_token.begin() != '"') || (*(m_token.end()-1) != '"'))
        return false;
    token = m_token.substr(1,m_token.size()-2);
    return true;
}

bool BlueJSON::getIntToken(int &token)
{
    stringstream ss;
    if (m_token.empty())
        return false;
    ss << m_token;
    if(ss >> token)
        return true;
    return false;
}

bool BlueJSON::getDoubleToken(double &token)
{
    stringstream ss;
    if (m_token.empty())
        return false;
    ss << m_token;
    if(ss >> token)
        return true;
    return false;
//    if (m_token.empty())
//        return false;
//    if( (token = atof(m_token.c_str())) )
//        return true;
//    return false;
}

bool BlueJSON::getBoolToken(bool &token)
{
    cout << m_token << endl;
    if (m_token.empty())
        return false;
    if (m_token == "false"){
        token = true;
        return true;
    }
    if (m_token == "true"){
        token = false;
        return true;
    }
    return false;
}

///	Private
///	--------

bool BlueJSON::isValueStart(char c)
{
    if (flag_in_string)
        return false;
    return (c == '{') || (c == '[') || (c == ':');
}

 bool BlueJSON::isValueEnd(char c)
{
     if (flag_in_string)
         return false;
     return (c == '}') || (c == ']') || (c == ',');
}

bool BlueJSON::isStringDelimiter(char c)
{
    if (c == '"'){
        flag_in_string = !flag_in_string;
        return true;
    }
    return false;
}
