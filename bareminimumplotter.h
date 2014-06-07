#ifndef BAREMINIMUMPLOTTER_H
#define BAREMINIMUMPLOTTER_H

#include <QMainWindow>
#include <QtWidgets>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include "deplot/include/variable.h"
#include "deplot/include/inequality.h"
#include "deplot/include/expression.h"
#include "variable_input/variableinput.h"
#include "inequality_input/inequalityinput.h"

using namespace std;

namespace Ui {
class BareMinimumPlotter;
}

class BareMinimumPlotter : public QMainWindow
{
    Q_OBJECT

public:
    explicit BareMinimumPlotter(QWidget *parent = 0);
    ~BareMinimumPlotter();

    // 	core
    void plot();
    void vectorCombineNone(int);
    void vectorCombineUnion(int);
    void vectorCombineIntersection(int);
    void vectorCombineSubtraction(int);
    void formatGraph(int);
    void formatErrorGraph();
    // 	validation
    void clearFormatting();
    // 	gui
    void addVariableInput();
    void addInequalityInput();
    void reOrderInequalityInputs();

public slots:
    void checkAxisMode(int);
    void removeVariableInput(int);
    void removeInequalityInput(int);
    void moveInequalityInputUp(int);
    void moveInequalityInputDown(int);

private slots:
    void on_button_Plot_clicked();
    void on_button_AddVariable_clicked();
    void on_button_AddInequality_clicked();

private:
    Ui::BareMinimumPlotter *ui;
    //	plotter elements
    vector<VariableInput*> vVariableInputs;
    vector<InequalityInput*> vInequalityInputs;
    //	plotting
    int nGraphIndex;
    int nPrevCombination;
    bool flag_Combination;
    Variable mVariableX, mVariableY;
    QVector<double> qvX, qvY, qvX_problem, qvY_problem;
    //	gui management
    int nLatestVariableInput;
    int nLatestInequalityInput;

};

#endif // BAREMINIMUMPLOTTER_H
