#ifndef INEQUALITYINPUT_H
#define INEQUALITYINPUT_H

#include <QWidget>
#include <QtWidgets>
#include "../deplot/include/inequality.h"
#include "../deplot/include/expression.h"

namespace Ui {
class InequalityInput;
}

class InequalityInput : public QWidget
{
    Q_OBJECT

public:
    explicit InequalityInput(QWidget *parent = 0);
    ~InequalityInput();

    //	setters
    void setNumber(int);
    void setXYVariables(Variable, Variable);
    void enablePositionButtons(bool);
    //	getters
    int getNumber();
    int getColorIndex();
    int getShapeIndex();
    int getCombination();
    QVector<double> getX();
    QVector<double> getY();
    QVector<double> getXProblem();
    QVector<double> getYProblem();
    //	validation
    bool highlightInvalidExpressionTerms();
    // 	formatting
    void clearFormatting();
    //	core
    bool createInequality();
    bool addVariable(Variable);
    bool evaluate();

signals:
    void moveUp(int nNumber);
    void moveDown (int nNumber);
    void killThis (int nNumber);

private slots:
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_pushButtonDelete_clicked();
    void on_lineEditLeft_textChanged(const QString&);
    void on_lineEditRight_textChanged(const QString&);

private:
    Ui::InequalityInput *ui;
    Inequality mInequality;
    Variable mVariableX, mVariableY;
    int nInequalityInputNumber;
    vector<bool> vPlotSpace;
    vector<int> vProblemSpace;
    QVector<double> qvX, qvY, qvX_problem, qvY_problem;
};

#endif // INEQUALITYINPUT_H
