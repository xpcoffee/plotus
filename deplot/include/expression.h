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
//	"	Preprocessor Definitions	"
//	"""""""""""""""""""""""""""""""""

    #ifndef COMPRESSION_CHAR
    #define COMPRESSION_CHAR '$'
    #endif

    #ifndef PI
    #define PI 3.141592653589793238462643383279502884
    #endif

//	"""""""""""""""""""""""""""""""""
//	"		Enumerated Types		"
//	"""""""""""""""""""""""""""""""""
    enum READABLILITY_CODES
    {
        UNINITIALIZED_COUNTER = -1,
    };

    enum INPUT_ERROR_CODES
    {
        INPUT_ERROR_UNINITIALIZED_VARIABLE = 300,
        INPUT_ERROR_INVALID_EXPRESSION = 301,
        INPUT_ERROR_PARENTH_EMPTY = 302,
        INPUT_ERROR_PARENTH_NOT_EMPTY = 303,
    };

    enum MATH_ERROR_CODES
    {
        MATH_DIVIDE_BY_ZERO = 200,
        MATH_NAN = 201,
    };


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
    int nProblemTerm;
	vector<string> vExpression;
	vector<string> vOriginalExpression;
    vector<Variable> vVariables;
    vector<double> vResult;
    vector<int> vProblemElements_Result;
    vector<int> vProblemElements_Expression;
    bool flag_isValid;

	// functions
    // - parsing
	bool charIsDigit(char);
	bool charIsParenthesis(char);
	bool charIsOperator(char);
    bool charIsAlpha(char);
	bool charIsWhitespace(char);
    // - validation
    bool check_DecimalPointOK(string);
    bool checkIllegalChars(string);
    bool check_CharsOK(string);
    bool check_NumbersOK(string);
    bool check_OperatorsOK(string, int, int, bool&);
    bool variableNameIsUnique(Variable&);
    bool termIsNumeric(string);
    bool termIsAlpha(string);
    bool termIsFunction(string);
    bool termIsStandardValue(string);
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
    void handleMathException(MATH_ERROR_CODES);

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
    bool isValid();
    bool charIsValid(char);
    bool variableNameIsValid(Variable&);
    // - experimental

};



#endif
