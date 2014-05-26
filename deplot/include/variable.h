/*	Author(s):	Emerick Bosch
	Build:		0.1
	Date:		April 2014

	variable.h
	-------------
	
	"""""""""""""""""""""""""""""""""	
	"			Description			"
	"""""""""""""""""""""""""""""""""	
	
	Class

	Stores name, range and values of a variable to be evaluated.
*/

#ifndef VARIABLE_H
#define VARIABLE_H

//	"""""""""""""""""""""""""""""""""	
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""	

#include<string>
#include<vector>
#include<iostream>
#include<cassert>

//	"""""""""""""""""""""""""""""""""	
//	"			Namespaces			"
//	"""""""""""""""""""""""""""""""""	
using namespace std;

//	"""""""""""""""""""""""""""""""""	
//	"		Class Definition: 		"
//	"			Variable			"
//	"""""""""""""""""""""""""""""""""	


class Variable
{
    //	"""""""""""""""""""""""""""""""""
    //	"			Private				"
    //	"""""""""""""""""""""""""""""""""
private:
    // # variables #
	string sName;
    vector<double> vValues;
    double dMin, dMax, dResolution;
    int nElements;
    int nCurrentPosition;
	bool flag_populated;

    // # functions #
	void populateValues(){
        for (double i = 0; i < nElements; i++){
            double result = dMin + i*dResolution;
			vValues.push_back(result);
		}
		nCurrentPosition = 0;
		flag_populated   = true;
	}

    //	"""""""""""""""""""""""""""""""""
    //	"			Public				"
    //	"""""""""""""""""""""""""""""""""
public:

    // # constructor #
    Variable(string name = "", double num1 = 0, double num2 = 0, int elements = 1){
		//	TODO: assert input is correct
		setName(name);
		setMinMax(num1, num2);
		setGrid(elements);
		// includes beginning and end
        dResolution = (dMax - dMin)/static_cast<double>(nElements-1);
		populateValues();
	}

    // # functions #
	void nextPosition(){
		assert(flag_populated);
		if (isEnd()){ nCurrentPosition = 0; }
		else { nCurrentPosition++; }
	} 

	bool isEnd(){
		if ( nCurrentPosition == nElements - 1 ){ return true; }
		else { return false; }
	}

    // # setters #
	void setName(string name){
		sName = name;
	}

    void resetPosition () { nCurrentPosition = 0; }

    void setMinMax(double num1, double num2){
		if (num1 > num2){
            dMin = num2;
            dMax = num1;
		}
		else {
            dMin = num1;
            dMax = num2;
		}
		flag_populated = false;
	}

    void setGrid(int elements){
		nElements      = elements;
		flag_populated = false;
	}

    // # getters #
	string getName()         { return sName; }
    int getCurrentPosition(){ return nCurrentPosition; }
    int getGrid() { return nElements; }
    int  size() { return vValues.size(); }
    double getMin()           { return dMin; }
    double getMax()           { return dMax; }
    double getResolution()    { return dResolution; }
    double getCurrentValue()	{
		assert(flag_populated);
		return vValues[nCurrentPosition];
	}
};

#endif
