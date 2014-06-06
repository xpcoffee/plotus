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

    // core
    void plot();
    // validation
    void checkFields();
    void clearFormatting();
    bool highlightInvalidExpressionTerms(Inequality, QLineEdit*, QLineEdit*);
    bool charsValid(QLineEdit*);
    bool isEmpty_InputFields();
    // gui
    void addVariableInput();
    void addInequalityInput();

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
    vector<VariableInput*> vVariableInputs;
    vector<InequalityInput*> vInequalityInputs;
    Variable mVariableX, mVariableY;
    Inequality mInequality;
    int nLatestVariableInput;
    int nLatestInequalityInput;
    QVector<double> qvX, qvY, qvX_problem, qvY_problem;

};

#endif // BAREMINIMUMPLOTTER_H
