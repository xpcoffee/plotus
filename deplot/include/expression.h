/*	Author(s):	Emerick Bosch
	Build:		0.1
	Date:		April 2014

	expression.
	-------------
	
	"""""""""""""""""""""""""""""""""	
	"			Description			"
	"""""""""""""""""""""""""""""""""	
	
	Class

	Interprets mathematical expressions from strings.
	Parses the strings iteratively into arrays.
	Arrayes solved iteratively.
*/

#ifndef EXPRESSION_H
#define EXPRESSION_H

//	"""""""""""""""""""""""""""""""""	
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""	

#include<string>
#include<vector>
#include<iostream>
#include<locale.h>
#include<stdlib.h>
#include<iomanip>
#include"variable.h"

//	"""""""""""""""""""""""""""""""""	
//	"			Namespaces			"
//	"""""""""""""""""""""""""""""""""	
using namespace std;

//	"""""""""""""""""""""""""""""""""	
//	"		Class Definition: 		"
//	"			Expression			"
//	"""""""""""""""""""""""""""""""""	

class Expression
{
private:
	// member variables
    int nTerms;
    int nVariables;
    int nCurrentVariable;
	vector<string> vExpression;
	vector<string> vOriginalExpression;
    vector<Variable> vVariables;
    vector<double> vResult;
    vector<int> vProblemElements_Result;
    vector<int> vProblemElements_Expression;
    bool flag_invalid;

	// functions
    // - parsing
	bool charIsDigit(char);
	bool charIsParenthesis(char);
	bool charIsOperator(char);
    bool charIsAlpha(char);
	bool charIsWhitespace(char);
    // - validation
    bool checkDecimalPoint(string);
    bool checkIllegalChars(string);
    bool checkIllegalVar(string);
    bool checkOperators(string, int, int, bool&);
    bool variableIsUnique(Variable&);
    // - recursive evaluation functions
    bool compressExpression(vector<string>&);
    bool doPowers(vector<string>&);
	bool doMultiplication(vector<string>&);
	bool doDivision(vector<string>&);
	bool doSubtraction(vector<string>&);
	bool doAddition(vector<string>&);
	bool doParenthesis(vector<string>&);
    // - evaluation functions
    void doSpecial(vector<string>&, int, bool);
	void doBasic(vector<string>&);
    void recEval();
    void resetExpression();
    double evaluateExpression();
    // - internal getters
    string getStringArray(vector<string>);
    // - exceptions and error handling
    void handleException(int);

public:
	// constructor
	Expression(string sExpressionString = "")
	{
		vOriginalExpression = parseExpressionArray(sExpressionString);
        resetExpression();
        vProblemElements_Expression = checkExpressionArray(vExpression);
		nCurrentVariable = 0;
        setlocale(LC_NUMERIC,"C"); // make '.' the decimal separator
    }

	// functions
    // - parsing
	vector<string> parseExpressionArray(string);
    vector<int> checkExpressionArray(vector<string>&);
    // - evaluation
    void subVariableValues();
    vector<double> evaluateAll();
    // - setters
	void setExpression(string);
	void addVariable(Variable);
	void clearVariables();
    // - getters
	string getExpression();
    string getTerm(int);
    int getNumTerms();
    vector<int> getProblemElements_Expression();
    vector<int> getProblemElements_Result();
    // - validation
    bool isInvalid();
    bool variableNameValid(Variable&);
    // - experimental
    void printExpression();

};



#endif
