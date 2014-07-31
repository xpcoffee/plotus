/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    inequalityinput.h
    ------------------

    Description
    ============
    Provides the GUI for:
    - input into inequality
    - error feedback from inequality
    - plot settings

    Subwidget of BareMinimumPlotter.
*/

#ifndef INEQUALITYINPUT_H
#define INEQUALITYINPUT_H


///	Includes
///	=========

#include <QWidget>
#include <QtWidgets>
#include <sstream>
#include <qvalidator.h>
#include <qwt_symbol.h>
#include "inequality.h"
#include "expression.h"


///	Enumerated Types
///	=================

enum CombinationMode
{
    CombinationNone 		= 0,
    CombinationIntersect,
    CombinationUnion,
    CombinationSubtract,

};

enum ComboboxColor {
    Blue 		= 0,
    Green,
    Red,
    DarkBlue,
    DarkGreen,
    DarkRed,
    Grey,
    Black,
    White,
};

enum ComboboxShape {
    CrossX 		= 0,
    CrossPlus,
    Circle,
    UpTriangle,
    DownTriangle,
    Square,
    Diamond
};


///	Namespaces
/// ===========

namespace Ui {
class InequalityInput;
}


///	Class
///	======

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
    string problemDataToJSON();
    void fromJSON(string);

    //	getters: ui
    int getNumber();
    int getShapeIndex();
    int getCombination();
    double getPrecision();
    bool getSkip();
    string getLeftExpression();
    string getRightExpression();
    string getErrors();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();
    QColor getColor();
    InequalitySymbol getSymbol();
    QwtSymbol::Style getShape();

    //	getters: data
    QVector<double> getX();
    QVector<double> getY();
    QVector<double> getXProblem();
    QVector<double> getYProblem();

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
    void moveUp(int gui_number);
    void moveDown (int gui_number);
    void killThis (int gui_number);

public slots:
    void splitterResize(QList<int> sizes);

private slots:
    void on_pushButton_Up_clicked();
    void on_pushButton_Down_clicked();
    void on_pushButton_Delete_clicked();
    void on_lineEdit_Left_textChanged(const QString&);
    void on_lineEdit_Right_textChanged(const QString&);
    void on_comboBox_Interact_currentIndexChanged(int index);
    void on_checkBox_Skip_toggled(bool checked);
    void on_comboBox_Inequality_currentIndexChanged(int index);

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
