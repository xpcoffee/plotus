/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    inequality.h
    -------------

    Description
    ============
    Stores expressions and symbol required to represent an inequality.

    Provides methods to:
    - compare results of expression evaluation and return boolean array
      of the comparison results.
    - handle validation outputs from the Expression class
*/

#ifndef INEQUALITY_H
#define INEQUALITY_H


///	Includes
///	=========

#include<string>
#include<vector>
#include<cassert>
#include"variable.h"
#include"expression.h"


/// Enumerated Types
/// =================

enum InequalitySymbol{
    NoSymbol			= -1,
    SmallerThan	,
    GreaterThan,
    SmallerThanEqual,
    GreaterThanEqual,
    ApproxEqual,
};


///	Namespaces
///	===========

using namespace std;


///	Class
///	======

class Inequality
{
private:
    Expression m_LeftExpression, m_RightExpression;
    InequalitySymbol m_Sym;
    int m_Symbol;
    bool flag_Initialized;
    double m_Precision;
    string m_ErrorMessage;
    vector<double> m_LeftResults, m_RightResults;

public:
    //! variables
    bool *cancelFlagLeft;
    bool *cancelFlagRight;

    //! functions
    Inequality(string expression1 = "", InequalitySymbol symbol = NoSymbol, string expression2 = "");

    //	setters
    void clearVariables();
    void addVariable(Variable variable);
    void setInequality(string left_expression, int symbol, string right_expression);
    void setInequality(string left_expression, InequalitySymbol symbol, string right_expression);
    void changeSymbol(int symbol);
    void changeSymbol(InequalitySymbol symbol);
    void setPrecision(double value);

    //	getters
    vector<int> getProblemElements_ExpressionLHS();
    vector<int> getProblemElements_ExpressionRHS();
    string getTermLHS(int nTerm);
    string getTermRHS(int nTerm);
    string getExpressionLHS();
    string getExpressionRHS();
    int getNumTermsLHS();
    int getNumTermsRHS();
    bool getXBeforeY(Variable mXVar, Variable mYVar);


    //	evaluation
    vector<bool> evaluate();
    vector<int> getProblemElements_ResultsCombined();

    //	validation
    bool isValidLHS();
    bool isValidRHS();
    bool variablesInit();
    bool variableIsValid (Variable & myVar);
    string getErrors();

}; // Inequality

#endif
