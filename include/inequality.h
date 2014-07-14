/*	Author(s):	Emerick Bosch
	Build:		0.1
	Date:		April 2014

	inequality.h
	-------------
	
	"""""""""""""""""""""""""""""""""	
	"			Description			"
	"""""""""""""""""""""""""""""""""	
	
	Class

	Stores two expressions, their variables and the inequality that relates them.
	Runs evaluations of the expressions, evaluates true/false between the expressions. 
*/

#ifndef INEQUALITY_H 
#define INEQUALITY_H


//	"""""""""""""""""""""""""""""""""	
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""	

#include<string>
#include<vector>
#include<iostream>
#include<cassert>
#include"variable.h"
#include"expression.h"


/// Enumerated Types
/// =================

enum INEQUALITY_SYMBOL{
    SYMBOL_SMALLER_THAN 		= 0,
    SYMBOL_GREATER_THAN 		= 1,
    SYMBOL_SMALLER_THAN_EQUAL 	= 2,
    SYMBOL_GREATER_THAN_EQUAL	= 3,
    SYMBOL_APPROX_EQUAL			= 4,
};


//	"""""""""""""""""""""""""""""""""
//	"			Namespaces			"
//	"""""""""""""""""""""""""""""""""	

using namespace std;


//	"""""""""""""""""""""""""""""""""	
//	"		Class Definition: 		"
//	"			Variable			"
//	"""""""""""""""""""""""""""""""""	

class Inequality
{
private:

    // variables
    double dPrecision;
    int nSymbol;
    string sErrorMessage;
    vector<double> vEvalArray1, vEvalArray2;
    bool flag_initialized;
	
public:

    // variables
    Expression mExpression1, mExpression2;

    // constructor
    Inequality(string sExp1 = "", int symbol = -1, string sExp2 = "") :
     dPrecision(0),
     sErrorMessage(""),
     flag_initialized(true)
    {
        if (symbol == -1){
            flag_initialized = false;
        }
        else{
            mExpression1.setExpression(sExp1);
            mExpression2.setExpression(sExp2);
            nSymbol = symbol;
        }
	}

    // functions
    // - setters
	void clearVariables(){
        assert(flag_initialized);
        mExpression1.clearVariables();
        mExpression2.clearVariables();
	}

	void addVariable(Variable myVar){
        assert(flag_initialized);
        mExpression1.addVariable(myVar);
		mExpression2.addVariable(myVar);
	}

    void setInequality(string sExp1, int symbol, string sExp2){
            mExpression1.setExpression(sExp1);
            mExpression2.setExpression(sExp2);
            nSymbol = symbol;
            flag_initialized = true;
    }

    void changeSymbol(int symbol){
        assert(flag_initialized);
        nSymbol = symbol;
	}

    void setPrecision(double dPrec){ dPrecision = dPrec; }

    // - member variable getters
    vector<int> getProblemElements_ExpressionLHS(){ return mExpression1.getProblemElements_Expression(); }

    vector<int> getProblemElements_ExpressionRHS(){ return mExpression2.getProblemElements_Expression(); }

    string getTermLHS(int nTerm){ return mExpression1.getTerm(nTerm); }

    string getTermRHS(int nTerm){ return mExpression2.getTerm(nTerm); }

    string getExpressionLHS(){ return mExpression1.getExpression(); }

    string getExpressionRHS(){ return mExpression2.getExpression(); }

    int getNumTermsLHS(){ return mExpression1.getNumTerms(); }

    int getNumTermsRHS(){ return mExpression2.getNumTerms(); }

    // - getters
    bool isValidLHS(){ return mExpression1.isValid(); }

    bool isValidRHS(){ return mExpression2.isValid(); }

    bool getXBeforeY(Variable mXVar, Variable mYVar) { return mExpression1.getXBeforeY(mXVar, mYVar); }

    vector<int> getProblemElements_ResultsCombined(){
        vector <int> vCombinedProblemSpace;
        vector <int> vProblemSpace1 = mExpression1.getProblemElements_Result();
        vector <int> vProblemSpace2 = mExpression2.getProblemElements_Result();

        //combine both problem spaces
        vector<int>::iterator it = vProblemSpace1.begin();
        vector<int>::iterator jit = vProblemSpace2.begin();

        if(vProblemSpace1.empty()){
            vCombinedProblemSpace = vProblemSpace2;
        } else if (vProblemSpace2.empty()){
                vCombinedProblemSpace = vProblemSpace1;
        } else {
            while (it != vProblemSpace1.end() && jit != vProblemSpace2.end()){
                if (it == vProblemSpace1.end())	{
                        vCombinedProblemSpace.push_back(*jit);
                        jit++;
                }
                else if (jit == vProblemSpace2.end())	{
                        vCombinedProblemSpace.push_back(*it);
                        it++;
                }
                else if (*it > *jit)	{
                        vCombinedProblemSpace.push_back(*jit);
                        jit++;
                }
                else if (*it == *jit)	{
                        vCombinedProblemSpace.push_back(*jit);
                        vCombinedProblemSpace.push_back(*it);
                        it++;
                        jit++;
                }
                else if (*it < *jit)	{
                        vCombinedProblemSpace.push_back(*it);
                        it++;
                }
            }
        }

        return vCombinedProblemSpace;
    }

    // - evaluation
    vector<bool> evaluate(){
		vector<bool> vResult;

        vEvalArray1 = mExpression1.evaluateAll();
        vEvalArray2 = mExpression2.evaluateAll();

        for (unsigned int i = 0; i < vEvalArray1.size(); i++){
            switch (nSymbol){
            case SYMBOL_SMALLER_THAN:
//                cout << vEvalArray1[i] << " < " << vEvalArray2[i] << ": " << (vEvalArray1[i] < vEvalArray2[i]) << endl;
                vResult.push_back(vEvalArray1[i] < vEvalArray2[i]);
                break;
            case SYMBOL_GREATER_THAN:
//                cout << vEvalArray1[i] << " > " << vEvalArray2[i] << ": " << (vEvalArray1[i] > vEvalArray2[i]) << endl;
                vResult.push_back(vEvalArray1[i] > vEvalArray2[i]);
                break;
            case SYMBOL_SMALLER_THAN_EQUAL:
                vResult.push_back(vEvalArray1[i] <= vEvalArray2[i]);
                break;
            case SYMBOL_GREATER_THAN_EQUAL:
                vResult.push_back(vEvalArray1[i] >= vEvalArray2[i]);
                break;
            case SYMBOL_APPROX_EQUAL:
                {
                    double diff = (vEvalArray1[i]-vEvalArray2[i])*(vEvalArray1[i]-vEvalArray2[i]);
                    vResult.push_back((dPrecision*dPrecision) >= diff);
                    break;
                }
            default:
                cerr << "Unknown symbol" << endl;
                assert (false && "Unknown inequality operator");
                break;
            }
		}
        return vResult;
	}

    // - validation
    bool variableIsValid (Variable & myVar){ return mExpression1.variableNameIsValid(myVar); }

    string getErrors(){
        sErrorMessage += mExpression1.getErrors();
        sErrorMessage += mExpression2.getErrors();
        return sErrorMessage;
    }

}; // Inequality

#endif
