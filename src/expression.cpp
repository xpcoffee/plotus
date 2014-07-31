///	Includes
/// =========

    #include"include/expression.h"


///	Public Functions
///	=================

Expression::Expression(string expression):
    m_ErrorMessage("")
{
    setlocale(LC_NUMERIC,"C"); // make '.' the decimal separator

    // initialize
    setExpression(expression);
    resetEvaluation();
}


//	Setters
//	--------

void Expression::setExpression(string expression){
    // parse
    m_OriginalExpression = parseExpressionArray(expression);
    // validate
    m_Expression_Problems = checkExpressionArray(m_OriginalExpression);
    // assign expression to working expression
    resetExpression();
}

void Expression::addVariable(Variable variable){
    assert(variableNameIsValid(variable));
    m_Variables.push_back(variable);
}

void Expression::clearVariables(){ m_Variables.clear(); }


//	Getters
//	-------

int Expression::getNumTerms(){ return static_cast<int> (m_WorkingExpression.size()); }

string Expression::getExpression(){ return getStringArray(m_OriginalExpression); }

string Expression::getTerm(int term_pos)
{
    assert(!(term_pos < 0 || term_pos >= static_cast<int>(m_WorkingExpression.size())) && "getTerm: Out of Bounds");
    return m_OriginalExpression[term_pos];
}

string Expression::getErrors(){ return m_ErrorMessage; }


bool Expression::isValid(){ return flag_Valid; }

vector<int> Expression::getProblemElements_Expression(){ return m_Expression_Problems; }

vector<int> Expression::getProblemElements_Result(){ return m_Results_Problems; }

bool Expression::isXBeforeY(Variable x_variable, Variable y_variable)
{
    /*! Returns true if variable X is listed before variable Y in m_Variables*/

    for (int i = 0; i < static_cast<int>(m_Variables.size()); i++){
        if (m_Variables[i].name() == x_variable.name())	{ return true; }
        else if (m_Variables[i].name() == y_variable.name()) { return false; }
    }

    assert (false && "getXBeforeY: Neither X nor Y variables were found. Make sure they are added.");

    return false; 	// will never get here, used just for synta
}

//	Evaluation
//	-----------

void Expression::subVariableValues(){
    m_TermCount = m_WorkingExpression.size();
    for (int j = 0; j < m_TermCount; j++){	// for all terms
        string sTerm = m_WorkingExpression[j];
        bool flag_initialized = false;
        for (int i = 0; i < static_cast<int>(m_Variables.size()); i++){ // for all variables
            if (m_Variables[i].name() == sTerm){
                ostringstream buffer;
                buffer << m_Variables[i].getCurrentValue();
                m_WorkingExpression[j] = buffer.str();
                flag_initialized = true;
            } else if (("-" + m_Variables[i].name()) == sTerm){
                ostringstream buffer;
                buffer << -1 * m_Variables[i].getCurrentValue();
                m_WorkingExpression[j] = buffer.str();
                flag_initialized = true;
            }
        }
        if (!flag_initialized && Variable::nameIsLegal(sTerm)){
            m_Expression_Problems.push_back(j);
            flag_Valid = false;
            m_ErrorMessage += "Problem | Input | Uninitialized variable: " + sTerm + "\n";
            throw InputErrorUninitializedVariable; // [DOCUMENTATION] unitialized variable
        }
    }
}

vector<double> Expression::evaluateAll(){
    if (!flag_Valid)
        throw InputErrorInvalidExpression; // [DOCUMENTATION] invalid expression

    resetEvaluation();
    recEval();
    return m_Results;
}


///	Private Functions
///	==================


//	Parsing
//	--------

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

