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
#include "inequalityinput.h"	// CombinationMode, ComboBoxColor, ComboBoxShape
#include "bluejson.h"

///	Namespaces
///	===========

using namespace std;

namespace Ui {
class InequalityLoader;
}

///	Structs
///	========

struct DetailItem
{
    int level;
    QString type;
    QString description;
    QString data1;
    QString data2;
};

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
    void clearCombinationResults();
    void createDetailItem(int level, QString type, QString desc, QString data1, QString data2);

    //	getters: ui
    QString getName();
    int getNumber();
    int getShapeIndex();
    int getCombination();
    int getPlot();
    bool getSkip();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();
    QColor getColor();
    QwtSymbol::Style getShape();

    //	getters: data
    QVector<double> getX();
    QVector<double> getY();
    QVector<double> getXProblem();
    QVector<double> getYProblem();
    string getFile();
    string getErrors();

    //	parsers
    void parsePlotData(string json);
    void parseProblem(string problem);
    void formatVariables(string json);
    void formatInequality(string json);
    void formatCase(string json);
    void formatExpressions(string json);
    string expressionToJSON();
    string dataToJSON();

    template <typename T>
    inline string htmlTable(T value, string options = "align=\"center\"");
    template <typename T>
    inline string htmlTableCell (T value, string options = "align=\"center\"");
    template <typename T>
    inline string htmlTableRow (T value, string options = "");
    template <typename T>
    inline string htmlTableHeader (T value, string options = "");

    //	evaluation
    void setPlot(int index = -1);

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
    vector< QVector<double> > m_xResults, m_yResults;
    vector< QVector<double> > m_xResults_Problem, m_yResults_Problem;
    QVector<double> m_xResults_combination, m_yResults_combination;
    vector<string> m_detailsHTML;
    vector<string> m_detailsJSON;

    //	meta
    QStringList m_dropDownList;
    QList<DetailItem> m_details;
    int m_guiNumber;
    int m_detailLevel;
    bool flag_skip;
    bool flag_problem;
    string m_name;
    string m_filename;
    string m_variables;
    string m_errorMessage;

    //	evaluation
    unsigned int m_currentPlot;
};

#endif // INEQUALITYLOADER_H
