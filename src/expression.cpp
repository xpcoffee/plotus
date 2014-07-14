/*	Author(s):	Emerick Bosch
	Build:		0.1
	Date:		April 2014

	expression.cpp
	-------------
	
	"""""""""""""""""""""""""""""""""	
	"			Description			"
	"""""""""""""""""""""""""""""""""	
	
	Class Functions

	Interprets mathematical expressions from strings.
	Parses the strings iteratively into arrays.
	Arrayes solved iteratively.
*/

/*
    BREAK
        ln(x) breaking when x = 0
    VALIDATION
        TODO: pi without brackets must be able to be used as a variable
        TODO: uninitialized variables not highlighting properly when followed by powers

    MATHS
        TODO: Expression | handle inf

    FEATURES / GUI
        TODO: clear button
        TODO: error display

    LONG TERM
        TODO: create report of plot
*/

//	"""""""""""""""""""""""""""""""""	
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""	

    #include"include/expression.h"


//	"""""""""""""""""""""""""""""""""	
//	"			Namespaces			"
//	"""""""""""""""""""""""""""""""""	
	using namespace std;

//	"""""""""""""""""""""""""""""""""	
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""	

//	Setters
//	-------

void Expression::setExpression(string sInput){
	vOriginalExpression = parseExpressionArray(sInput);
    resetExpression();
    vProblemElements_Expression = checkExpressionArray(vOriginalExpression);
}

void Expression::addVariable(Variable myVar){
    assert(variableNameIsValid(myVar));
	vVariables.push_back(myVar);
}

void Expression::clearVariables(){ vVariables.clear(); }

void Expression::resetExpression(){ vExpression = vOriginalExpression; }

//	Parsing
//	-------
	
bool Expression::charIsDigit(char c){ return (('0' <= c) && (c <= '9')) || (c == '.'); }

