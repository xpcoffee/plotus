#include "include/plotworker.h"


///	Public Functions
///	=================

PlotWorker::PlotWorker(QObject *parent) :
    QObject(parent),
    flag_cancel(false),
    m_prevCombination(CombinationNone)
{
}

void PlotWorker::doWork()
{
    for (int i = 0; i < m_inequalityCount + 1; i++){
        bool flag_skip;
        for (int j = 0; j < static_cast<int>(m_inequalityInputs.size()); j++){
        if (flag_cancel)
            break;

        if ( m_inequalityInputs[j]->getNumber() == i ){
            if ( m_inequalityInputs[j]->getSkip() ) {
                flag_skip = true;
                break;
            }
            plotNew(j);
        }
    }

    for (int j = 0; j < static_cast<int>(m_inequalityLoaders.size()); j++){
        if (flag_cancel)
            break;

        if ( m_inequalityLoaders[j]->getNumber() == i ){
            if ( m_inequalityLoaders[j]->getSkip() ) {
                flag_skip = true;
                break;
            }
            plotOld(j);
        }
    }
        if (flag_cancel)
            break;
        if (flag_skip)
        continue;
    }

    if (flag_cancel)
        emit progressUpdate(100, "Cancelled.");
    flag_cancel = true;

    emit PlotWorker::memberChanges(m_variableInputs, m_inequalityInputs, m_inequalityLoaders);
    emit PlotWorker::workFinished();
}



///	Public Slots
///	=============

void PlotWorker::createWorker() { emit dataRequest(); }

void PlotWorker::workerInit(VarInputArray var_inputs,
                            IneqInputArray ineq_inputs,
                            IneqLoaderArray ineq_loaders,
                            Variable x_variable,
                            Variable y_variable,
                            double comparison_precision)
{
    m_variableInputs = var_inputs;
    m_inequalityInputs = ineq_inputs;
    m_inequalityLoaders = ineq_loaders;

    m_inequalityCount = m_inequalityInputs.size() + m_inequalityLoaders.size();

    m_xVariable = x_variable;
    m_yVariable = y_variable;

    m_compPrec = comparison_precision;

    doWork();
}


///	Private Functions
///	==================

void PlotWorker::plotNew(int gui_number)
{
    InequalityInput *input = m_inequalityInputs[gui_number];
    QString gui_number_str = QString::number(gui_number);

    emit progressUpdate(0, "Evaluating inequality " + gui_number_str + "...");
    input->setXYVariables(m_xVariable, m_yVariable);
    if(!input->evaluate()){
        printError();
        return;
    }

    if (flag_cancel)
        return;

    emit progressUpdate(40, "Combining results, inequality " + gui_number_str + "...");
    combineResults(input);
    createQwtSamples();

    if (flag_cancel)
        return;

    m_prevCombination = input->getCombination();
    if (m_prevCombination != CombinationNone){	//	do not plot if combination requested
        emit progressUpdate(100, "Done.");
        return;
    }

    if (flag_cancel)
        return;

    emit progressUpdate(80, "Plotting results, inequality " + gui_number_str + "..." );
    emit newGraph(m_samples, input->getShape(), input->getColor());

    if (!m_xResults_problem.isEmpty())
        emit newErrorGraph(m_samples_problem);

    printError();
    emit progressUpdate(100, "Done.");
//    flag_empty = false;
}


void PlotWorker::plotOld(int gui_number)
{
    InequalityLoader* loader = m_inequalityLoaders[gui_number];
    QString gui_number_str = QString::number(gui_number);

    loader->setPlot();

    emit progressUpdate(40, "Combining results, inequality " + gui_number_str + "...");
//    combineResults(loader->getNumber());
    combineResults(loader);
    createQwtSamples();

    // log combination mode
    m_prevCombination = loader->getCombination();
    if (m_prevCombination != CombinationNone){	//	do not plot if combination requested
        emit progressUpdate(100, "Done.");
        return;
    }

    emit progressUpdate(80, "Plotting results, inequality " + gui_number_str + "..." );
    emit newGraph(m_samples, loader->getShape(), loader->getColor());

    if (!m_xResults_problem.isEmpty())
        emit newErrorGraph(m_samples_problem);

    printError();
    emit progressUpdate(100, "Done.");
//    flag_empty = false;
}

template<typename T>
void PlotWorker::combineResults(T *inequality)
{
    switch(m_prevCombination){
    case CombinationNone:
        vectorCombineNone(inequality);
        break;
    case CombinationIntersect:
        vectorCombineIntersection(inequality);
        break;
    case CombinationUnion:
        vectorCombineUnion(inequality);
        break;
    case CombinationSubtract:
        vectorCombineSubtraction(inequality);
        break;
    default:
        break;
    }
}

template<typename T>
void PlotWorker::vectorCombineNone(T *inequality)
{
        m_xResults = inequality->getX();
        m_yResults = inequality->getY();
        m_xResults_problem = inequality->getXProblem();
        m_yResults_problem = inequality->getYProblem();
}

