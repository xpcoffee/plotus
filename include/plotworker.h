#ifndef PLOTWORKER_H
#define PLOTWORKER_H

///	Includes
///	=========

#include <QObject>
#include <vector>
#include <algorithm>
#include "variableinput.h"
#include "inequalityinput.h"
#include "inequalityloader.h"


///	TypeDefs
///	=========

typedef vector<VariableInput*> VarInputArray;
typedef vector<InequalityInput*> IneqInputArray;
typedef vector<InequalityLoader*> IneqLoaderArray;
typedef QVector<QPointF> PlottingVector;
typedef QwtSymbol::Style PlotStyle;


///	Class
///	======

class PlotWorker : public QObject
{
    Q_OBJECT
public:
    //! variables
    bool flag_cancel;
    bool *cancelFlagA;
    bool *cancelFlagB;

    //! functions
    explicit PlotWorker(QObject *parent = 0);

    void doWork();

signals:
    void dataRequest();
    void progressUpdate(int value, QString message);
    void newGraph(PlottingVector qwt_samples, PlotStyle shape, QColor color, QString tag);
    void newErrorGraph(PlottingVector qwt_problem_samples);
    void logMessage(QString message);
    void memberChanges(VarInputArray var_inputs,
                    IneqInputArray ineq_inputs,
                    IneqLoaderArray ineq_loaders);
    void workFinished();

public slots:
    void createWorker();
    void workerInit(VarInputArray var_inputs,
                    IneqInputArray ineq_inputs,
                    IneqLoaderArray ineq_loaders,
                    Variable x_variable,
                    Variable y_variable,
                    double comparison_precision);

private:
    //! member variables
    //	plotter elements
    VarInputArray m_variableInputs;
    IneqInputArray m_inequalityInputs;
    IneqLoaderArray m_inequalityLoaders;

    //	plotting
    int m_prevCombination;
    int m_lastMatch;

    Variable m_xVariable, m_yVariable;
    PlottingVector m_results;
    PlottingVector m_resultsProblem;
    PlottingVector m_resultsCombined;
    PlottingVector *m_findRange;


//    PlottingVector m_samples, m_samples_problem;

    //	gui management
    int m_inequalityCount;

    //	error handling
//    bool flag_empty;
    QString m_errorMessage;

    // 	settings
    double m_compPrec;

    //! member functions
    //	evaluation and plotting
    void plotNew(int gui_number);
    void plotOld(int gui_number);

    template <typename input_type>
    void combineResults(input_type *inequality);

    template <typename input_type>
    void combinationNone(input_type *inequality);

    template <typename input_type>
    void combinationIntersection(input_type *inequality);

    template <typename input_type>
    void combinationUnion(input_type *inequality);

    template <typename input_type>
    void combinationSubtraction(input_type *inequality);

    PlottingVector createPlottingVector(QVector<double> x_values, QVector<double> y_values);
    QVector<double> splitPlottingVectorX(PlottingVector points);
    QVector<double> splitPlottingVectorY(PlottingVector points);

    //	notification
    void printError();

};

#endif // PLOTWORKER_H
