/*
    Emerick Bosch
    June 2014
    VariableInput.cpp

    Modular component that allows variable information to be input.
    Modularity allows for dynamic addition and deletion of variables.
*/

//	"""""""""""""""""""""""""""""""""
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""

#include "include/variableinput.h"
#include "ui_variableinput.h"
#include  <sstream>


//	"""""""""""""""""""""""""""""""""
//	"		Static Functions		"
//	"""""""""""""""""""""""""""""""""

static void setQLineEditBackground(QLineEdit* lineEdit_, string fg, string bg){
    string str = 	"QLineEdit{ color: " + fg + "; background: " + bg + ";}";
    lineEdit_->setStyleSheet(QString::fromStdString(str));
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""


//	Constructor
//	-----------

VariableInput::VariableInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VariableInput),
    flag_initialized(false)
{
    ui->setupUi(this);
    // global variable initialization
    setAccessibleDescription("variable-input");
    // input validation
    QDoubleValidator *dValidator = new QDoubleValidator;
    QIntValidator *iValidator = new QIntValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEdit_Elements->setValidator(iValidator);
    ui->lineEdit_Max->setValidator(dValidator);
    ui->lineEdit_Min->setValidator(dValidator);
    //	splitters
    for(int i = 0; i < ui->splitter->count(); i++){
        ui->splitter->handle(i)->setEnabled(false);
    }
}


//	Destructor
//	-----------

VariableInput::~VariableInput()
{
    delete ui;
}


//	Validation
//	----------

