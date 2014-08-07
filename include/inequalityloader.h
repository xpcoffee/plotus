/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    inequalityloader.h
    -------------------

    Description
    ============
    Provides the GUI for:
    - loading inequality
    - selecting specific plot from loaded inequality
    - plot settings

    Subwidget of BareMinimumPlotter.
*/


#ifndef INEQUALITYLOADER_H
#define INEQUALITYLOADER_H

///	Includes
///	=========

#include <QWidget>
#include <QtWidgets>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "inequalityinput.h"	// CombinationMode, ComboBoxColor, ComboBoxShape
#include "bluejson.h"

///	Namespaces
///	===========

using namespace std;

namespace Ui {
class InequalityLoader;
}

///	Class
/// ======

class InequalityLoader : public QWidget
{
    Q_OBJECT

public:
    explicit InequalityLoader(QWidget *parent = 0);
    ~InequalityLoader();

    //	setters
    void setNumber(int number);
    void setCaseName(string value);
    void loadCase(string filename);
    void setX(QVector<double> vector);
    void setY(QVector<double> vector);

    //	getters: ui
    int getNumber();
    int getShapeIndex();
    int getCombination();
    bool getSkip();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();
    QColor getColor();
    QwtSymbol::Style getShape();

    //	getters: data
    QVector<double> getX();
    QVector<double> getY();
    string getFile();
    string getErrors();

    //	parsers
    void parsePlotData(string json);
    void parseProblem(string problem);
    string formatName(string json);
    string formatVariables(string json);
    string formatInequality(string json);
    string formatCase(string json);
    string formatExpressions(string json);
    string expressionToJSON();
    string dataToJSON();

    //	evaluation
    void setPlot();

    //	gui
    void enablePositionButtons(bool flag_enable);
    void enableCombinations(bool flag_enable);
    void resetCombinations();
    void setComboBoxPlot();

signals:
    void moveUp(int guiNumber);
    void moveDown (int guiNumber);
    void killThis (int guiNumber);

public slots:
    void splitterResize(QList<int> sizes);

private slots:
    void on_pushButton_Details_clicked();
    void on_pushButton_Delete_clicked();
    void on_pushButton_Up_clicked();
    void on_pushButton_Down_clicked();
    void on_checkBox_Skip_toggled(bool checked);
    void on_comboBox_Combination_currentIndexChanged(int index);
    void on_comboBox_Plot_currentIndexChanged(int index);

private:
    Ui::InequalityLoader *ui;

    //	data
    vector<QVector<double> > m_xResults;
    vector<QVector<double> > m_yResults;
    vector<string> m_detailsHTML;
    vector<string> m_detailsJSON;
    string m_detail;

    //	meta
    int m_guiNumber;
    bool flag_skip;
    bool flag_problem;
    string m_name;
    string m_filename;
    string m_errorMessage;

    //	evaluation
    unsigned int m_currentPlot;
};

#endif // INEQUALITYLOADER_H