vector<string> Expression::parseExpressionArray (string expression){
    string current_term = "";
    vector<string> terms;
    string::iterator it = expression.begin();

    // flags that help determine wether a digit is start of number
    // and whether a minus is negative
    // both set to true to cater for the first term
    bool flag_new_value = true;
    bool flag_preceding_operator = true;

    while (it != expression.end()){
        if (charIsDigit(*it) || charIsAlpha(*it)){
            // if start of new value, start new term
            // else continue current term
            if (flag_new_value){
                current_term = "";
                current_term += *it;
            }
            else {
                current_term += *it;
            }

            flag_new_value = false;
            flag_preceding_operator = false;
        }
        else if (charIsParenthesis(*it) || charIsOperator(*it)){
            // finish off value
            if (!flag_new_value){
                terms.push_back(current_term);
            }

            // start new term
            current_term = "";
            current_term += *it;
            flag_new_value = true;

            // (preceding operator)(minus) signifies start of negative number,
            // otherwise it is a standalone operator
            if (*it == '-' && flag_preceding_operator) { flag_new_value = false; }
            else { terms.push_back(current_term); }

            // if this is an operator, set the prev operator flag
            if (charIsOperator(*it)) { flag_preceding_operator = true; }
            else { flag_preceding_operator = false; }
        } else if (charIsWhitespace(*it)){
            // do nothing, char is still recognized
        } else {
            // unrecognized character,
            // finish off value
            if (!flag_new_value){
                terms.push_back(current_term);
            }

            // add character as separate term
            // will be highlighed during checkExpression Array
            current_term = "";
            current_term += *it;
            terms.push_back(current_term);
            flag_new_value = true;
            flag_preceding_operator = false;

            // log error
            ostringstream buffer;
            buffer << "Problem | Input | Illegal character in expression: " << current_term << " | ASCII Value: " << (int)*it << "\n";
            m_ErrorMessage += buffer.str();
        }
        it++;
    }
    // if the last term is a number, push it
    if (!flag_new_value){
        terms.push_back(current_term);
    }
    m_TermCount = terms.size();
    return terms;
}


//	Checking and Error Handling
//	---------------------------

bool Expression::check_DecimalPointOK(string term){
   int decimal_points = 0;
   bool flag_isVariable = false;

   for(string::iterator it = term.begin(); it != term.end(); it++) {
       flag_isVariable = flag_isVariable || charIsAlpha(*it);
       if (*it == '.' || *it == ',') { decimal_points++; }

       // multiple decimal points
       if (decimal_points > 1) { return false; }
       // decimal points in variable names
       else if (decimal_points > 0 && flag_isVariable) { return false;}
   }
   return true; // if no problem
}

bool Expression::check_NumbersOK(string term){
    if (termIsNumeric(term)){
        for (string::iterator it = term.begin(); it != term.end(); it++){
            if (!charIsDigit(*it)){
                m_ErrorMessage += "Problem | Input | Variables may not start with a number.\n";
                return false; // numbers must only be made out of digits
            }
        }
    }
    return true;
}

bool Expression::check_CharsOK(string term){
    for (string::iterator it = term.begin(); it != term.end(); it++){
        if (!charIsValid(*it))
        {
            return false;	// illegal character
        }
    }
    return true; // if no problem
}

bool Expression::check_OperatorsOK(string term, int term_pos, int last_term_pos, bool &flag_prev_operator)
{
    // term is an operator
    if (charIsOperator(term[0]) && (term.size() == 1)){
        // consecutive operators
        if (flag_prev_operator)
            return false;

        // log operator found
        flag_prev_operator = true;

        // operator at beginning of expression (non negative) or
        // operator at the end of the expression
        if (	(term_pos == 0 && term[0] != '-') ||
                (term_pos == last_term_pos - 1))
            return false;

    } else { flag_prev_operator = false; } // log operator not found

   return true; // no problem
}

