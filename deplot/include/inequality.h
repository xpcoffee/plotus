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
    //	"""""""""""""""""""""""""""""""""
    //	"			Private				"
    //	"""""""""""""""""""""""""""""""""
private:

    // variables
    string sSymbol;
	vector<Variable> vVariables;
    vector<double> vEvalArray1, vEvalArray2;
    bool flag_initialized;
	
    //	"""""""""""""""""""""""""""""""""
    //	"			Public				"
    //	"""""""""""""""""""""""""""""""""
public:

    // variables
    Expression mExpression1, mExpression2;

    // constructor
    Inequality(string sExp1 = "", string symbol = "!", string sExp2 = ""){
        if (symbol == "!"){
            flag_initialized = false;
        }
        else{
            mExpression1.setExpression(sExp1);
            mExpression2.setExpression(sExp2);
            sSymbol = symbol;
            flag_initialized = true;
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

    void setInequality(string sExp1, string symbol, string sExp2){
            mExpression1.setExpression(sExp1);
            mExpression2.setExpression(sExp2);
            sSymbol = symbol;
            flag_initialized = true;
    }

    void changeSymbol(string symbol){
        assert(flag_initialized);
        sSymbol = symbol;
	}

    // - member variable getters
    vector<int> getProblemElements_ExpressionLHS(){
        return mExpression1.getProblemElements_Expression();
    }

    vector<int> getProblemElements_ExpressionRHS(){
        return mExpression2.getProblemElements_Expression();
    }

    string getTermLHS(int nTerm){
        return mExpression1.getTerm(nTerm);
    }

    string getTermRHS(int nTerm){
        return mExpression2.getTerm(nTerm);
    }

    string getExpressionLHS(){
        return mExpression1.getExpression();
    }

    string getExpressionRHS(){
        return mExpression2.getExpression();
    }

    int getNumTermsLHS(){
        return mExpression1.getNumTerms();
    }

    int getNumTermsRHS(){
        return mExpression2.getNumTerms();
    }

    // - getters
    bool isValidLHS(){
        return mExpression1.isValid();
    }

    bool isValidRHS(){
        return mExpression2.isValid();
    }

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
            if(sSymbol == "<"){
//                cout << vEvalArray1[i] << " < " << vEvalArray2[i] << ": " << (vEvalArray1[i] < vEvalArray2[i]) << endl;
                vResult.push_back(vEvalArray1[i] < vEvalArray2[i]);
            } else if (sSymbol == ">"){
//                cout << vEvalArray1[i] << " > " << vEvalArray2[i] << ": " << (vEvalArray1[i] > vEvalArray2[i]) << endl;
                vResult.push_back(vEvalArray1[i] > vEvalArray2[i]);
            } else if (sSymbol == "<="){
                vResult.push_back(vEvalArray1[i] <= vEvalArray2[i]);
            } else if (sSymbol == ">="){
                vResult.push_back(vEvalArray1[i] >= vEvalArray2[i]);
            } else {
                cerr << "Unknown symbol" << endl;
                assert (true);
            }
		}
        return vResult;
	}

    // - validation
    bool variableIsValid (Variable & myVar){
        if (!mExpression1.variableNameIsValid(myVar))
            cerr << "[ERROR] Inequality | variableIsValid() |" << "Invalid variable name: " << myVar.getName() << endl;
        return mExpression1.variableNameIsValid(myVar);
    }

}; // Inequality

#endif
