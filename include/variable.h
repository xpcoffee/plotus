/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    variable.h
    -------------

    Description
    ============
    Stores the name and domain of a variable/parameter.
    Stores standard variable names and values (constants, etc.).

    Provides methods to:
    - create the domain of the variable
    - check the validity of the variable name
*/

#ifndef VARIABLE_H
#define VARIABLE_H

///	Includes
///	=========

#include<string>
#include<vector>
#include<iostream>
#include<cassert>
#include<math.h>


///	Namespace
///	==========

using namespace std;


///	Enumerators
///	============

enum Spacing {
    Linear	= 0,
    Logarithmic,
};

///	Class
///	======

class Variable
{
private:
    //	Member Variables
    //	-----------------

    string m_Name;
    vector<double> m_Domain;
    Spacing m_DomainSpacing;
    double m_Min, m_Max;
    int m_Elements;
    int m_Pos;
    bool flag_Populated;
    bool flag_Initialized;

public:
    Variable ():
        m_Name(""),
        m_Elements(1),
        flag_Populated(false),
        flag_Initialized(false)
    { }

    Variable(string name, double domain_start = 0, double domain_end = 0, int elements = 1):
        m_Name(name),
        m_Elements(elements)
    {
        setMinMax(domain_start, domain_end);
        setLinearDomain();
    }

    //	Setters
    //	--------

    void setName(string name) { m_Name = name; }

    void setElements(int elements) { m_Elements = elements; }

    void setMinMax(double num1, double num2)
    {
        if (num1 > num2)	{ m_Min = num2; m_Max = num1; }
        else 				{ m_Min = num1; m_Max = num2; }

        m_Domain.clear();
        flag_Populated = false;
        flag_Initialized = true;
    }

    //	Getters
    //	--------

    string name() { return m_Name; }

    int position(){ return m_Pos; }

    int elements() { return m_Elements; }

    double min() { return m_Min; }

    double max() { return m_Max; }

    double getCurrentValue() { return m_Domain[m_Pos]; }

    Spacing getDomainSpacing() { return m_DomainSpacing; }


    //	Domain Creation
    //	----------------

    void setLinearDomain()
    {
        if (!flag_Initialized)
            return;

        setLinearDomain(m_Min, m_Max, m_Elements);
    }

    void setLinearDomain(double min, double max, double steps)
    {
        double spacing;

        if (steps == 1) { spacing = 0; }
        else { spacing = (max - min)/static_cast<double>(steps - 1); }

        m_Domain.clear();
        for (int i = 0; i < steps; i++){
            m_Domain.push_back( min + i*spacing );
        }

        resetPosition();
        flag_Populated   = true;
        flag_Initialized = true;
        m_DomainSpacing = Linear;
    }

    void setLogarithmicDomain(double base = 10)
    {
        if (!flag_Initialized || base == 0)
            return;

        setLogarithmicDomain(m_Min, m_Max, m_Elements, base);
    }

    void setLogarithmicDomain(double min, double max, double steps, double base = 10)
    {
        if (base == 0)
            return;

        double log_min = log(min)/log(base);
        double log_max = log(max)/log(base);

        m_Domain.clear();
        for (int i = 0; i < steps; i++){
            double exponent = i*( log_max - log_min )/steps ;

            // skip negative roots
            if ((exponent < 0) && (exponent > -1)){ continue; }

            m_Domain.push_back( pow( base, exponent  ) );
        }

        // account for skipped items
        m_Elements = m_Domain.size();

        resetPosition();
        flag_Populated   = true;
        flag_Initialized = true;
        m_DomainSpacing = Logarithmic;
    }


    //	Iteration
    //	----------

    void nextPosition()
    {
        if (!flag_Populated)
            return;

        if ( isEnd() ){ resetPosition(); }
        else { m_Pos++; }
    }

    bool isEnd() { return m_Pos == (m_Elements - 1); }

    void resetPosition () { m_Pos = 0; }

    //	Validation
    //	-----------

    static bool nameIsLegal(string name)
    {
        // name is set
        if (name.empty())
            return false;

        // start with number
        char varNameStart = name[0];
        if (('0' <= varNameStart) && (varNameStart  <= '9'))
            return false;

        // illegal name
        if (isFunction(name) || isStandardValue(name))
            return false;

        // illegal characters
        for (string::iterator it = name.begin(); it != name.end(); it++){
            char c = *it;
            if (	// operators
                    (c == '+') || (c == '-') ||(c == '/') ||(c == '*') ||(c == '^') ||
                    // parentheses
                    (c == '(') || (c == ')') ||
                    // whitespace
                    (c == ' ')  || (c == '\t') || (c == '\n')  || (c == '\r') ||
                    (c == '\v') || (c == '\b') || (c == '\f')
                    ){
                return false;
            }
        }
        return true; // all checks passed
    }

    static bool isFunction(string sName)
    {
         return (sName == "sin") 		|| (sName == "cos") 	|| (sName == "tan") 	||
                (sName == "sec") 		|| (sName == "csc") 	|| (sName == "cot") 	||
                (sName == "sinh")		|| (sName == "cosh")	|| (sName == "tanh")	||
                (sName == "sech") 		|| (sName == "csch") 	|| (sName == "coth") 	||
                (sName == "arcsin") 	|| (sName == "arccos") 	|| (sName == "arctan") 	||
                (sName == "arcsec") 	|| (sName == "arccsc") 	|| (sName == "arccot") 	||
                (sName == "arcsinh")	|| (sName == "arccosh")	|| (sName == "arctanh") ||
                (sName == "arcsech") 	|| (sName == "arccsch") || (sName == "arccoth") ||
                (sName == "exp") 		|| (sName == "ln") 		|| (sName == "log");
    }

    static bool isStandardValue(string sName)
    {
        return 	(sName == "pi");
    }
};

#endif
