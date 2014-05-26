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

    // # variables #
	Expression mExpression1, mExpression2;
	char cSymbol;
	vector<Variable> vVariables;
    vector<double> vEvalArray1, vEvalArray2;
    bool flag_set;
	
    //	"""""""""""""""""""""""""""""""""
    //	"			Public				"
    //	"""""""""""""""""""""""""""""""""
public:

    // # constructor #
    Inequality(string sExp1 = "", char symbol = '!', string sExp2 = ""){
        if (symbol == '!'){
            flag_set = false;
        }
        else{
            mExpression1.setExpression(sExp1);
            mExpression2.setExpression(sExp2);
            cSymbol = symbol;
            flag_set = true;
        }
	}

    // # setters #
	void clearVariables(){
        assert(flag_set);
        mExpression1.clearVariables();
        mExpression2.clearVariables();
	}

	void addVariable(Variable myVar){
        assert(flag_set);
        mExpression1.addVariable(myVar);
		mExpression2.addVariable(myVar);
	}

	void setSymbol(char c){
        assert(flag_set);
        cSymbol = c;
	}

    // # feature functions #
    vector<int> getProblemTerms1(){
        return mExpression1.getProblemTerms();
    }

    vector<int> getProblemTerms2(){
        return mExpression2.getProblemTerms();
    }

    vector<bool> evaluate(){
		vector<bool> vResult;
		vEvalArray1 = mExpression1.evaluateAll();	
		vEvalArray2 = mExpression2.evaluateAll();	
        cout << "symbol: " << cSymbol << endl;
        for (unsigned int i = 0; i < vEvalArray1.size(); i++){
			switch(cSymbol){
				case '<':
					vResult.push_back(vEvalArray1[i] < vEvalArray2[i]);
                    cout << vEvalArray1[i] << "\t<\t" << vEvalArray2[i] << "\t:" << (vEvalArray1[i] < vEvalArray2[i]) << endl;
					break;
				case '>':
					vResult.push_back(vEvalArray1[i] > vEvalArray2[i]);
                    cout << vEvalArray1[i] << "\t>\t" << vEvalArray2[i] << "\t:" << (vEvalArray1[i] > vEvalArray2[i]) << endl;
                    break;
			}
		}
		return vResult;
	}

}; // Inequality

#endif
