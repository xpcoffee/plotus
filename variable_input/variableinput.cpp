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

#include "variableinput.h"
#include "ui_variableinput.h"


//	"""""""""""""""""""""""""""""""""
//	"		Static Functions		"
//	"""""""""""""""""""""""""""""""""

static void setQLineEditBackground(QLineEdit* lineEdit, string fg, string bg){
    string str = 	"QLineEdit{ color: " + fg + "; background: " + bg + ";}";
    lineEdit->setStyleSheet(QString::fromStdString(str));
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""


//	Constructor
//	-----------

VariableInput::VariableInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VariableInput)
{
    ui->setupUi(this);
    // global variable initialization
    flag_initialized = false;
    // input validation
    QDoubleValidator *dValidator = new QDoubleValidator;
    QIntValidator *iValidator = new QIntValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEditElements->setValidator(iValidator);
    ui->lineEditMax->setValidator(dValidator);
    ui->lineEditMin->setValidator(dValidator);
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
    if	(ui->lineEditElements->text().isEmpty()){
        setQLineEditBackground(ui->lineEditElements, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEditMin->text().isEmpty()) {
        setQLineEditBackground(ui->lineEditMin, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEditMax->text().isEmpty()){
        setQLineEditBackground(ui->lineEditMax, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    if	(ui->lineEditName->text().isEmpty()){
        setQLineEditBackground(ui->lineEditName, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // remove whitespace in name lineEdit
    ui->lineEditName->setText(ui->lineEditName->text().replace(QString(" "), QString("")));
    // illegal variable name
    if (!Variable::nameIsLegal(ui->lineEditName->text().toStdString())){
        setQLineEditBackground(ui->lineEditName, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // at least 1 element
    if 	(ui->lineEditElements->text().toInt() < 1){
        setQLineEditBackground(ui->lineEditElements, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    // max and min not equal
    if  (ui->lineEditMax->text().toDouble() == ui->lineEditMin->text().toDouble()){
        setQLineEditBackground(ui->lineEditMax, COLOR_ERROR_FG, COLOR_ERROR_BG);
        setQLineEditBackground(ui->lineEditMin, COLOR_ERROR_FG, COLOR_ERROR_BG);
        flag_isOK = false;
    }
    return flag_isOK;
}


// 	Formatting
//	----------

void VariableInput::highlightName(){
    setQLineEditBackground(ui->lineEditName, COLOR_ERROR_FG, COLOR_ERROR_BG);
}

void VariableInput::clearFormatting(){
    setQLineEditBackground(ui->lineEditName, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEditElements, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEditMax, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    setQLineEditBackground(ui->lineEditMin, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
}

void VariableInput::clearFields(){
    ui->lineEditName->clear();
    ui->lineEditMin->clear();
    ui->lineEditMax->clear();
    ui->lineEditElements->clear();
}


//	Setters
//	-------

void VariableInput::setAxisMode(int nMode){
    switch(nMode){
    case 0:
        nAxisMode = MODE_X_AXIS;
        ui->comboBoxAxes->setCurrentIndex(MODE_X_AXIS);
        break;
    case 1:
        nAxisMode = MODE_Y_AXIS;
        ui->comboBoxAxes->setCurrentIndex(MODE_Y_AXIS);
        break;
    case 2:
        nAxisMode = MODE_POINT;
        ui->comboBoxAxes->setCurrentIndex(MODE_POINT);
        break;
    default:
        cerr << "[ERROR] VariableInput | setAxisMode | " << "Unknown axis mode: " << nMode << endl;
        break;
    }
}

void VariableInput::setNumber(int nNumber){
    nVariableInputNumber = nNumber;
}


// 	Getters
//	-------

int VariableInput::getAxisMode(){
    return nAxisMode;
}

int VariableInput::getNumber(){
    return nVariableInputNumber;
}

string VariableInput::getUnits(){
    return ui->lineEditUnits->text().toStdString();
}

Variable VariableInput::getVariable(){
    if (ui->comboBoxAxes->currentIndex() == MODE_POINT)	{
        cout << "[INFO] variableinput.cpp | getvariable | " << "creating point variable" << endl;
        createPoint();
    } else {
        cout << "[INFO] variableinput.cpp | getvariable | " << "creating variable" << endl;
        createVariable();
    }
    return mVariable;
}


//	GUI
//	---

void VariableInput::enableRemoveButton(bool flag_enable){
    ui->pushButtonDelete->setEnabled(flag_enable);
}

//	"""""""""""""""""""""""""""""""""
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""

void VariableInput::sliderCheck(){
    resetSlider();
    if (ui->lineEditMin->text().isEmpty() || ui->lineEditMax->text().isEmpty() || ui->lineEditElements->text().isEmpty())
        return;
    if (!checkInput())
        return;
    createVariable();
    if (ui->comboBoxAxes->currentIndex() == MODE_POINT){
       ui->horizontalSliderPoint->setEnabled(true);
       ui->horizontalSliderPoint->setTickInterval(1);
       ui->labelPoint->setNum(mVariable.getMin());
       ui->horizontalSliderPoint->setMaximum(mVariable.getElements());
    }
}

void VariableInput::createVariable(){
    mVariable = Variable(	ui->lineEditName->text().toStdString(),
                            ui->lineEditMin->text().toDouble(),
                            ui->lineEditMax->text().toDouble(),
                            ui->lineEditElements->text().toInt());
    flag_initialized = true;
}

void VariableInput::createPoint(){
    double dSelectedValue = mVariable.getMin() + ui->horizontalSliderPoint->value()*(mVariable.getMax()-mVariable.getMin())/mVariable.getElements();
    mVariable = Variable(	ui->lineEditName->text().toStdString(),
                            dSelectedValue,
                            dSelectedValue,
                            1);
}

void VariableInput::resetSlider(){
       ui->horizontalSliderPoint->setValue(0);
       ui->labelPoint->setText("-");
       ui->horizontalSliderPoint->setEnabled(false);
}

//	"""""""""""""""""""""""""""""""""
//	"		Private Slots			"
//	"""""""""""""""""""""""""""""""""

void VariableInput::on_comboBoxAxes_currentIndexChanged(int index)
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

void VariableInput::on_horizontalSliderPoint_sliderMoved(int position)
{
    createVariable();
    double dSelectedValue = mVariable.getMin() + position*(mVariable.getMax()-mVariable.getMin())/mVariable.getElements();
    ui->labelPoint->setNum(dSelectedValue);
}

void VariableInput::on_lineEditElements_editingFinished()
{
    clearFormatting();
    sliderCheck();
}

void VariableInput::on_lineEditMax_editingFinished()
{
    clearFormatting();
    sliderCheck();
}

void VariableInput::on_lineEditMin_editingFinished()
{
    clearFormatting();
    sliderCheck();
}

void VariableInput::on_lineEditName_editingFinished()
{
    // remove whitespace in name lineEdit
    ui->lineEditName->setText(ui->lineEditName->text().replace(QString(" "), QString("")));
    // illegal variable name
    if (!ui->lineEditName->text().isEmpty() && !Variable::nameIsLegal(ui->lineEditName->text().toStdString())){
        setQLineEditBackground(ui->lineEditName, COLOR_ERROR_FG, COLOR_ERROR_BG);
    } else {
        setQLineEditBackground(ui->lineEditName, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    }
}

void VariableInput::on_pushButtonDelete_clicked()
{
    emit killThis(nVariableInputNumber);
}