template<typename T>
void PlotWorker::vectorCombineIntersection(T *inequality)
{
    QVector<double> x_resultsOld = m_xResults;
    QVector<double> y_resultsOld = m_yResults;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    int max_size = ( x_resultsOld.size() > x_resultsNew.size() ) ? x_resultsOld.size() : x_resultsNew.size();
    m_xResults.clear();
    m_yResults.clear();
    m_xResults.reserve(max_size);
    m_yResults.reserve(max_size);

    x_resultsNew = inequality->getX();
    y_resultsNew = inequality->getY();
    m_xResults_problem = inequality->getXProblem();
    m_yResults_problem = inequality->getYProblem();

    for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
       for (int j = 0; j < static_cast<int>(x_resultsNew.size()); j++) {
           // ApproxEqualPrecision is in case you are comparing results of different precisions.
           if (Expression::approxEqual(x_resultsOld[i], x_resultsNew[j], m_compPrec) && Expression::approxEqual(y_resultsOld[i], y_resultsNew[j], m_compPrec)) {
               m_xResults.push_back(x_resultsOld[i]);
               m_yResults.push_back(y_resultsOld[i]);
           }
       }
    }

    // save results for plotting
    if (inequality->getCombination() == CombinationNone){
        inequality->setX(m_xResults);
        inequality->setY(m_yResults);
    }
}

template<typename T>
void PlotWorker::vectorCombineUnion(T *inequality)
{
    QVector<double> x_resultsOld = m_xResults;
    QVector<double> y_resultsOld = m_yResults;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    int max_size = ( x_resultsOld.size() > x_resultsNew.size() ) ? x_resultsOld.size() : x_resultsNew.size();
    m_xResults.clear();
    m_yResults.clear();
    m_xResults.reserve(max_size);
    m_yResults.reserve(max_size);

    x_resultsNew = inequality->getX();
    y_resultsNew = inequality->getY();
    m_xResults_problem = inequality->getXProblem();
    m_yResults_problem = inequality->getYProblem();

    for (m_xVariable.resetPosition(); !m_xVariable.isEnd(); m_xVariable.nextPosition()){
        for (m_yVariable.resetPosition(); !m_yVariable.isEnd(); m_yVariable.nextPosition()){
            for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
                if (Expression::approxEqual(m_xVariable.currentValue(), x_resultsOld[i], m_compPrec) &&
                        Expression::approxEqual(m_yVariable.currentValue(), y_resultsOld[i], m_compPrec)){
                    m_xResults.push_back(m_xVariable.currentValue());
                    m_yResults.push_back(m_yVariable.currentValue());
                }
            }
            for (int i = 0; i < static_cast<int>(x_resultsNew.size()); i++){
                if ((m_xVariable.currentValue() == x_resultsNew[i]) && (m_yVariable.currentValue() == y_resultsNew[i])){
                    m_xResults.push_back(m_xVariable.currentValue());
                    m_yResults.push_back(m_yVariable.currentValue());
                }
            }
        }
    }

    if (inequality->getCombination() == CombinationNone){
        inequality->setX(m_xResults);
        inequality->setY(m_yResults);
    }

}

template<typename T>
void PlotWorker::vectorCombineSubtraction(T *inequality)
{
    QVector<double> x_resultsOld = m_xResults;
    QVector<double> y_resultsOld = m_yResults;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    m_xResults.clear();
    m_yResults.clear();

    x_resultsNew = inequality->getX();
    y_resultsNew = inequality->getY();
    m_xResults_problem = inequality->getXProblem();
    m_yResults_problem = inequality->getYProblem();

    bool flag_keep;

    for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
       flag_keep = true;
       for (int j = 0; j < static_cast<int>(x_resultsNew.size()); j++) {
           if ((Expression::approxEqual(x_resultsOld[i], x_resultsNew[j], m_compPrec) && Expression::approxEqual(y_resultsOld[i], y_resultsNew[j], m_compPrec))) {
               flag_keep = false;
           }
       }
       if (flag_keep){
           m_xResults.push_back(x_resultsOld[i]);
           m_yResults.push_back(y_resultsOld[i]);
       }
    }

    if (inequality->getCombination() == CombinationNone){
        inequality->setX(m_xResults);
        inequality->setY(m_yResults);
    }
}


void PlotWorker::createQwtSamples()
{
    m_samples.clear();
    m_samples_problem.clear();

    for (int i = 0; i < m_xResults.size(); i++){
       m_samples << QPointF(m_xResults[i], m_yResults[i]);
    }
    for (int i = 0; i < m_xResults_problem.size(); i++){
       m_samples_problem << QPointF(m_xResults_problem[i], m_yResults_problem[i]);
    }
}


void PlotWorker::printError()
{
    // get all error messages
    // display messages
    for (int i = 0; i < static_cast<int>(m_inequalityInputs.size()); i++){
        m_errorMessage += m_inequalityInputs[i]->getErrors();
    }
    for (int i = 0; i < static_cast<int>(m_inequalityLoaders.size()); i++){
        m_errorMessage += QString::fromStdString(m_inequalityLoaders[i]->getErrors());
    }

    emit logMessage(m_errorMessage);

    // progress bar
    emit progressUpdate(100, "Error.");
//    flag_empty = true;
}