vector<int> Expression::checkExpressionArray(vector<string> &expression)
{
    vector<int> error_terms, error_parenth;
    vector<string> terms_before_parenth;
    vector<int> open_parenth_positions;
    string term;
    int term_pos = 0;
    bool flag_preceding_operator = false;
    bool flag_checks_passed;

    flag_Valid = true;

    for (vector<string>::iterator it = expression.begin(); it != expression.end(); it++){
       term = *it;
       // input checks
       flag_checks_passed = 	check_CharsOK(term)		&&
                            check_NumbersOK(term)		&&
                            check_OperatorsOK(term, term_pos, static_cast<int>(expression.size()), flag_preceding_operator)	&&
                            check_DecimalPointOK(term);

       if(!flag_checks_passed) {
           error_terms.push_back(term_pos); // incorrect input in current term
           flag_Valid = false;
       }
       // parentheses checks
       if (term[0] == '('){
           error_parenth.push_back(term_pos);
           open_parenth_positions.push_back(term_pos);

           if (term_pos > 0)	// keep track of term before parenth - in case of function
               terms_before_parenth.push_back(expression[term_pos-1]);
       }
       else if (term[0] == ')'){
           // unopened parentheses
           if (error_parenth.size() == 0){
               flag_Valid = false;
               error_terms.push_back(term_pos);
               m_ErrorMessage += "Problem | Input | Unopened parenthesis.\n";
           }
           else {
               error_parenth.pop_back();
           }

           // check if contents of parenth match what is needed by function/standard value
           if (!terms_before_parenth.empty() && termIsFunction(terms_before_parenth.back())	// functions need non-empty parenth
                   && (term_pos - open_parenth_positions.back()) == 1){
               flag_Valid = false;
               error_terms.push_back(open_parenth_positions.back()-1);
               m_ErrorMessage += "Problem | Input | Empty parentheses - function requires a value in parentheses.\n";
           } else if (!terms_before_parenth.empty() && termIsStandardValue(terms_before_parenth.back())
                      && (term_pos - open_parenth_positions.back()) > 1){ 	// value with non-empty parenth
               flag_Valid = false;
               error_terms.push_back(open_parenth_positions.back()-1);
               m_ErrorMessage += "Problem | Input | Non-empty parentheses - value requires parentheses to be empty.\n";
           }
           if(!terms_before_parenth.empty()){
                terms_before_parenth.pop_back();
                open_parenth_positions.pop_back();
           }
       }
       term_pos++;
    }
    // unclosed parentheses
    for (vector<int>::iterator it = error_parenth.begin(); it != error_parenth.end(); it++){
        error_terms.push_back(*it);
        m_ErrorMessage += "Problem | Input | Unclosed parenthesis.\n";
        flag_Valid = false;
    }

    // return
    return error_terms;
}

bool Expression::variableNameIsUnique(Variable &variable){
    for (vector<Variable>::iterator it = m_Variables.begin(); it != m_Variables.end(); it++){
            if ((*it).name() == variable.name()){ return false; }
    }
    return true;
}

bool Expression::variableNameIsValid(Variable & myVar){
    string sName = myVar.name();
    if (!Variable::nameIsLegal(sName))
        m_ErrorMessage += "Problem | Input | Variable name is illegal." + sName + "\n";
    if (!variableNameIsUnique(myVar))
        m_ErrorMessage += "Problem | Input | Variable name is not unique: " + sName + "\n";
    return Variable::nameIsLegal(sName) && variableNameIsUnique(myVar);
}

bool Expression::charIsValid(char c){
    return charIsAlpha(c) || charIsDigit(c) || charIsOperator(c) || charIsParenthesis(c) || charIsWhitespace(c);
}

bool Expression::termIsNumeric(string term){ return charIsDigit(term[0]); } // reason why variable names may not start with digit

bool Expression::termIsAlpha(string term){ return charIsAlpha(term[0]); }

bool Expression::termIsStandardValue(string term){ return Variable::isStandardValue(term); }

bool Expression::termIsFunction(string term){ return Variable::isFunction(term); }


//	Expression Reduction
//	---------------------