bool VariableInput::checkInput(){
    bool flag_isOK = true;
    // all fields filled in
    if	(ui->lineEdit_Elements->text().isEmpty()){
        setQLineEditBackground(ui->lineEdit_Elements, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Min->text().isEmpty()) {
        setQLineEditBackground(ui->lineEdit_Min, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Max->text().isEmpty()){
        setQLineEditBackground(ui->lineEdit_Max, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Name->text().isEmpty()){
        setQLineEditBackground(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // remove whitespace in name lineEdit_
    ui->lineEdit_Name->setText(ui->lineEdit_Name->text().replace(QString(" "), QString("")));
    // illegal variable name
    if (!Variable::nameIsLegal(ui->lineEdit_Name->text().toStdString())){
        setQLineEditBackground(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // at least 1 element
    if 	(ui->lineEdit_Elements->text().toInt() < 1){
        setQLineEditBackground(ui->lineEdit_Elements, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // max and min not equal
    if  (ui->lineEdit_Max->text().toDouble() == ui->lineEdit_Min->text().toDouble()){
        setQLineEditBackground(ui->lineEdit_Max, COLOR_ERROR_FG, COLOR_ERROR_BG);
        setQLineEditBackground(ui->lineEdit_Min, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    return flag_isOK;
}


// 	Formatting
//	----------

void VariableInput::highlightName(){ setQLineEditBackground(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG); }

void VariableInput::clearFormatting(){
    setQLineEditBackground(ui->lineEdit_Name, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEdit_Elements, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEdit_Max, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEdit_Min, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
}

void VariableInput::clearFields(){
    ui->lineEdit_Name->clear();
    ui->lineEdit_Min->clear();
    ui->lineEdit_Max->clear();
    ui->lineEdit_Elements->clear();
    ui->comboBox_Axes->setCurrentIndex(0);
}


//	Setters
//	-------

void VariableInput::setAxisMode(int nMode){
    switch(nMode){
    case 0:
        nAxisMode = MODE_X_AXIS;
        ui->comboBox_Axes->setCurrentIndex(MODE_X_AXIS);
        break;
    case 1:
        nAxisMode = MODE_Y_AXIS;
        ui->comboBox_Axes->setCurrentIndex(MODE_Y_AXIS);
        break;
    case 2:
        nAxisMode = MODE_POINT;
        ui->comboBox_Axes->setCurrentIndex(MODE_POINT);
        break;
    default:
        cerr << "[ERROR] VariableInput | setAxisMode | " << "Unknown axis mode: " << nMode << endl;
        break;
    }
}

void VariableInput::setNumber(int nNumber){ nVariableInputNumber = nNumber; }


// 	Getters
//	-------

int VariableInput::getAxisMode(){ return nAxisMode; }

int VariableInput::getNumber(){ return nVariableInputNumber; }

string VariableInput::getUnits(){ return ui->lineEdit_Units->text().toStdString(); }

string VariableInput::toJSON(){
    ostringstream buffer;
    createVariable();
    buffer <<  "{\"name\":\"" << mVariable.getName() << "\"," <<
            "\"min\":" << mVariable.getMin() << "," <<
            "\"max\":" << mVariable.getMax() << "," <<
            "\"elements\":" << mVariable.getElements() << "," <<
            "\"units\":\"" << getUnits() << "\"";
    if (ui->comboBox_Axes->currentIndex() == MODE_POINT)
        buffer << ",\"slider point\":" << ui->horizontalSlider_Point->value();
    if (ui->comboBox_Axes->currentIndex() == MODE_X_AXIS)
        buffer << ",\"axis\":\"x\"";
    if (ui->comboBox_Axes->currentIndex() == MODE_Y_AXIS)
        buffer << ",\"axis\":\"y\"";
    buffer << "}";
    return buffer.str();
}

Variable VariableInput::getVariable(){
    if (ui->comboBox_Axes->currentIndex() == MODE_POINT) { createPoint(); }
    else { createVariable(); }
    return mVariable;
}

QWidget* VariableInput::getFocusInWidget() { return ui->lineEdit_Name; }

QWidget* VariableInput::getFocusOutWidget()
{
    QWidget::setTabOrder(ui->lineEdit_Name, ui->lineEdit_Min);
    QWidget::setTabOrder(ui->lineEdit_Min, ui->lineEdit_Max);
    QWidget::setTabOrder(ui->lineEdit_Max, ui->lineEdit_Elements);
    QWidget::setTabOrder(ui->lineEdit_Elements, ui->lineEdit_Units);
    return ui->lineEdit_Units;
}


//	Parsers
//	-------

// TODO: Make this more elegant
void VariableInput::fromJSON(string sInput){
    string token;
    stringstream ss;
    ss << sInput;
    while (getline (ss, token, '"')){
        if (token == "name")
            if (getline (ss, token, '"'))
                if (getline (ss, token, '"'))
                    ui->lineEdit_Name->setText(QString::fromStdString(token));
        if (token == "min")
            if (getline (ss, token, ':'))
                if (getline (ss, token, ','))
                    ui->lineEdit_Min->setText(QString::fromStdString(token));
        if (token == "max")
            if (getline (ss, token, ':'))
                if (getline (ss, token, ','))
                    ui->lineEdit_Max->setText(QString::fromStdString(token));
        if (token == "elements")
            if (getline (ss, token, ':'))
                if (getline (ss, token, ',')){
                    ui->lineEdit_Elements->setText(QString::fromStdString(token));
                }
        if (token == "units")
            if (getline (ss, token, '"'))
                if (getline (ss, token, '"'))
                    ui->lineEdit_Units->setText(QString::fromStdString(token));
        if (token == "slider point")
            if (getline (ss, token, ':'))
                if (getline (ss, token, ',')){
                    stringstream ss;
                    int value;
                    ss << token;
                    if(!(ss >> value))
                        value = 0;
                    sliderCheck();
                    ui->horizontalSlider_Point->setValue(value);
                    double dSelectedValue = mVariable.getMin() + value*(mVariable.getMax()-mVariable.getMin())/mVariable.getElements();
                    ui->label_Point->setNum(dSelectedValue);
                }
        if (token == "axis")
            if (getline (ss, token, '"'))
                if (getline (ss, token, '"')){
                    if (token == "x")
                        ui->comboBox_Axes->setCurrentIndex(MODE_X_AXIS);
                    if (token == "y")
                        ui->comboBox_Axes->setCurrentIndex(MODE_Y_AXIS);
                }
    }
}


//	GUI
//	---

void VariableInput::enableRemoveButton(bool flag_enable){
    ui->pushButton_Delete->setEnabled(flag_enable);
}

//	"""""""""""""""""""""""""""""""""
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""

void VariableInput::sliderCheck(){
    resetSlider();
    if (ui->lineEdit_Min->text().isEmpty() || ui->lineEdit_Max->text().isEmpty() || ui->lineEdit_Elements->text().isEmpty())
        return;
    if (!checkInput())
        return;
    createVariable();
    if (ui->comboBox_Axes->currentIndex() == MODE_POINT){
       ui->horizontalSlider_Point->setEnabled(true);
       ui->horizontalSlider_Point->setTickInterval(1);
       ui->label_Point->setNum(mVariable.getMin());
       ui->horizontalSlider_Point->setMaximum(mVariable.getElements());
    }
}

void VariableInput::createVariable(){
    mVariable = Variable(	ui->lineEdit_Name->text().toStdString(),
                            ui->lineEdit_Min->text().toDouble(),
                            ui->lineEdit_Max->text().toDouble(),
                            ui->lineEdit_Elements->text().toInt());
    flag_initialized = true;
}

void VariableInput::createPoint(){
    double dSelectedValue = mVariable.getMin() + ui->horizontalSlider_Point->value()*(mVariable.getMax()-mVariable.getMin())/mVariable.getElements();
    mVariable = Variable(	ui->lineEdit_Name->text().toStdString(),
                            dSelectedValue,
                            dSelectedValue,
                            1);
}

void VariableInput::resetSlider(){
       ui->horizontalSlider_Point->setValue(0);
       ui->label_Point->setText("-");
       ui->horizontalSlider_Point->setEnabled(false);
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Slots			"
//	"""""""""""""""""""""""""""""""""

void VariableInput::splitterResize(QList<int> sizes){
    ui->splitter->setSizes(sizes);
}


//	"""""""""""""""""""""""""""""""""
//	"		Private Slots			"
//	"""""""""""""""""""""""""""""""""

void VariableInput::on_comboBox_Axes_currentIndexChanged(int index)
{
   clearFormatting();
   switch(index) {
   case 0:
        nAxisMode = MODE_X_AXIS;
        resetSlider();
        emit axisModeChanged(nVariableInputNumber);
        break;
   case 1:
        nAxisMode = MODE_Y_AXIS;
        resetSlider();
        emit axisModeChanged(nVariableInputNumber);
        break;
   case 2:
        nAxisMode = MODE_POINT;
        sliderCheck();
        emit axisModeChanged(nVariableInputNumber);
        break;
   }
}

void VariableInput::on_horizontalSlider_Point_sliderMoved(int position)
{
    createVariable();
    double dSelectedValue = mVariable.getMin() + position*(mVariable.getMax()-mVariable.getMin())/mVariable.getElements();
    ui->label_Point->setNum(dSelectedValue);
}


void VariableInput::on_lineEdit_Elements_textChanged(const QString&) { clearFormatting(); sliderCheck(); }

void VariableInput::on_lineEdit_Max_textChanged(const QString&) { clearFormatting(); sliderCheck(); }

void VariableInput::on_lineEdit_Min_textChanged(const QString&) { clearFormatting(); sliderCheck(); }

void VariableInput::on_lineEdit_Name_textChanged(const QString&)
{
    // remove whitespace in name lineEdit_
    ui->lineEdit_Name->setText(ui->lineEdit_Name->text().replace(QString(" "), QString("")));
    // illegal variable name
    if (!ui->lineEdit_Name->text().isEmpty() && !Variable::nameIsLegal(ui->lineEdit_Name->text().toStdString())){
        setQLineEditBackground(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG);
    } else {
        setQLineEditBackground(ui->lineEdit_Name, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    }
}

void VariableInput::on_pushButton_Delete_clicked() { emit killThis(nVariableInputNumber); }
