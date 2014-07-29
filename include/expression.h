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

///	Includes
///	=========

#include<stdlib.h>
#include<string>
#include<vector>
#include<iomanip>
#include<iostream>
#include<sstream>
#include<cassert>
#include<locale.h>
#include<math.h>
#include"variable.h"

///	Preprocessor Definitions
///	=========================

    #ifndef COMPRESSION_CHAR
    #define COMPRESSION_CHAR '$'
    #endif

    #ifndef PI
    #define PI 3.141592653589793238462643383279502884
    #endif


///	Enumerated Types
///	=================

    enum READABLILITY_CODES
    {
        UninitializedCounter = -1,
    };

    enum INPUT_ERROR_CODES
    {
        InputErrorUninitializedVariable 	= 300,
        InputErrorInvalidExpression 		= 301,
        InputErrorParenthesesEmpty 		= 302,
        InputErrorParenthesesNotEmpty 		= 303,
    };

    enum MATH_ERROR_CODES
    {
        MathDivideByZero = 200,
        MathNaN = 201,
        MathPole = 202,
        MathComplex = 203,
    };


///	Namespaces
///	===========

using namespace std;


///	Class
/// ======

class Expression
{
public:
    Expression(string expression = "");

    //	setters
    void setExpression(string expression);
    void addVariable(Variable variable);
    void clearVariables();

    //	getters
    int getNumTerms();
    string getExpression();
    string getTerm(int term_pos);
    string getErrors();
    vector<int> getProblemElements_Expression();
    vector<int> getProblemElements_Result();
    bool isXBeforeY(Variable x_variable, Variable y_variable);

    //	parsing
    vector<string> parseExpressionArray(string expression);
    vector<int> checkExpressionArray(vector<string> &expression);

    //	evaluation
    void subVariableValues();
    vector<double> evaluateAll();

    //	validation
    bool isValid();
    bool charIsValid(char);
    bool variableNameIsValid(Variable&);

    //	static
    static bool approxEqual(double dNum1, double dNum2, double dPrec){
        return sqrt((dNum1-dNum2)*(dNum1-dNum2)) <= dPrec;
    }

private:
    //! member variables
    // - meta
    int m_TermCount;
    int m_VariableCount;
    int m_CurrentVariable;

    // - data
    vector<string> m_OriginalExpression;
    vector<string> m_WorkingExpression;
    vector<Variable> m_Variables;
    vector<double> m_Results;
    vector<int> m_Results_Problems;
    bool flag_XBeforeY;

    // - error handling
    bool flag_Valid;
    int m_ProblemTerm;
    string m_ErrorMessage;
    vector<int> m_Expression_Problems;

    // - evaluation events
    bool flag_Nan;
    bool flag_Pole;
    bool flag_DivByZero;
    bool flag_Complex;

    //! functions
    // - parsing
    bool charIsDigit		(char c);
    bool charIsParenthesis	(char c);
    bool charIsOperator		(char c);
    bool charIsAlpha		(char c);
    bool charIsWhitespace	(char c);

    // - validation
    bool check_DecimalPointOK	(string term);
    bool checkIllegalChars		(string term);
    bool check_NumbersOK		(string term);
    bool check_CharsOK			(string term);
    bool check_OperatorsOK		(string term, int term_pos, int last_term_pos, bool &flag_prev_operator);
    bool variableNameIsUnique(Variable &variable);
    bool termIsNumeric		(string term);
    bool termIsAlpha		(string term);
    bool termIsFunction		(string term);
    bool termIsStandardValue(string term);

    // - expression reduction
    bool compressExpression	(vector<string> &expression);
    bool doParenthesis		(vector<string> &expression);
    bool doPowers			(vector<string> &expression);
    bool doDivision			(vector<string> &expression);
    bool doMultiplication	(vector<string> &expression);
    bool doSubtraction		(vector<string> &expression);
    bool doAddition			(vector<string> &expression);
    void doBasic(vector<string>&);
    void doSpecial(vector<string>&, int, bool);

    // - recursive evaluation
    void recEval();
    double evaluateExpression();
    void resetExpression();

    // - internal getters
    string getStringArray(vector<string> string_array);

    // - exceptions and error handling
    void handleMathException(MATH_ERROR_CODES);
    void resetEvaluation();
};



#endif