bool Expression::compressExpression(vector<string> &expression)
{
    m_TermCount = expression.size();

    for (int i = 0; i < m_TermCount; i++){
        string sTerm = expression[i];

        // if compression character is found:
        // shift everything after the compression character left by 1 and
        // delete last term
        if (COMPRESSION_CHAR == sTerm[0]){
            for (int j = i; j < m_TermCount-1; j++){
                expression[j] = expression[j+1];
            }
            expression.pop_back();
            return true;
        }
    }
    return false; // if no whitespace is found
}

bool Expression::doParenthesis (vector<string> &expression)
{
    m_TermCount = expression.size();

    int open_pos = UninitializedCounter;
    int range = UninitializedCounter;
    bool flag_empty_parenth;
    string temp_term, term_before_operator, term_after_operator;

    // find parentheses
    for (int i = 0; i < m_TermCount; i++){
        temp_term = expression[i];

        // if opening parentheses found:
        // start counting range,
        // log term before parenth
        if (charIsParenthesis(temp_term[0]) && temp_term[0] == '('){
            open_pos = i;
            if (i > 0)
                term_before_operator = expression[i-1];
        }

        // if closing parentheses found:
        // check contents of parentheses
        else if (charIsParenthesis(temp_term[0]) && temp_term[0] == ')'){

            assert(open_pos != UninitializedCounter); // check for unopened bracket

            // determine range
            range = i - open_pos;

            // log term after parenth
            if (i+1 < m_TermCount)
                term_after_operator = expression[i+1];

            // if parentheses not empty,
            // create new array from contents,
            // perform maths on contents
            // substitute result back into expression
            if (range > 1){
                flag_empty_parenth = false;
                vector<string> sParenthesisArray;
                for (int j = 0; j < range -1; j++){
                    sParenthesisArray.push_back(expression[open_pos+1+j]);
                }
                doBasic(sParenthesisArray);
                expression[open_pos] = sParenthesisArray[0];
            }

            // if parentheses empty,
            // substitute parentheses with 0
            else {
                flag_empty_parenth = true;
                expression[open_pos] = "0";
            }

            // insert compression characters
            for (int j = open_pos+1; j < i+1; j++){
                expression[j] = COMPRESSION_CHAR;
            }

            // parenthesis multiplication
            bool flag_doneBefore = false;
            if (!term_before_operator.empty()){				// there is term before parenth
                if (termIsNumeric(term_before_operator)){ 	// term before parenth is number
                    expression[open_pos+1] = expression[open_pos];	// shift answer up by 1
                    expression[open_pos] = "*"; 					// convert to multiplication
                    flag_doneBefore = true;
                }
            }
            if (!term_after_operator.empty()){				// there is term before parenth
                if (termIsNumeric(term_after_operator)){ 	// term before parenth is number
                    if (flag_empty_parenth){			// if parenth were empty (need to deal with differently)
                        if (flag_doneBefore){
                                expression[open_pos+2] = COMPRESSION_CHAR;
                        } else {
                                expression[open_pos+1] = "*";
                        }
                    } else {
                        expression[open_pos+2] = "*"; 	// convert to multiplication
                    }
                }
            }

            // compress expression
            while(compressExpression(expression)){
                //! loop will auto-terminate
            }

            // do special operations (sin, log, etc)
            doSpecial(expression, open_pos, flag_empty_parenth);
            return true;
        }
    }

    // check for unclosed bracket
    assert((open_pos == UninitializedCounter) && (range == UninitializedCounter));
    return false;	// no parenthesis was found
}

