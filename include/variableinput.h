#ifndef VARIABLEINPUT_H
#define VARIABLEINPUT_H

//	Includes
#include <QWidget>
#include <QSplitter>
#include <qvalidator.h>
#include "variable.h"


//	Preprocessor Definitions
#ifndef COLOR_DEFAULT_BG
#define COLOR_DEFAULT_BG "white"
#endif

#ifndef COLOR_DEFAULT_FG
#define COLOR_DEFAULT_FG "black"
#endif

#ifndef COLOR_ERROR_BG
#define COLOR_ERROR_BG "red"
#endif

#ifndef COLOR_ERROR_FG
#define COLOR_ERROR_FG "white"
#endif

//	Enumerators
enum AXIS_MODE{
    MODE_X_AXIS = 0,
    MODE_Y_AXIS = 1,
    MODE_POINT	= 2,
};

//	Namespace
namespace Ui {
class VariableInput;
}

//	Class
class VariableInput : public QWidget
{
    Q_OBJECT

public:
    explicit VariableInput(QWidget *parent = 0);
    ~VariableInput();

    // validation
    bool checkInput();

    // formatting
    void clearFields();
    void clearFormatting();
    void highlightName();

    //	setters
    void setAxisMode(int axis_mode);
    void setNumber(int gui_number);

    // 	getters
    int getAxisMode();
    int getNumber();
    string getUnits();
    string toJSON();
    Variable getVariable();
    QWidget *getFocusInWidget();
    QWidget *getFocusOutWidget();

    //	parsers
    void fromJSON(string json);

    //	gui
    void enableRemoveButton(bool flag_enable);
    void setSplitterSizes(QList<int> sizes);

signals:
    void axisModeChanged(int gui_number);
    void killThis(int gui_number);

public slots:
    void splitterResize(QList<int> sizes);

private slots:
    void on_comboBox_Axes_currentIndexChanged(int index);
    void on_horizontalSlider_Point_sliderMoved(int position);
    void on_lineEdit_Elements_textChanged(const QString &arg1);
    void on_lineEdit_Max_textChanged(const QString &arg1);
    void on_lineEdit_Min_textChanged(const QString &arg1);
    void on_lineEdit_Name_textChanged(const QString &arg1);
    void on_pushButton_Delete_clicked();

private:
    Ui::VariableInput *ui;

    //	data
    Variable m_variable;

    //	meta
    bool flag_initialized;
    int m_guiNumber;
    int m_axisMode;

    //	validation
    void sliderCheck();

    //	evaluation
    void createVariable();
    void createPoint();

    //	gui
    void resetSlider();
};

#endif // VARIABLEINPUT_H
