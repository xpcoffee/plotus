#ifndef BAREMINIMUMPLOTTER_H
#define BAREMINIMUMPLOTTER_H

#include <QMainWindow>
#include <QtWidgets>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <qvalidator.h>
#include <qlocale.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "variable.h"
#include "inequality.h"
#include "expression.h"
#include "variableinput.h"
#include "inequalityinput.h"
#include "inequalityloader.h"
#include "bluejson.h"

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
    void plotNew(int, string, string, int);
    void plotOld(int, string, string, int);
    void vectorCombineNone(int);
    void vectorCombineIntersection(int);
    void vectorCombineUnion(int);
    void vectorCombineSubtraction(int);
    void formatGraph(int, int);
    void formatErrorGraph();
    // 	validation
    void print(string);
    void printclr();
    void printError();
    // 	gui
    void addVariableInput();
    void addInequalityInput();
    void addInequalityLoader(string filename = "");
    void reOrderInequalityInputs();
    void setCombinationInputs();
    void resetTabOrder();
    void clearFormatting();
    void clearGUI();
    QWidget *getFocusInWidget(QWidget* widget);
    QWidget *getFocusOutWidget(QWidget* widget);
    //	parsing and file i/o
    void save_JSON(QString);
    void open_variables(string json);

signals:
    void variableSplitterMoved(QList<int>);

public slots:
    void checkAxisMode(int);
    void removeVariableInput(int);
    void removeInequalityInput(int);
    void moveInequalityInputUp(int);
    void moveInequalityInputDown(int);

private slots:
    void menu_about();
    void menu_open();
    void menu_saveAs();
    void menu_new();
    void menu_quit();
    void on_button_AddVariable_clicked();
    void on_button_AddInequality_clicked();
    void on_button_Plot_clicked();
    void on_pushButton_AddInequalityLoader_clicked();
    void on_splitter_variable_splitterMoved(int pos, int index);
    void on_lineEdit_SettingsTolerance_editingFinished();

private:
    Ui::BareMinimumPlotter *ui;
    //	plotter elements
    vector<VariableInput*> m_VariableInputs;
    vector<InequalityInput*> m_InequalityInputs;
    vector<InequalityLoader*> m_InequalityLoaders;
    //	plotting
    int m_graph_count;
    int m_prev_combination;
    bool flag_Combination;
    Variable m_XVariable, m_YVariable;
    QVector<double> x_results, y_results, x_results_problem, y_results_problem;
    //	gui management
    int m_variable_count;
    int m_inequality_count;
    bool flag_Saved;
    //	error handling
    bool flag_Problem;
    bool flag_Empty;
    string m_error_message;
    // 	settings
    double m_tolerance;
    string m_default_directory;
    // experimental

};

#endif // BAREMINIMUMPLOTTER_H
