#ifndef INEQUALITYLOADER_H
#define INEQUALITYLOADER_H

//	Include
#include <QWidget>
#include <QtWidgets>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "inequalityinput.h"
#include "bluejson.h"

//	Namespace
using namespace std;

namespace Ui {
class InequalityLoader;
}

//	Class
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
    // getters
    int getNumber();
    int getColorIndex();
    int getShapeIndex();
    int getCombination();
    bool getSkip();
    QVector<double> getX();
    QVector<double> getY();
    string getFile();
    string getErrors();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();
    // parsers
    void parsePlotData(string json);
    void parseProblem(string problem);
    string formatVariables(string json);
    string formatInequality(string json);
    string formatCase(string json);
    string formatExpressions(string json);
    string expressionToJSON();
    string dataToJSON();
    // evaluation
    void setPlot();
    // gui
    void enablePositionButtons(bool flag_enable);
    void enableCombinations(bool flag_enable);
    void resetCombinations();
    void setComboBoxPlot();

signals:
    void moveUp(int guiNumber);
    void moveDown (int guiNumber);
    void killThis (int guiNumber);

private slots:
    void on_pushButton_Details_clicked();
    void on_pushButton_Remove_clicked();
    void on_pushButton_Up_clicked();
    void on_pushButton_Down_clicked();
    void on_checkBox_Skip_toggled(bool checked);
    void on_comboBox_Interact_currentIndexChanged(int index);
    void on_comboBox_Plot_currentIndexChanged(int index);

private:
    Ui::InequalityLoader *ui;
    // data
    vector<QVector<double> > m_x_results;
    vector<QVector<double> > m_y_results;
    vector<string> m_details;
    vector<string> m_expressions;
    string sDetails;
    // meta
    int m_gui_number;
    bool flag_skip;
    bool flag_problem;
    string m_name;
    string m_filename;
    string m_error_message;
    // evaluation
    unsigned int m_current_plot;

    // functions

};

#endif // INEQUALITYLOADER_H
