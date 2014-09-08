/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    expression.h
    -------------

    Description
    ============
    Provides the GUI for:
    - organising input widgets for inequalities and variables
    - combining, plotting, saving and opening evaluation results
    - editing evaluation settings

    Saving and opening is done in JSON format.

    Subwidgets:
    - InequalityInput
    - InequalityLoader
    - VariableInput

    First Party Code
    =================
    A custom parser was coded for use in Plotus and is included:
    BlueJSON	-	Emerick Bosch	-	July 2014

    Third Party Code
    =================
    Plotting functionality is provided through QWT:
    http://qwt.sourceforge.net/
    link date: 29 July 2014

*/

#ifndef PLOTUS_H
#define PLOTUS_H

///	Includes
///	=========

#include <QMainWindow>
#include <QtWidgets>
#include <QThread>
#include <QDesktopServices>
#include <qvalidator.h>
#include <qlocale.h>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>

#include "variable.h"
#include "inequality.h"
#include "expression.h"
#include "variableinput.h"
#include "inequalityinput.h"
#include "inequalityloader.h"
#include "bluejson.h"
#include "plotworker.h"
#include "exportdialog.h"


///	Enumerated Types
///	=================

enum UIMode {
    Busy = 0,
    Available
};

///	TypeDefs
///	=========

typedef vector<VariableInput*> VarInputArray;
typedef vector<InequalityInput*> IneqInputArray;
typedef vector<InequalityLoader*> IneqLoaderArray;
typedef QVector<QPointF> PlottingVector;
typedef QwtSymbol::Style PlotStyle;
typedef std::numeric_limits<double> precDouble	;

///	Namespaces
///	===========

using namespace std;

namespace Ui {
class Plotus;
}


///	Class
///	======

class Plotus : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plotus(QWidget *parent = 0);
    ~Plotus();

    //	overrides
    void showEvent(QShowEvent *event);

    // 	plotting and evaluation
    void plot();
    void configurePlot();
    void configureAxes();
    bool addVariables(InequalityInput *input);

    // 	validation
    void printclr();
    void printError();
    bool checkVariables();
    bool checkExpressions();

    // 	gui
    void setupUiCont();
    void setupInputValidation();
    void setupQwtPlot();
    void setupScrollAreas();
    void setupSplitterHandles();
    void setupButtons();
    void setupDynamicUi();
    void loadCSS();
    void loadSettings();

    void clearGUI();
    void clearFormatting();
    void resetQwtPlotWindow();
    void addVariableInput();
    void addInequalityInput();
    void addInequalityLoader(QString filename = "");
    void determineInequalityOrder();
    void determineButtonStates();
    void determineTabOrder();
    void setUIMode(UIMode mode);

    QWidget *getFocusInWidget(QWidget *widget);
    QWidget *getFocusOutWidget(QWidget *widget);

    //	parsing and file i/o
    void saveCase_JSON(QString filename);
    void openCase(QString filename);
    void openVariables(string json);

signals:
    void variableSplitterMoved(QList<int> sizes);
    void inequalitySplitterMoved(QList<int> sizes);
    void plotThreadCancel();
    void feedPlotWorker(VarInputArray var_inputs,
                        IneqInputArray ineq_inputs,
                        IneqLoaderArray ineq_loaders,
                        Variable x_variable,
                        Variable y_variable,
                        double comparison_precision);

public slots:
    void checkAxisMode(int gui_number);
    void removeVariableInput(int gui_number);
    void removeInequalityInput(int gui_number);
    void moveInequalityInputUp(int gui_number);
    void moveInequalityInputDown(int gui_number);
    void scrollInequalityHeader(int value);
    void scrollVariableHeader(int value);


    void sendWorkerData();
    void setProgress(int value, QString message);
    void addGraph(PlottingVector qwt_samples, PlotStyle shape, QColor color, QString tag = "");
    void addErrorGraph(PlottingVector qwt_problem_samples);
    void registerMemberChanges(VarInputArray var_inputs,
                    IneqInputArray ineq_inputs,
                    IneqLoaderArray ineq_loaders);
    void log(QString message);
    void plottingFinished();

private slots:
    void menu_about();
    void menu_open();
    void menu_saveAs();
    void menu_export();
    void menu_new();
    void menu_quit();
    void menu_qwt_context(const QPoint &pos);
    void exportQwt(int width, int height, int dpi);
    void copyQwtToClipboard();
    void openLink_github();
    void on_toolButton_AddVariable_clicked();
    void on_toolButton_AddInequality_clicked();
    void on_toolButton_Plot_clicked();
    void on_toolButton_AddInequalityLoader_clicked();
    void on_lineEdit_SettingsTolerance_editingFinished();
    void on_splitter_VariableHeader_splitterMoved(int pos, int index);
    void on_splitter_InequalityHeader_splitterMoved(int pos, int index);
    void on_lineEdit_PlotTitle_returnPressed();
    void on_lineEdit_PlotTitle_textChanged(const QString &arg1);
    void on_lineEdit_PlotTitle_editingFinished();
    void on_pushButton_Cancel_clicked();

private:
    Ui::Plotus *ui;

    //	plotter elements
    QwtPlot *plotter;
    VarInputArray m_variableInputs;
    IneqInputArray m_inequalityInputs;
    IneqLoaderArray m_inequalityLoaders;

    //	plotting
    QString m_title;
    int m_graphCount;
    int m_prevCombination;
    Variable m_xVariable, m_yVariable;
    QThread *thread;
    PlotWorker *worker;

    //	gui management
    int m_variableCount;
    int m_inequalityCount;
    bool flag_Saved;
    bool flag_Cancel;

    //	error handling
    bool flag_Problem;
    bool flag_Empty;
    QString m_errorMessage;

    // 	settings
    double m_compPrec;
    QString m_defaultDir;

    //	static & 3rd party
    static void elideLable(QLabel *label, QString text);
    static void fitLineEditToContents(QLineEdit *edit);

        //	-- chelmuth
    static void removeFocusRect(QWidget *widget);

        //	-- David Walthall
    static void putLinesInSplitterHandles(QSplitter *splitter, QFrame::Shape line_type, int size = 0);
};


///	Template Definitions
///	=====================

#endif // PLOTUS_H
