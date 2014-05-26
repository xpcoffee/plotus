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
	// member variables
    int nTerms;
    int nVariables;
    int nCurrentVariable;
	vector<string> vExpression;
	vector<string> vOriginalExpression;
    vector<Variable> vVariables;
    vector<double> vResult;
    vector<int> vProblemSpace;
    vector<int> vProblemTerms;
    bool flag_EmptyParenth;

	// functions
	bool charIsDigit(char);
	bool charIsParenthesis(char);
	bool charIsOperator(char);
    bool charIsAlpha(char);
	bool charIsWhitespace(char);
    bool checkDecimalPoint(string);
    bool checkIllegalChars(string);
    bool checkIllegalVar(string);
    bool checkOperators(string);
    bool compressExpression(vector<string>&);
    bool doPowers(vector<string>&);
	bool doMultiplication(vector<string>&);
	bool doDivision(vector<string>&);
	bool doSubtraction(vector<string>&);
	bool doAddition(vector<string>&);
	bool doParenthesis(vector<string>&);
    void doSpecial(vector<string>&, int);
	void doBasic(vector<string>&);
	void resetExpression();
	void recEval();
	string getStringArray(vector<string>);
	
public:
	// constructor
	Expression(string sExpressionString = "")
	{
		vOriginalExpression = parseExpressionArray(sExpressionString);
        vProblemTerms = checkExpressionArray(vOriginalExpression);
		vExpression         = vOriginalExpression;
		nCurrentVariable = 0;
        std::cout << std::setprecision(6) << "";
        setlocale(LC_NUMERIC,"C");
        flag_EmptyParenth = false;
    }

	// functions
	vector<string> parseExpressionArray(string);
    vector<int> checkExpressionArray(vector<string>&);
    double evaluateExpression();
	void printExpression();
	void setExpression(string);
	void addVariable(Variable);
	void clearVariables();
	void subVariableValues();
    vector<double> evaluateAll();
	string getExpression();
    string getTerm(int);
    vector<int> getProblemTerms();
    vector<int> getProblemSpace();


	// experimental

};



#endif
