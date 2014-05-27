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
    vector<int> getProblemElements_Expression1(){
        return mExpression1.getProblemElements_Expression();
    }

    vector<int> getProblemElements_Expression2(){
        return mExpression2.getProblemElements_Expression();
    }

    vector<int> getProblemElements_Result(){
        vector <int> vReturnProblemSpace;
        vector <int> vProblemSpace1 = mExpression1.getProblemElements_Result();
        vector <int> vProblemSpace2 = mExpression2.getProblemElements_Result();

        cout << "[INFO] getProblemElements_Result() | " << "vProblemSpace1: ";
        for (vector<int>::iterator it = vProblemSpace1.begin(); it!=vProblemSpace1.end(); it++){
            cout << *it << ", ";
        }
        cout << endl;

        cout << "[INFO] getProblemElements_Result() | " << "vProblemSpace2: ";
        for (vector<int>::iterator it = vProblemSpace2.begin(); it!=vProblemSpace2.end(); it++){
            cout << *it << ", ";
        }
        cout << endl;

        //combine both problem spaces
        vector<int>::iterator it = vProblemSpace1.begin();
        vector<int>::iterator jit = vProblemSpace2.begin();

        if(vProblemSpace1.empty()){
            vReturnProblemSpace = vProblemSpace2;
        } else if (vProblemSpace2.empty()){
                vReturnProblemSpace = vProblemSpace1;
        } else {
            while (it != vProblemSpace1.end() && jit != vProblemSpace2.end()){
                if (it == vProblemSpace1.end())	{
                        vReturnProblemSpace.push_back(*jit);
                        jit++;
                }
                else if (jit == vProblemSpace2.end())	{
                        vReturnProblemSpace.push_back(*it);
                        it++;
                }
                else if (*it > *jit)	{
                        vReturnProblemSpace.push_back(*jit);
                        jit++;
                }
                else if (*it == *jit)	{
                        vReturnProblemSpace.push_back(*jit);
                        vReturnProblemSpace.push_back(*it);
                        it++;
                        jit++;
                }
                else if (*it < *jit)	{
                        vReturnProblemSpace.push_back(*it);
                        it++;
                }
            }
        }


        cout << "[INFO] getProblemElements_Result() | " << "Combined space: ";
        for (vector<int>::iterator it = vReturnProblemSpace.begin(); it!=vReturnProblemSpace.end(); it++){
            cout << *it << ", ";
        }
        cout << endl;

        return vReturnProblemSpace;
    }

    vector<bool> evaluate(){
		vector<bool> vResult;
		vEvalArray1 = mExpression1.evaluateAll();	
		vEvalArray2 = mExpression2.evaluateAll();	
//        cout << "[INFO] evaluate() |" << " symbol: " << cSymbol << endl;
//        cout << "[BEGIN INFO BLOCK] evaluate() |" << " check inequality" << endl;
        for (unsigned int i = 0; i < vEvalArray1.size(); i++){
			switch(cSymbol){
				case '<':
					vResult.push_back(vEvalArray1[i] < vEvalArray2[i]);
//                    cout << vEvalArray1[i] << "\t<\t" << vEvalArray2[i] << "\t:" << (vEvalArray1[i] < vEvalArray2[i]) << endl;
					break;
				case '>':
					vResult.push_back(vEvalArray1[i] > vEvalArray2[i]);
//                    cout << vEvalArray1[i] << "\t>\t" << vEvalArray2[i] << "\t:" << (vEvalArray1[i] > vEvalArray2[i]) << endl;
                    break;
			}
		}
//        cout << "[END INFO BLOCK] evaluate() |" << endl;
        return vResult;
	}

}; // Inequality

#endif
