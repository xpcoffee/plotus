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

public slots:
    void checkAxisMode(int);
    void removeVariableInput(int);

private slots:
    void on_buttonPlot_clicked();
    void on_lineEditInequalityLeft_textChanged(const QString &arg1);
    void on_lineEditInequalityRight_textChanged(const QString &arg1);
    void on_pushButtonAddVariable_clicked();

private:
    Ui::BareMinimumPlotter *ui;
    vector<VariableInput*> vVariableInputs;
    Variable mVariableX, mVariableY;
    Expression mExpression1, mExpression2;
    Inequality mInequality;
    int nLatestVariableInput;

};

#endif // BAREMINIMUMPLOTTER_H