bool Expression::charIsAlpha(char c){ return (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z')) || (c == '_'); }

bool Expression::charIsParenthesis(char c){ return (c == '(') || (c == ')'); }

bool Expression::charIsOperator(char c){
	return 	(c == '+') || (c == '-') ||
            (c == '*') || (c == '/') ||
            (c == '^');
}

bool Expression::charIsWhitespace(char c){
	return 	(c == ' ')  || (c == '\t') || (c == '\n')  || (c == '\r') ||
			(c == '\v') || (c == '\b') || (c == '\f');
}

vector<string> Expression::parseExpressionArray (string sExpression){
//	int nTerm = 0;
	// FUTURE: check if there is better way to declare dynamic array
	string sTerm = "";
	vector<string> vTermArray;
	string::iterator it = sExpression.begin();
	// flags that help determine wether a digit is start of number
	// and whether a minus is negative
	// both set to true to cater for the first term
	bool flag_newNum = true;
	bool flag_prevOp = true;
	while (it != sExpression.end()){
		if (charIsDigit(*it) || charIsAlpha(*it)){
            // if digit is the start of a new number, put it in the next term
			if (flag_newNum){
				sTerm = "";
				sTerm += *it;
			}
			else {
				sTerm += *it;
			}
			// a digit following this one, will not be the start of a new number
			flag_newNum = false;
            // this was not an operator
			flag_prevOp = false;
		} 
		else if (charIsParenthesis(*it) || charIsOperator(*it)){
			// if there was a number before this character, push it
			if (!flag_newNum){
				vTermArray.push_back(sTerm);
			}
			sTerm = "";
			sTerm += *it;
			// a digit following this term will be the start of a new number
			flag_newNum = true;
			// unless this is a '-' that shows a negative number
			// done by checking for combination of 'operator/parenthesis' followed by minus OR
			// minus is the first term (taken care of already by the if)
			if (*it == '-' && flag_prevOp) { flag_newNum = false; } 
			else { vTermArray.push_back(sTerm); }
			// if this is an operator, set the prev operator flag
			if (charIsOperator(*it)) { flag_prevOp = true; } 
			else { flag_prevOp = false; }
        } else if (charIsWhitespace(*it)){
            // do nothing, char is still recognized
        } else {
            // add it to expression, it will be highlighted in checkExpressionArray
            if (!flag_newNum){
                vTermArray.push_back(sTerm);
            }
            sTerm = "";
            sTerm += *it;
            vTermArray.push_back(sTerm);
            flag_newNum = true;
            flag_prevOp = false;
            ostringstream buffer;
            buffer << "Problem | Input | Illegal character in expression: " << sTerm << " | ASCII Value: " << (int)*it << "\n";
            sErrorMessage += buffer.str();
        }
		it++;	
	}
	// if the last term is a number, push it
	if (!flag_newNum){
		vTermArray.push_back(sTerm);
	}
	nTerms = vTermArray.size();
	return vTermArray;
}

//	Checking and Error Handling
//	---------------------------
// [BREAK] error message for variables starting with number
bool Expression::check_DecimalPointOK(string sTerm){
   int nDecimalPoints = 0;
   bool flag_IsVar = false;
   for(string::iterator it = sTerm.begin(); it != sTerm.end(); it++) {
       flag_IsVar = flag_IsVar || charIsAlpha(*it);
       if (*it == '.' || *it == ',') { nDecimalPoints++; }
       if (nDecimalPoints > 1) {
           return false; }
       else if (nDecimalPoints > 0 && flag_IsVar) { return false;} // decimal points not allowed in vars
   }
   return true; // if no problem
}

bool Expression::check_NumbersOK(string sTerm){
    if (termIsNumeric(sTerm)){
        for (string::iterator it = sTerm.begin(); it != sTerm.end(); it++){
            if (!charIsDigit(*it)){
                sErrorMessage += "Problem | Input | Variables may not start with a number.\n";
                return false; // numbers must only be made out of digits
            }
        }
    }
    return true;
}

bool Expression::check_CharsOK(string sTerm){
    for (string::iterator it = sTerm.begin(); it != sTerm.end(); it++){
        if (!charIsValid(*it))
        {
            return false;	// illegal character
        }
    }
    return true; // if no problem
}

bool Expression::check_OperatorsOK(string sTerm, int nTerm, int nSize, bool & flag_prev){
    if (charIsOperator(sTerm[0]) && (sTerm.size() == 1)){
        // consecutive operators
        if (flag_prev)
            return false;

        flag_prev = true; 	// set flag | placement important - after flag check and before the other checks

        if ((nTerm == 0 && sTerm[0] != '-') || (nTerm == nSize-1))
            return false;
    } else { flag_prev = false; }
   return true; // if no problem
}

vector<int> Expression::checkExpressionArray(vector<string> & vExpression){
    vector<int> vErrorTerms, vErrorParenth;
    vector<string> vTermsBeforeParenth;
    vector<int> vParenthRangeStart;
    string sTerm;
    int nTerm = 0;
    bool flag_prevOperator = false;
    bool flag_checksPassed;

    flag_isValid = true;

    for (vector<string>::iterator it = vExpression.begin(); it != vExpression.end(); it++){
       sTerm = *it;
       // input checks
       flag_checksPassed = 	check_CharsOK(sTerm)		&&
                            check_NumbersOK(sTerm)		&&
                            check_OperatorsOK(sTerm, nTerm, static_cast<int>(vExpression.size()), flag_prevOperator)	&&
                            check_DecimalPointOK(sTerm);

       if(!flag_checksPassed) {
           vErrorTerms.push_back(nTerm); // incorrect input in current term
           flag_isValid = false;
       }
       // parentheses checks
       if (sTerm[0] == '('){
           vErrorParenth.push_back(nTerm);
           vParenthRangeStart.push_back(nTerm);

           if (nTerm > 0)	// keep track of term before parenth - in case of function
               vTermsBeforeParenth.push_back(vExpression[nTerm-1]);
       }
       else if (sTerm[0] == ')'){
           // unopened parentheses
           if (vErrorParenth.size() == 0){
               flag_isValid = false;
               vErrorTerms.push_back(nTerm);
               sErrorMessage += "Problem | Input | Unopened parenthesis.\n";
           }
           else {
               vErrorParenth.pop_back();
           }

           // check if contents of parenth match what is needed by function/standard value
           if (!vTermsBeforeParenth.empty() && termIsFunction(vTermsBeforeParenth.back())	// functions need non-empty parenth
                   && (nTerm - vParenthRangeStart.back()) == 1){
               flag_isValid = false;
               vErrorTerms.push_back(vParenthRangeStart.back()-1);
               sErrorMessage += "Problem | Input | Empty parentheses - function requires a value in parentheses.\n";
           } else if (!vTermsBeforeParenth.empty() && termIsStandardValue(vTermsBeforeParenth.back())
                      && (nTerm - vParenthRangeStart.back()) > 1){ 	// value with non-empty parenth
               flag_isValid = false;
               vErrorTerms.push_back(vParenthRangeStart.back()-1);
               sErrorMessage += "Problem | Input | Non-empty parentheses - value requires parentheses to be empty.\n";
           }
           if(!vTermsBeforeParenth.empty()){
                vTermsBeforeParenth.pop_back();
                vParenthRangeStart.pop_back();
           }
       }
       nTerm++;
    }
    // unclosed parentheses
    for (vector<int>::iterator it = vErrorParenth.begin(); it != vErrorParenth.end(); it++){
        vErrorTerms.push_back(*it);
        sErrorMessage += "Problem | Input | Unclosed parenthesis.\n";
        flag_isValid = false;
    }

    // return
    return vErrorTerms;
}

bool Expression::variableNameIsUnique(Variable& myVar){
    for (vector<Variable>::iterator it = vVariables.begin(); it != vVariables.end(); it++){
            if ((*it).getName() == myVar.getName()){ return false; }
    }
    return true;
}

bool Expression::variableNameIsValid(Variable & myVar){
    string sName = myVar.getName();
    if (!Variable::nameIsLegal(sName))
        sErrorMessage += "Problem | Input | Variable name is illegal." + sName + "\n";
    if (!variableNameIsUnique(myVar))
        sErrorMessage += "Problem | Input | Variable name is not unique: " + sName + "\n";
    return Variable::nameIsLegal(sName) && variableNameIsUnique(myVar);
}

bool Expression::charIsValid(char c){
    return charIsAlpha(c) || charIsDigit(c) || charIsOperator(c) || charIsParenthesis(c) || charIsWhitespace(c);
}

bool Expression::termIsNumeric(string sTerm){ return charIsDigit(sTerm[0]); } // reason why variable names may not start with digit

bool Expression::termIsAlpha(string sTerm){ return charIsAlpha(sTerm[0]); }

bool Expression::termIsStandardValue(string sTerm){ return Variable::isStandardValue(sTerm); }

bool Expression::termIsFunction(string sTerm){ return Variable::isFunction(sTerm); }

//	Evaluation
//	-----------

bool Expression::doPowers(vector<string> & vExpression) {
    nTerms = vExpression.size();
    for (int i = nTerms-1; i > 0; i--){ // powers must be done right to left (directional)
        string sTerm = vExpression[i];
        // if operation found and it is the correct operation
        if (charIsOperator(sTerm[0]) && sTerm[0] == '^'){
                //do the math
                string termBeforeOperator = vExpression[i-1];
                string termAfterOperator  = vExpression[i+1];
                double dBefore = atof(termBeforeOperator.c_str());
                double dAfter = atof(termAfterOperator.c_str());
                if ((dBefore < 0) && (fmod(dBefore, 1) != 0) && (fmod(dAfter, 1) != 0))
                    throw MATH_COMPLEX;
                if ((dBefore == 0) && (dAfter == 0))
                    throw MATH_NAN;
                if ((dBefore == 0) && (dAfter < 0))
                    throw MATH_POLE;
                double result =  pow(atof(termBeforeOperator.c_str()), atof(termAfterOperator.c_str()));
                // update expression - operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                vExpression[i-1] = buffer.str();
                vExpression[i]   = COMPRESSION_CHAR;
                vExpression[i+1] = COMPRESSION_CHAR;
                // shift rest of expression into hitespace
                while(compressExpression(vExpression)){
                    // loop will auto-terminate
                }
                // if an operation was done:
                return true;
        }
    }
    //if no operation found
    return false;
}

bool Expression::doMultiplication (vector<string>& vExpression) {
	nTerms = vExpression.size();
	for (int i = 0; i < nTerms; i++){
		string sTerm = vExpression[i];
		// if operation found and it is the correct operation
		if (charIsOperator(sTerm[0]) && sTerm[0] == '*'){
				//do the math
				string termBeforeOperator = vExpression[i-1];
				string termAfterOperator  = vExpression[i+1];
                double result = atof(termBeforeOperator.c_str()) * atof(termAfterOperator.c_str());
				// update expression - operator and second value filled with special character 
				ostringstream buffer;
				buffer << result;
				vExpression[i-1] = buffer.str();
				vExpression[i]   = COMPRESSION_CHAR;
				vExpression[i+1] = COMPRESSION_CHAR;
				// shift rest of expression into hitespace
				while(compressExpression(vExpression)){
					// loop will auto-terminate	
				}
				// if an operation was done:
				return true;	
		}
	}
	// if no operation found
	return false;
}

bool Expression::doDivision (vector<string>& vExpression) {
	nTerms = vExpression.size();
	for (int i = 0; i < nTerms; i++){
		string sTerm = vExpression[i];
		// if operation found and it is the correct operation
		if (charIsOperator(sTerm[0]) && sTerm[0] == '/'){
				//do the math
				string termBeforeOperator = vExpression[i-1];
				string termAfterOperator  = vExpression[i+1];
                // [ERROR] Expression | doDivision() | no consecutive operators allowed
                assert( !((termAfterOperator.size() == 1) &&
                        charIsOperator(termAfterOperator[0])) );
                if (atof(termAfterOperator.c_str()) == 0)
                    throw MATH_DIVIDE_BY_ZERO;
                double result = atof(termBeforeOperator.c_str()) / atof(termAfterOperator.c_str());
				// update expression - operator and second value filled with special character 
				ostringstream buffer;
				buffer << result;
				vExpression[i-1] = buffer.str();
				vExpression[i]   = COMPRESSION_CHAR;
				vExpression[i+1] = COMPRESSION_CHAR;
				// shift rest of expression into whitespace
				while(compressExpression(vExpression)){
					// loop will auto-terminate	
				}
                return true; // if an operation was done:
        }
	}
    return false; // if no operation found
}

bool Expression::doAddition (vector<string>& vExpression) {
	nTerms = vExpression.size();
	for (int i = 0; i < nTerms; i++){
		string sTerm = vExpression[i];
		// if operation found and it is the correct operation
		if (charIsOperator(sTerm[0]) && sTerm[0] == '+'){
				//do the math
				string termBeforeOperator = vExpression[i-1];
				string termAfterOperator  = vExpression[i+1];
                double result = atof(termBeforeOperator.c_str()) + atof(termAfterOperator.c_str());
				// update expression - operator and second value filled with special character 
				ostringstream buffer;
				buffer << result;
				vExpression[i-1] = buffer.str();
				vExpression[i]   = COMPRESSION_CHAR;
				vExpression[i+1] = COMPRESSION_CHAR;
				// shift rest of expression into whitespace
				while(compressExpression(vExpression)){
					// loop will auto-terminate	
				}
				// if an operation was done:
				return true;	
		}
	}
	// if no operation found
	return false;
}

bool Expression::doSubtraction (vector<string>& vExpression) {
	nTerms = vExpression.size();
	for (int i = 0; i < nTerms; i++){
		string sTerm = vExpression[i];
		// if operation found and it is the correct operation
		if (charIsOperator(sTerm[0]) && sTerm[0] == '-' && !charIsDigit(sTerm[1])){
            // if '-' is at the beginning of expression , it signifies a negative number
            double result;
            if (i == 0){
                // multiply the term after the minus by -1
                string termAfterOperator  = vExpression[i+1];
                result = -1*atof(termAfterOperator.c_str());
                // update expression - operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                vExpression[i] = buffer.str();
                vExpression[i+1] = COMPRESSION_CHAR;
            } else {
                // do the math
                string termBeforeOperator = vExpression[i-1];
                string termAfterOperator  = vExpression[i+1];
                result = atof(termBeforeOperator.c_str()) - atof(termAfterOperator.c_str());
                // update expression - operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                vExpression[i-1] = buffer.str();
                vExpression[i]   = COMPRESSION_CHAR;
                vExpression[i+1] = COMPRESSION_CHAR;
            }
            // shift rest of expression into whitespace
            while(compressExpression(vExpression)){
                // loop will auto-terminate
            }
            // if an operation was done:
            return true;
        }
	}
	// if no operation found
	return false;
}

void Expression::doBasic(vector<string>& vExpression) {
    while (doPowers(vExpression)) {}
    while (doDivision(vExpression)) {}
    while (doMultiplication(vExpression)) {}
    while (doSubtraction(vExpression)) {}
    while (doAddition(vExpression)) {}
}

bool Expression::compressExpression(vector<string>& vExpression) {
    nTerms = vExpression.size();
    for (int i = 0; i < nTerms; i++){
        string sTerm = vExpression[i];
        // if compression character is found
        if (COMPRESSION_CHAR == sTerm[0]){
            // shift everything after the compression character left by 1
            for (int j = i; j < nTerms-1; j++){
                vExpression[j] = vExpression[j+1];
            }
            // delete last term
            vExpression.pop_back();
            return true;
        }
    }
    // if no whitespace is found
    return false;
}

bool Expression::doParenthesis (vector<string>& vExpression) {
    nTerms = vExpression.size();
    int nOpen = UNINITIALIZED_COUNTER;
    int nRange = UNINITIALIZED_COUNTER;
    string sTerm;
    string sTermBefore, sTermAfter;
    bool flag_EmptyParenth;
    // find parenth
	for (int i = 0; i < nTerms; i++){
        sTerm = vExpression[i];
		if (charIsParenthesis(sTerm[0]) && sTerm[0] == '('){
            nOpen = i;	// opening parentheses found, start counting range
            if (i > 0)
                sTermBefore = vExpression[i-1];
		}
		else if (charIsParenthesis(sTerm[0]) && sTerm[0] == ')'){
            assert(nOpen != UNINITIALIZED_COUNTER); // check for unopened bracket
            if (i+1 < nTerms)
                sTermAfter = vExpression[i+1];
            nRange = i - nOpen;	// closing parentheses found, stop counting range
            if (nRange > 1){
                flag_EmptyParenth = false;			// parentheses not empty
                vector<string> sParenthesisArray; 	// create new array from contents
                for (int j = 0; j < nRange -1; j++){
                    sParenthesisArray.push_back(vExpression[nOpen+1+j]);
                }
                doBasic(sParenthesisArray); 				// perform maths on contents
                vExpression[nOpen] = sParenthesisArray[0]; 	// substitute back the result
            }
            else {
                flag_EmptyParenth = true; 	// parentheses empty
                vExpression[nOpen] = "0"; 	// substitute parentheses with 0
            }
			for (int j = nOpen+1; j < i+1; j++){
                vExpression[j] = COMPRESSION_CHAR;	// compress expression
			}
            // parenthesis multiplication
            bool flag_doneBefore = false;
            if (!sTermBefore.empty()){				// there is term before parenth
                if (termIsNumeric(sTermBefore)){ 	// term before parenth is number
                    vExpression[nOpen+1] = vExpression[nOpen];	// shift answer up by 1
                    vExpression[nOpen] = "*"; 					// convert to multiplication
                    flag_doneBefore = true;
                }
            }
            if (!sTermAfter.empty()){				// there is term before parenth
                if (termIsNumeric(sTermAfter)){ 	// term before parenth is number
                    if (flag_EmptyParenth){			// if parenth were empty (need to deal with differently)
                        if (flag_doneBefore){
                                vExpression[nOpen+2] = COMPRESSION_CHAR;
                        } else {
                                vExpression[nOpen+1] = "*";
                        }
                    } else {
                        vExpression[nOpen+2] = "*"; 	// convert to multiplication
                    }
                }
            }
            // compress expression
            while(compressExpression(vExpression)){
				// loop will auto-terminate	
			}
            // do special operations (sin, log, etc)
            doSpecial(vExpression, nOpen, flag_EmptyParenth);
			return true;
		}
	}
    assert((nOpen == UNINITIALIZED_COUNTER) && (nRange == UNINITIALIZED_COUNTER)); // check for unclosed bracket
    // no parenthesis was found
	return false;
}

void Expression::doSpecial(vector<string> & vExpression, int nEvalPos, bool flag_EmptyParenth){
    // if the brackets were right at the beginning, nothing before them
    // so no special function to be done
    if (nEvalPos < 1) { return; }
    string termBeforeParenthesis = vExpression[nEvalPos-1];
    string sEvalTerm = vExpression[nEvalPos];
    nProblemTerm = nEvalPos;
    double result;
    //trig functions
    if (termBeforeParenthesis == "sin"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = sin(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "cos"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = cos(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "tan"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = tan(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "sinh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = sinh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "cosh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = cosh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "tanh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = tanh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arcsin"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = asin(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arccos"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = acos(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arctan"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = atan(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arcsinh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = asinh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arccosh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = acosh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "arctanh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = atanh(atof(sEvalTerm.c_str()));
    }
    //	negative
    else if (termBeforeParenthesis == "-sin"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -sin(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-cos"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -cos(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-tan"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -tan(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-sinh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -sinh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-cosh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -cosh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-tanh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -tanh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arcsin"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -asin(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arccos"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -acos(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arctan"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -atan(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arcsinh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -asinh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arccosh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -acosh(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-arctanh"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -atanh(atof(sEvalTerm.c_str()));
    }
    //	exponential
    else if(termBeforeParenthesis == "exp"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = exp(atof(sEvalTerm.c_str()));
    }
    else if(termBeforeParenthesis == "-exp"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        result = -exp(atof(sEvalTerm.c_str()));
    }
    // logarithmic functions
    else if(termBeforeParenthesis == "log"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        double dEval = atof(sEvalTerm.c_str());
        if (dEval == 0)
            throw MATH_POLE;
        result = log10(dEval);
    }
    else if(termBeforeParenthesis == "ln"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        double dEval = atof(sEvalTerm.c_str());
        if (dEval == 0)
            throw MATH_POLE;
        result = log(dEval);
    }
    else if(termBeforeParenthesis == "-log"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        double dEval = atof(sEvalTerm.c_str());
        if (dEval == 0)
            throw MATH_POLE;
        result = -log10(dEval);
    }
    else if(termBeforeParenthesis == "-ln"){
        if (flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_EMPTY;
        double dEval = atof(sEvalTerm.c_str());
        if (dEval == 0)
            throw MATH_POLE;
        result = -log(dEval);
    }
    // values
    else if(termBeforeParenthesis == "pi"){
        if (!flag_EmptyParenth){
            throw INPUT_ERROR_PARENTH_NOT_EMPTY;
        }
        result = PI;
    }
    else if(termBeforeParenthesis == "-pi"){
        if (!flag_EmptyParenth)
            throw INPUT_ERROR_PARENTH_NOT_EMPTY;
        result = -1*PI;
    }
    // no matching function
    else { return; }

    // return evaluated value
    // - can only get here if 'break' out of switch
    ostringstream buffer;
    buffer << result;
    if ((buffer.str() == "nan") || (buffer.str() == "-nan"))
        throw MATH_NAN;
    vExpression[nEvalPos-1] = buffer.str();
    vExpression[nEvalPos] = COMPRESSION_CHAR;

    while(compressExpression(vExpression)){
        // loop will auto-terminate
    }
}

double Expression::evaluateExpression(){
    if (!flag_isValid)
        throw INPUT_ERROR_INVALID_EXPRESSION; // [DOCUMENTATION] invalid expression

    while (doParenthesis(vExpression)) {}
    // evaluate reduced expression
    doBasic(vExpression);
    string tmpString = vExpression[0];
    double result = atof(tmpString.c_str());
    resetExpression();
    return result;
}

void Expression::recEval(){
    if (!flag_isValid)
        throw INPUT_ERROR_INVALID_EXPRESSION;
    // for all values of the current variable (current variable is global)
	for (int i = 0; i < vVariables[nCurrentVariable].size(); i++){
			int j = nCurrentVariable;
            // if this is the final nested variable;
            // substitute current values and evaluate...
            if (nCurrentVariable == static_cast<int>(vVariables.size()) - 1){
                    // do math
                    subVariableValues();
                    double dResult;
                    try{
                        dResult = evaluateExpression();
                    }
                    catch(MATH_ERROR_CODES e){
                        // MATH ERRORS HANDLED HERE
                        // does not stop the evaluation, only logs a problem point
                        handleMathException(e);
                        resetExpression();
                        vProblemElements_Result.push_back(vResult.size());
                        dResult = 0;
                    }
                    catch(INPUT_ERROR_CODES e){
                        vProblemElements_Expression.push_back(nProblemTerm);
                        flag_isValid = false;
                    }

                    vResult.push_back(dResult);
			}
            // ...otherwise move on to the next nested variable level
            else {
                    nCurrentVariable++; 	// go to the next variable
                    recEval(); 				// recurse through it
                    nCurrentVariable--;		// come back to this variable to carry on with it
			}
			vVariables[j].nextPosition();
	}
}

//	Output
//	------

string Expression::getStringArray(vector<string> vExpression){
	nTerms = vExpression.size();
	string sExpression = "";
    for (unsigned int i = 0; i < vExpression.size(); i++){
		sExpression += vExpression[i];
	}
	return sExpression;
}

// 	Exception Handling and Validation
//	---------------------------------

void Expression::handleMathException(MATH_ERROR_CODES e){

    switch(e){
    case MATH_DIVIDE_BY_ZERO:
        if (!flag_DivByZero)
            sErrorMessage += "Warning | Evaluation | Found division(s) by zero.\n";
        flag_DivByZero = true;
        break;
    case MATH_NAN:
        if (!flag_Nan)
            sErrorMessage += "Warning | Evaluation | Found occurrence(s) of NaN (not a number). Caused by invalid mathematical operation.\n";
        flag_Nan = true;
        break;
    case MATH_POLE:
        if (!flag_Pole)
            sErrorMessage += "Warning | Evaluation | Found pole(s).\n";
        flag_Pole = true;
        break;
    case MATH_COMPLEX:
        if (!flag_Complex)
            sErrorMessage += "Warning | Evaluation | Found complex number result(s).\n";
        flag_Complex = true;
        break;
    default:
        sErrorMessage += "Bug | Evaluation | Unhandled math exception. | Please report this bug to the following email address:\n emerick.bosch+bugmail@gmail.com\n";
        break;
    }
}

void Expression::resetEvaluation(){
    nCurrentVariable = 0;
    vResult.clear();
    flag_Nan = false;
    flag_Pole = false;
    flag_DivByZero = false;
    flag_Complex = false;
}

//	"""""""""""""""""""""""""""""""""	
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""	

//	Getters
//	-------

string Expression::getExpression(){ return getStringArray(vOriginalExpression); }

string Expression::getTerm(int nTerm){ // [DOCUMENTATION] getTerm() | 0th terms are a thing in DePlot
    assert(!(nTerm < 0 || nTerm >= static_cast<int>(vExpression.size())) && "getTerm: Out of Bounds");
    return vOriginalExpression[nTerm];
}

string Expression::getErrors(){ return sErrorMessage; }

bool Expression::getXBeforeY(Variable mXVar, Variable mYVar){
    for (int i = 0; i < static_cast<int>(vVariables.size()); i++){
        if (vVariables[i].getName() == mXVar.getName())	{ return true; }
        else if (vVariables[i].getName() == mYVar.getName()) { return false; }
    }
    assert (false && "getXBeforeY: Neither X nor Y variables were found. Make sure they are added.");
    return false; 	// will never get here, used just for synta
}

int Expression::getNumTerms(){ return static_cast<int> (vExpression.size()); }

bool Expression::isValid(){ return flag_isValid; }

vector<int> Expression::getProblemElements_Expression(){ return vProblemElements_Expression; }

vector<int> Expression::getProblemElements_Result(){ return vProblemElements_Result; }

//	Evaluation
//	-----------
void Expression::subVariableValues(){
	nTerms = vExpression.size();
    for (int j = 0; j < nTerms; j++){	// for all terms
        string sTerm = vExpression[j];
        bool flag_initialized = false;
        for (int i = 0; i < static_cast<int>(vVariables.size()); i++){ // for all variables
            if (vVariables[i].getName() == sTerm){
                ostringstream buffer;
                buffer << vVariables[i].getCurrentValue();
                vExpression[j] = buffer.str();
                flag_initialized = true;
            } else if (("-" + vVariables[i].getName()) == sTerm){
                ostringstream buffer;
                buffer << -1 * vVariables[i].getCurrentValue();
                vExpression[j] = buffer.str();
                flag_initialized = true;
            }
		}
        if (!flag_initialized && Variable::nameIsLegal(sTerm)){
            vProblemElements_Expression.push_back(j);
            flag_isValid = false;
            sErrorMessage += "Problem | Input | Uninitialized variable: " + sTerm + "\n";
            throw INPUT_ERROR_UNINITIALIZED_VARIABLE; // [DOCUMENTATION] unitialized variable
        }
    }
}

vector<double> Expression::evaluateAll(){
    if (!flag_isValid)
        throw INPUT_ERROR_INVALID_EXPRESSION; // [DOCUMENTATION] invalid expression

    resetEvaluation();
	recEval();
	return vResult;
}


//	Utility
//	-------

//static bool Expression::approxEqual(double dNum1, double dNum2, double dPrec){
//    if ((dNum1 == null) || (dNum2 == null) || (dPrec == null))
//        return false;
//    return sqrt((dNum1-dNum2)*(dNum1-dNum2)) <= dPrec;
//}
