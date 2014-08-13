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

    string m_name;
    vector<double> m_domain;
    Spacing m_domainSpacing;
    double m_min, m_max;
    int m_elements;
    int m_pos;
    bool flag_populated;
    bool flag_initialized;

public:
    Variable ():
        m_name(""),
        m_elements(1),
        flag_populated(false),
        flag_initialized(false)
    { }

    Variable(string name, double domain_start = 0, double domain_end = 0, int elements = 1):
        m_name(name),
        m_elements(elements)
    {
        setMinMax(domain_start, domain_end);
        setLinearDomain();
    }

    //	Setters
    //	--------

    void setName(string name) { m_name = name; }

    void setElements(int elements) { m_elements = elements; }

    void setMinMax(double num1, double num2)
    {
        if (num1 > num2)	{ m_min = num2; m_max = num1; }
        else 				{ m_min = num1; m_max = num2; }

        m_domain.clear();
        flag_populated = false;
        flag_initialized = true;
    }

    //	Getters
    //	--------

    string name() { return m_name; }

    int position(){ return m_pos; }

    int elements() { return m_elements; }

    double min() { return m_min; }

    double max() { return m_max; }

    double currentValue() { return m_domain[m_pos]; }

    Spacing domainSpacing() { return m_domainSpacing; }

    vector<double> domain() { return m_domain; }


    //	Domain Creation
    //	----------------

    void setLinearDomain()
    {
        if (!flag_initialized)
            return;

        setLinearDomain(m_min, m_max, m_elements);
    }

    void setLinearDomain(double min, double max, double steps)
    {
        double spacing;

        if (steps == 1) { spacing = 0; }
        else { spacing = (max - min)/static_cast<double>(steps - 1); }

        m_domain.clear();
        for (int i = 0; i < steps; i++){
            m_domain.push_back( min + i*spacing );
        }

        resetPosition();
        flag_populated   = true;
        flag_initialized = true;
        m_domainSpacing = Linear;
    }

    void setLogarithmicDomain(double base = 10)
    {
        if (!flag_initialized || base == 0)
            return;

        setLogarithmicDomain(m_min, m_max, m_elements, base);
    }

    void setLogarithmicDomain(double min, double max, double steps, double base = 10)
    {
        if (base == 0)
            return;

        double log_min = log(min)/log(base);
        double log_max = log(max)/log(base);

        m_domain.clear();
        for (int i = 0; i < steps; i++){
            double exponent = i*( log_max - log_min )/steps ;

            // skip negative roots
            if ((exponent < 0) && (exponent > -1)){ continue; }

            m_domain.push_back( pow( base, exponent  ) );
        }

        // account for skipped items
        m_elements = m_domain.size();

        resetPosition();
        flag_populated   = true;
        flag_initialized = true;
        m_domainSpacing = Logarithmic;
    }


    //	Iteration
    //	----------

    void nextPosition()
    {
        if (!flag_populated)
            return;

        if ( isEnd() ){ resetPosition(); }
        else { m_pos++; }
    }

    bool isEnd() { return m_pos == (m_elements - 1); }

    void resetPosition () { m_pos = 0; }

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
