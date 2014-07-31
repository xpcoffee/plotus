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
#include<iostream>
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

    //	Member Variables
    //	-----------------

    Expression m_LeftExpression, m_RightExpression;
    int m_Symbol;
    InequalitySymbol m_Sym;
    double m_Precision;
    bool flag_Initialized;
    string m_ErrorMessage;
    vector<double> m_LeftResults, m_RightResults;

public:
    Inequality(string expression1 = "", InequalitySymbol symbol = NoSymbol, string expression2 = "") :
     m_Precision(0),
     flag_Initialized(true),
     m_ErrorMessage("")
    {
        if (symbol == NoSymbol){
            flag_Initialized = false;
        }
        else{
            m_LeftExpression.setExpression(expression1);
            m_RightExpression.setExpression(expression2);
            m_Sym = symbol;
        }
    }

    //	Setters
    //	========

    void clearVariables()
    {
        assert(flag_Initialized);
        m_LeftExpression.clearVariables();
        m_RightExpression.clearVariables();
    }

    void addVariable(Variable variable)
    {
        assert(flag_Initialized);
        m_LeftExpression.addVariable(variable);
        m_RightExpression.addVariable(variable);
    }

    void setInequality(string left_expression, int symbol, string right_expression)
    {
            m_LeftExpression.setExpression(left_expression);
            m_RightExpression.setExpression(right_expression);
            m_Symbol = symbol;
            flag_Initialized = true;
    }

    void setInequality(string left_expression, InequalitySymbol symbol, string right_expression)
    {
            m_LeftExpression.setExpression(left_expression);
            m_RightExpression.setExpression(right_expression);
            m_Sym = symbol;
            flag_Initialized = true;
    }

    void changeSymbol(int symbol)
    {
        assert(flag_Initialized);
        m_Symbol = symbol;
    }

    void changeSymbol(InequalitySymbol symbol)
    {
        assert(flag_Initialized);
        m_Sym = symbol;
    }

    void setPrecision(double value){ m_Precision = value; }

    //	Getters
    //	--------

    vector<int> getProblemElements_ExpressionLHS(){ return m_LeftExpression.getProblemElements_Expression(); }

    vector<int> getProblemElements_ExpressionRHS(){ return m_RightExpression.getProblemElements_Expression(); }

    string getTermLHS(int nTerm){ return m_LeftExpression.getTerm(nTerm); }

    string getTermRHS(int nTerm){ return m_RightExpression.getTerm(nTerm); }

    string getExpressionLHS(){ return m_LeftExpression.getExpression(); }

    string getExpressionRHS(){ return m_RightExpression.getExpression(); }

    int getNumTermsLHS(){ return m_LeftExpression.getNumTerms(); }

    int getNumTermsRHS(){ return m_RightExpression.getNumTerms(); }

    bool getXBeforeY(Variable mXVar, Variable mYVar) { return m_LeftExpression.isXBeforeY(mXVar, mYVar); }


    //	Evaluation
    //	-----------

    vector<bool> evaluate(){
        vector<bool> plot_points;

        m_LeftResults = m_LeftExpression.evaluateAll();
        m_RightResults = m_RightExpression.evaluateAll();

        for (unsigned int i = 0; i < m_LeftResults.size(); i++){
            switch (m_Sym){
            case SmallerThan:
                plot_points.push_back(m_LeftResults[i] < m_RightResults[i]);
                break;
            case GreaterThan:
                plot_points.push_back(m_LeftResults[i] > m_RightResults[i]);
                break;
            case SmallerThanEqual:
                plot_points.push_back(m_LeftResults[i] <= m_RightResults[i]);
                break;
            case GreaterThanEqual:
                plot_points.push_back(m_LeftResults[i] >= m_RightResults[i]);
                break;
            case ApproxEqual:
                {
                    double diff = (m_LeftResults[i]-m_RightResults[i])*(m_LeftResults[i]-m_RightResults[i]);
                    plot_points.push_back((m_Precision*m_Precision) >= diff);
                    break;
                }
            default:
                cerr << "Unknown symbol" << endl;
                assert (false && "Unknown inequality operator");
                break;
            }
        }
        return plot_points;
    }

    vector<int> getProblemElements_ResultsCombined()
    {
        vector <int> combined_plot_problems;
        vector <int> left_problem_points = m_LeftExpression.getProblemElements_Result();
        vector <int> right_problem_points = m_RightExpression.getProblemElements_Result();

        //combine both problem spaces
        vector<int>::iterator it = left_problem_points.begin();
        vector<int>::iterator jit = right_problem_points.begin();

        if( left_problem_points.empty() )
            { combined_plot_problems = right_problem_points; }
        else if ( right_problem_points.empty() )
            { combined_plot_problems = left_problem_points; }
        else {
            while ( it != left_problem_points.end() &&
                    jit != right_problem_points.end() ){
                if ( it == left_problem_points.end() ) {
                        combined_plot_problems.push_back(*jit);
                        jit++;
                }
                else if ( jit == right_problem_points.end() ) {
                        combined_plot_problems.push_back(*it);
                        it++;
                }
                else if (*it > *jit)	{
                        combined_plot_problems.push_back(*jit);
                        jit++;
                }
                else if (*it == *jit)	{
                        combined_plot_problems.push_back(*jit);
                        combined_plot_problems.push_back(*it);
                        it++; jit++;
                }
                else if (*it < *jit)	{
                        combined_plot_problems.push_back(*it);
                        it++;
                }
            }
        }

        return combined_plot_problems;
    }


    //	Validation
    //	-----------

    bool isValidLHS(){ return m_LeftExpression.isValid(); }

    bool isValidRHS(){ return m_RightExpression.isValid(); }

    bool variableIsValid (Variable & myVar){ return m_LeftExpression.variableNameIsValid(myVar); }

    string getErrors()
    {
        m_ErrorMessage += m_LeftExpression.getErrors();
        m_ErrorMessage += m_RightExpression.getErrors();
        return m_ErrorMessage;
    }

}; // Inequality

#endif
