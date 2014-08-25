#ifndef PLOTWORKER_H
#define PLOTWORKER_H

///	Includes
///	=========

#include <QObject>
#include <vector>
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
    Variable m_xVariable, m_yVariable;
    QVector<double> m_xResults, m_yResults;
    QVector<double> m_xResults_problem, m_yResults_problem;
    QVector<double> m_xResults_combination, m_yResults_combination;
    PlottingVector m_samples, m_samples_problem;

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
    void vectorCombineNone(input_type *inequality);

    template <typename input_type>
    void vectorCombineIntersection(input_type *inequality);

    template <typename input_type>
    void vectorCombineUnion(input_type *inequality);

    template <typename input_type>
    void vectorCombineSubtraction(input_type *inequality);

    void createQwtSamples();

    //	notification
    void printError();

};

#endif // PLOTWORKER_H
