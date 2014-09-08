#include "include/inequality.h"


Inequality::Inequality(string expression1, InequalitySymbol symbol, string expression2) :
 flag_Initialized(true),
 m_Precision(0),
 m_ErrorMessage("")
{
    if (symbol == NoSymbol) { flag_Initialized = false; }
    else { setInequality(expression1, symbol, expression2); }

    cancelFlagLeft = new bool(false);
    cancelFlagRight = new bool(false);

}

//	Setters
//	========

void Inequality::clearVariables()
{
    m_LeftExpression.clearVariables();
    m_RightExpression.clearVariables();
}

void Inequality::addVariable(Variable variable)
{
    assert(flag_Initialized);
    m_LeftExpression.addVariable(variable);
    m_RightExpression.addVariable(variable);
}

void Inequality::setInequality(string left_expression, int symbol, string right_expression)
{
        m_LeftExpression.setExpression(left_expression);
        m_RightExpression.setExpression(right_expression);
        m_Symbol = symbol;
        flag_Initialized = true;
}

void Inequality::setInequality(string left_expression, InequalitySymbol symbol, string right_expression)
{
        m_LeftExpression.setExpression(left_expression);
        m_RightExpression.setExpression(right_expression);

        m_Sym = symbol;
        flag_Initialized = true;
}

void Inequality::changeSymbol(int symbol)
{
    assert(flag_Initialized);
    m_Symbol = symbol;
}

void Inequality::changeSymbol(InequalitySymbol symbol)
{
    assert(flag_Initialized);
    m_Sym = symbol;
}

void Inequality::setPrecision(double value){ m_Precision = value; }

//	Getters
//	--------

vector<int> Inequality::getProblemElements_ExpressionLHS(){ return m_LeftExpression.getProblemElements_Expression(); }

vector<int> Inequality::getProblemElements_ExpressionRHS(){ return m_RightExpression.getProblemElements_Expression(); }

string Inequality::getTermLHS(int nTerm){ return m_LeftExpression.getTerm(nTerm); }

string Inequality::getTermRHS(int nTerm){ return m_RightExpression.getTerm(nTerm); }

string Inequality::getExpressionLHS(){ return m_LeftExpression.getExpression(); }

string Inequality::getExpressionRHS(){ return m_RightExpression.getExpression(); }

int Inequality::getNumTermsLHS(){ return m_LeftExpression.getNumTerms(); }

int Inequality::getNumTermsRHS(){ return m_RightExpression.getNumTerms(); }

bool Inequality::getXBeforeY(Variable mXVar, Variable mYVar) { return m_LeftExpression.isXBeforeY(mXVar, mYVar); }


//	Evaluation
//	-----------

vector<bool> Inequality::evaluate()
{
    vector<bool> plot_points;

    cancelFlagLeft = &m_LeftExpression.flag_Cancel;
    cancelFlagRight = &m_RightExpression.flag_Cancel;

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
            assert (false && "Unknown inequality operator");
            break;
        }
    }
    return plot_points;
}

vector<int> Inequality::getProblemElements_ResultsCombined()
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

bool Inequality::isValidLHS(){ return m_LeftExpression.isValid(); }

bool Inequality::isValidRHS(){ return m_RightExpression.isValid(); }

bool Inequality::variablesInit()
{
    bool flag_init = true;

    try {
        m_LeftExpression.subVariableValues();
    } catch (InputErrorCode e){
        switch(e){
            case InputErrorUninitializedVariable:
                flag_init = false;
            break;
        default:
            break;
        }
    }

    try {
        m_RightExpression.subVariableValues();
    } catch (InputErrorCode e){
        switch(e){
            case InputErrorUninitializedVariable:
                flag_init = false;
            break;
        default:
            break;
        }
    }

    return flag_init;

}

bool Inequality::variableIsValid (Variable & myVar){ return m_LeftExpression.variableNameIsValid(myVar); }

string Inequality::getErrors()
{
    m_ErrorMessage += m_LeftExpression.getErrors();
    m_ErrorMessage += m_RightExpression.getErrors();
    return m_ErrorMessage;
}