bool Expression::doPowers(vector<string> &expression)
{
    m_TermCount = expression.size();

    // powers must be done right to left (directional)
    for (int i = m_TermCount-1; i > 0; i--){
        string temp_term = expression[i];

        // if operation found and it is the correct operation
        if (charIsOperator(temp_term[0]) && temp_term[0] == '^'){
            //do the math
            string term_before_operator = expression[i-1];
            string term_after_operator  = expression[i+1];
            double value_before = atof(term_before_operator.c_str());
            double value_after = atof(term_after_operator.c_str());

            if ((value_before < 0) && (fmod(value_before, 1) != 0) && (fmod(value_after, 1) != 0))
                throw MathComplex;
            if ((value_before == 0) && (value_after == 0))
                throw MathNaN;
            if ((value_before == 0) && (value_after < 0))
                throw MathPole;

            double result =  pow(atof(term_before_operator.c_str()), atof(term_after_operator.c_str()));

            // update expression:
            // operator and second value filled with compression character
            ostringstream buffer;
            buffer << result;
            expression[i-1] = buffer.str();
            expression[i]   = COMPRESSION_CHAR;
            expression[i+1] = COMPRESSION_CHAR;

            // compress expression
            while(compressExpression(expression)){
                //! loop will auto-terminate
            }

            return true; // if an operation was done:
        }
    }
    return false;	// if no operation found
}

bool Expression::doDivision (vector<string>	&expression)
{
    m_TermCount = expression.size();

    for (int i = 0; i < m_TermCount; i++){
        string temp_term = expression[i];

        // if operation found and it is the correct operation
        if (charIsOperator(temp_term[0]) && temp_term[0] == '/'){

           //do the math
           string term_before_operator = expression[i-1];
           string term_after_operator  = expression[i+1];

           // [ERROR] Expression | doDivision() | no consecutive operators allowed
           assert( !((term_after_operator.size() == 1) &&
                   charIsOperator(term_after_operator[0])) );
           if (atof(term_after_operator.c_str()) == 0)
               throw MathDivideByZero;
           double result = atof(term_before_operator.c_str()) / atof(term_after_operator.c_str());

           // update expression - operator and second value filled with special character
           ostringstream buffer;
           buffer << result;
           expression[i-1] = buffer.str();
           expression[i]   = COMPRESSION_CHAR;
           expression[i+1] = COMPRESSION_CHAR;

           // compress expression
           while(compressExpression(expression)){
               //! loop will auto-terminate
           }
           return true; // if an operation was done:
        }
    }
    return false; // if no operation found
}

bool Expression::doMultiplication (vector<string> &expression)
{
    m_TermCount = expression.size();

    for (int i = 0; i < m_TermCount; i++){
        string temp_term = expression[i];

        // if operation found and it is the correct operation
        if (charIsOperator(temp_term[0]) && temp_term[0] == '*'){

            //do the math
            string term_before_operator = expression[i-1];
            string term_after_operator  = expression[i+1];
            double result = atof(term_before_operator.c_str()) * atof(term_after_operator.c_str());

            // update expression:
            // operator and second value filled with special character
            ostringstream buffer;
            buffer << result;
            expression[i-1] = buffer.str();
            expression[i]   = COMPRESSION_CHAR;
            expression[i+1] = COMPRESSION_CHAR;

            // compress expression
            while(compressExpression(expression)){
                //! loop will auto-terminate
            }

            // if an operation was done:
            return true;
        }
    }
    return false;	// if no operation found
}


bool Expression::doSubtraction (vector<string> &expression)
{
    m_TermCount = expression.size();

    for (int i = 0; i < m_TermCount; i++){
        string temp_term = expression[i];

        // if operation found and it is the correct operation
        if (charIsOperator(temp_term[0]) && temp_term[0] == '-' && !charIsDigit(temp_term[1])){
            double result;

            // if '-' is at the beginning of expression , it signifies a negative number
            if (i == 0){
                // multiply the term after the minus by -1
                string term_after_operator  = expression[i+1];
                result = -1*atof(term_after_operator.c_str());

                // update expression - operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                expression[i] = buffer.str();
                expression[i+1] = COMPRESSION_CHAR;
            } else {
                // do the math
                string term_before_operator = expression[i-1];
                string term_after_operator  = expression[i+1];
                result = atof(term_before_operator.c_str()) - atof(term_after_operator.c_str());

                // update expression:
                // operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                expression[i-1] = buffer.str();
                expression[i]   = COMPRESSION_CHAR;
                expression[i+1] = COMPRESSION_CHAR;
            }

            // compress expression
            while(compressExpression(expression)){
                //! loop will auto-terminate
            }
            return true;	// if an operation was done:
        }
    }
    return false;	// if no operation found
}

