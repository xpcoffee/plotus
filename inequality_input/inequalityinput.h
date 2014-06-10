#ifndef INEQUALITYINPUT_H
#define INEQUALITYINPUT_H

//	Includes
#include <QWidget>
#include <QtWidgets>
#include "../deplot/include/inequality.h"
#include "../deplot/include/expression.h"

// 	Enumerators
enum COMBINE
{
    COMBINE_NONE 			= 0,
    COMBINE_INTERSECTION 	= 1,
    COMBINE_UNION 			= 2,
    COMBINE_SUBTRACTION 	= 3,

};

// 	Namespace
namespace Ui {
class InequalityInput;
}

//	Class
class InequalityInput : public QWidget
{
    Q_OBJECT

public:
    explicit InequalityInput(QWidget *parent = 0);
    ~InequalityInput();

    //	setters
    void setNumber(int);
    void setIndex(int);
    void setXYVariables(Variable, Variable);
    void enablePositionButtons(bool);
    void enableCombinations(bool);
    void resetCombinations();
    //	getters
    int getNumber();
    int getColorIndex();
    int getShapeIndex();
    int getCombination();
    bool getSkip();
    string getLeftExpression();
    string getRightExpression();
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
    void on_comboBoxInteract_currentIndexChanged(int index);
    void on_checkBoxSkip_toggled(bool checked);

private:
    Ui::InequalityInput *ui;
    Inequality mInequality;
    Variable mVariableX, mVariableY;
    int nInequalityInputNumber;
    int nIndex;
    bool flag_skip;
    vector<bool> vPlotSpace;
    vector<int> vProblemSpace;
    QVector<double> qvX, qvY, qvX_problem, qvY_problem;
};

#endif // INEQUALITYINPUT_H
