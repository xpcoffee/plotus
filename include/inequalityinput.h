#ifndef INEQUALITYINPUT_H
#define INEQUALITYINPUT_H

//	Includes
#include <QWidget>
#include <QtWidgets>
#include <sstream>
#include <qvalidator.h>
#include "inequality.h"
#include "expression.h"

// 	Enumerators
enum COMBINE
{
    CombinationNone 		= 0,
    CombinationIntersect 	= 1,
    CombinationUnion 		= 2,
    CombinationSubtract 	= 3,

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
    void setXYVariables(Variable, Variable);
    void setX(QVector<double>);
    void setY(QVector<double>);
    void enablePositionButtons(bool);
    void enableCombinations(bool);
    void resetCombinations();
    // 	parsers
    string expressionToJSON();
    string dataToJSON();
    void fromJSON(string);
    //	getters
    int getNumber();
    int getColorIndex();
    int getShapeIndex();
    int getCombination();
    double getPrecision();
    bool getSkip();
    string getLeftExpression();
    string getRightExpression();
    string getErrors();
    QVector<double> getX();
    QVector<double> getY();
    QVector<double> getXProblem();
    QVector<double> getYProblem();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();
    //	validation
    bool highlightInvalidExpressionTerms();
    // 	formatting
    void clearFormatting();
    void clearFields();
    //	core
    bool createInequality();
    bool addVariable(Variable variable);
    bool evaluate();

signals:
    void moveUp(int nNumber);
    void moveDown (int nNumber);
    void killThis (int nNumber);

public slots:
    void splitterResize(QList<int> sizes);

private slots:
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_pushButtonDelete_clicked();
    void on_lineEditLeft_textChanged(const QString&);
    void on_lineEditRight_textChanged(const QString&);
    void on_comboBoxInteract_currentIndexChanged(int index);
    void on_checkBoxSkip_toggled(bool checked);
    void on_comboBoxInequality_currentIndexChanged(int index);

private:
    Ui::InequalityInput *ui;
    // data
    Inequality m_Inequality;
    Variable m_VariableX, m_VariableY;
    // meta
    int m_guiNumber;
    int m_precisionIndex;
    bool flag_skip;
    // result vectors
    vector<bool> m_plotSpace;
    vector<int> m_problemSpace;
    QVector<double> m_X, m_Y, m_XProblem, m_YProblem;
    // error handling
    string m_error_message;
    string m_bugmail;
};

#endif // INEQUALITYINPUT_H