bool Expression::doAddition (vector<string> &expression)
{
    m_TermCount = expression.size();

    for (int i = 0; i < m_TermCount; i++){
        string temp_term = expression[i];

        // if operation found and it is the correct operation
        if (charIsOperator(temp_term[0]) && temp_term[0] == '+'){
                //do the math
                string term_before_operator = expression[i-1];
                string term_after_operator  = expression[i+1];
                double result = atof(term_before_operator.c_str()) + atof(term_after_operator.c_str());

                // update expression:
                // operator and second value filled with special character
                ostringstream buffer;
                buffer << result;
                expression[i-1] = buffer.str();
                expression[i]   = COMPRESSION_CHAR;
                expression[i+1] = COMPRESSION_CHAR;

                // compress expression
                while(compressExpression(expression)){
                    //! loop will auto-terminate
                }

                return true;	// if an operation was done
        }
    }
    return false;	// if no operation found
}

void Expression::doBasic(vector<string> &expression)
{
    //! loops will auto-terminate
    while (doPowers(expression)) {}
    while (doDivision(expression)) {}
    while (doMultiplication(expression)) {}
    while (doSubtraction(expression)) {}
    while (doAddition(expression)) {}
}

void Expression::doSpecial(vector<string> &expression, int eval_pos, bool flag_empty_parenth)
{
    // if the brackets were right at the beginning, nothing before them
    // so no special function to be done
    if (eval_pos < 1) { return; }

    string term_before_operator = expression[eval_pos-1];
    string eval_term = expression[eval_pos];
    m_ProblemTerm = eval_pos;
    double result;

    //trig functions
    if (term_before_operator == "sin"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = sin(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "cos"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = cos(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "tan"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = tan(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "sinh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = sinh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "cosh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = cosh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "tanh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = tanh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arcsin"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = asin(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arccos"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = acos(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arctan"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = atan(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arcsinh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = asinh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arccosh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = acosh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "arctanh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = atanh(atof(eval_term.c_str()));
    }
    //	negative
    else if (term_before_operator == "-sin"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -sin(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-cos"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -cos(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-tan"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -tan(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-sinh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -sinh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-cosh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -cosh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-tanh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -tanh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arcsin"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -asin(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arccos"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -acos(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arctan"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -atan(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arcsinh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -asinh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arccosh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -acosh(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-arctanh"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -atanh(atof(eval_term.c_str()));
    }

    //	exponential
    else if(term_before_operator == "exp"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = exp(atof(eval_term.c_str()));
    }
    else if(term_before_operator == "-exp"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        result = -exp(atof(eval_term.c_str()));
    }

    // logarithmic functions
    else if(term_before_operator == "log"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        double dEval = atof(eval_term.c_str());
        if (dEval == 0)
            throw MathPole;
        result = log10(dEval);
    }
    else if(term_before_operator == "ln"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        double dEval = atof(eval_term.c_str());
        if (dEval == 0)
            throw MathPole;
        result = log(dEval);
    }
    else if(term_before_operator == "-log"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        double dEval = atof(eval_term.c_str());
        if (dEval == 0)
            throw MathPole;
        result = -log10(dEval);
    }
    else if(term_before_operator == "-ln"){
        if (flag_empty_parenth)
            throw InputErrorParenthesesEmpty;
        double dEval = atof(eval_term.c_str());
        if (dEval == 0)
            throw MathPole;
        result = -log(dEval);
    }

    // values
    else if(term_before_operator == "pi"){
        if (!flag_empty_parenth){
            throw InputErrorParenthesesNotEmpty;
        }
        result = PI;
    }
    else if(term_before_operator == "-pi"){
        if (!flag_empty_parenth)
            throw InputErrorParenthesesNotEmpty;
        result = -1*PI;
    }

    // no matching function
    else { return; }

    // return evaluated value:
    // can only get here if 'break' out of switch
    ostringstream buffer;
    buffer << result;

    if ((buffer.str() == "nan") || (buffer.str() == "-nan"))
        throw MathNaN;

    expression[eval_pos-1] = buffer.str();
    expression[eval_pos] = COMPRESSION_CHAR;

    // compress expression
    while(compressExpression(expression)){
        //! loop will auto-terminate
    }
}


//	Recursive Evaluation
//	---------------------

void Expression::recEval()
{
    /*!	This function iterates through all combinations of variables
        for each combination, an evaluation is performed.*/

    if (!flag_Valid)
        throw InputErrorInvalidExpression;

    // for all values of the current variable (current variable is global)
    for (int i = 0; i < m_Variables[m_CurrentVariable].elements(); i++){
            int j = m_CurrentVariable;

            // if this is the final nested variable;
            // substitute current values and evaluate... (see else)
            if (m_CurrentVariable == static_cast<int>(m_Variables.size()) - 1){
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
                        m_Results_Problems.push_back(m_Results.size());
                        dResult = 0;
                    }
                    catch(INPUT_ERROR_CODES e){
                        // INPUT ERRORS HANDLED HERE
                        // stops the evaluation
                        m_Expression_Problems.push_back(m_ProblemTerm);
                        flag_Valid = false;
                    }

                    m_Results.push_back(dResult);
            }

            // ...otherwise move on to the next nested variable level
            else {
                    m_CurrentVariable++; 	// go to the next variable
                    recEval(); 				// recurse through it
                    m_CurrentVariable--;	// come back to this variable to carry on with it
            }

            m_Variables[j].nextPosition();	// next variable
    }
}

double Expression::evaluateExpression()
{
    /*!	Evaluates an expression. Variables must already added.*/

    if (!flag_Valid)
        throw InputErrorInvalidExpression; // [DOCUMENTATION] invalid expression

    // do parentheses; reduce the expression
    while (doParenthesis(m_WorkingExpression)) {}
    // evaluate reduced expression
    doBasic(m_WorkingExpression);

    string reduced_expression = m_WorkingExpression[0];
    double result = atof(reduced_expression.c_str());
    resetExpression();
    return result;
}

void Expression::resetExpression(){ m_WorkingExpression = m_OriginalExpression; }


//	Output
//	-------

string Expression::getStringArray(vector<string> string_array)
{
    m_TermCount = string_array.size();
    string result = "";

    for (unsigned int i = 0; i < string_array.size(); i++){
        result += string_array[i];
    }
    return result;
}


// 	Exception Handling and Validation
//	----------------------------------

void Expression::handleMathException(MATH_ERROR_CODES e)
{

    switch(e){
    case MathDivideByZero:
        if (!flag_DivByZero)
            m_ErrorMessage += "Warning | Evaluation | Found division(s) by zero.\n";
        flag_DivByZero = true;
        break;
    case MathNaN:
        if (!flag_Nan)
            m_ErrorMessage += "Warning | Evaluation | Found occurrence(s) of NaN (not a number). Caused by invalid mathematical operation.\n";
        flag_Nan = true;
        break;
    case MathPole:
        if (!flag_Pole)
            m_ErrorMessage += "Warning | Evaluation | Found pole(s).\n";
        flag_Pole = true;
        break;
    case MathComplex:
        if (!flag_Complex)
            m_ErrorMessage += "Warning | Evaluation | Found complex number result(s).\n";
        flag_Complex = true;
        break;
    default:
        m_ErrorMessage += "Bug | Evaluation | Unhandled math exception. | Please report this bug to the following email address:\n emerick.bosch+bugmail@gmail.com\n";
        break;
    }
}

void Expression::resetEvaluation(){
    m_CurrentVariable = 0;
    m_Results.clear();
    flag_Nan = false;
    flag_Pole = false;
    flag_DivByZero = false;
    flag_Complex = false;
}

