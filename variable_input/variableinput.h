#ifndef VARIABLEINPUT_H
#define VARIABLEINPUT_H

//	"""""""""""""""""""""""""""""""""
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""
#include <QWidget>
#include <QSplitter>
#include <qvalidator.h>
#include "../deplot/include/variable.h"


//	"""""""""""""""""""""""""""""""""
//	"	Preprocessor Definitions	"
//	"""""""""""""""""""""""""""""""""

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

//	"""""""""""""""""""""""""""""""""
//	"		Enumerated Types		"
//	"""""""""""""""""""""""""""""""""
enum AXIS_MODE{
    MODE_X_AXIS = 0,
    MODE_Y_AXIS = 1,
    MODE_POINT	= 2,
};

namespace Ui {
class VariableInput;
}

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
    void setAxisMode(int);
    void setNumber(int);
    // 	getters
    int getAxisMode();
    int getNumber();
    string getUnits();
    string toJSON();
    Variable getVariable();
    //	parsers
    void fromJSON(string);
    //	gui
    void enableRemoveButton(bool);

signals:
    void axisModeChanged(int nVarInputNumber);
    void killThis(int nVariableInputNumber);

public slots:
    void splitterResize(QList<int>);

private slots:
    void on_comboBoxAxes_currentIndexChanged(int index);
    void on_horizontalSliderPoint_sliderMoved(int position);
    void on_lineEditElements_textChanged(const QString &arg1);
    void on_lineEditMax_textChanged(const QString &arg1);
    void on_lineEditMin_textChanged(const QString &arg1);
    void on_lineEditName_textChanged(const QString &arg1);

    void on_pushButtonDelete_clicked();

private:
    // variables
    Ui::VariableInput *ui;
    Variable mVariable;
    bool flag_initialized;
    int nVariableInputNumber;
    int nAxisMode;

    // functions
    void sliderCheck();
    void createVariable();
    void createPoint();
    void resetSlider();
};

#endif // VARIABLEINPUT_H
