/*
    Emerick Bosch
    June 2014
    VariableInput.cpp

    Modular component that allows variable information to be input.
    Modularity allows for dynamic addition and deletion of variables.
*/

///	Includes
/// =========

#include "include/variableinput.h"
#include "ui_variableinput.h"


///	Public Functions
/// =================

VariableInput::VariableInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VariableInput),
    flag_initialized(false)
{
    ui->setupUi(this);
    // global variable initialization
    setAccessibleDescription("variable-input");

    DefaultColors <<
         //! Normal Background
         "white" <<
         //! Normal Foreground
         "black" <<
         //! Error Background
         "red" <<
         //! Error Foreground
         "white";

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

VariableInput::~VariableInput()
{
    delete ui;
}

bool VariableInput::checkInput()
{
    bool flag_isOK = true;
    // all fields filled in
    if	(ui->lineEdit_Elements->text().isEmpty()){
        formatLineEditError(ui->lineEdit_Elements);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Min->text().isEmpty()) {
        formatLineEditError(ui->lineEdit_Min);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Max->text().isEmpty()){
        formatLineEditError(ui->lineEdit_Max);
        flag_isOK = false;
    }
    if	(ui->lineEdit_Name->text().isEmpty()){
        formatLineEditError(ui->lineEdit_Name);
        flag_isOK = false;
    }

    // remove whitespace in name
    ui->lineEdit_Name->setText(ui->lineEdit_Name->text().replace(QString(" "), QString("")));

    // illegal variable name
    if (!Variable::nameIsLegal(ui->lineEdit_Name->text().toStdString())){
        formatLineEditError(ui->lineEdit_Name);
        flag_isOK = false;
    }

    // at least 1 element
    if 	(ui->lineEdit_Elements->text().toInt() < 1){
        formatLineEditError(ui->lineEdit_Elements);
        flag_isOK = false;
    }

    // max and min not equal
    if  (ui->lineEdit_Max->text().toDouble() == ui->lineEdit_Min->text().toDouble()){
        formatLineEditError(ui->lineEdit_Max);
        formatLineEditError(ui->lineEdit_Min);
        flag_isOK = false;
    }
    return flag_isOK;
}

void VariableInput::highlightName(){ formatLineEdit(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG); }

void VariableInput::clearFormatting()
{
    formatLineEditNormal(ui->lineEdit_Name);
    formatLineEditNormal(ui->lineEdit_Elements);
    formatLineEditNormal(ui->lineEdit_Min);
    formatLineEditNormal(ui->lineEdit_Max);
}

void VariableInput::clearFields()
{
    ui->lineEdit_Name->clear();
    ui->lineEdit_Min->clear();
    ui->lineEdit_Max->clear();
    ui->lineEdit_Elements->clear();
    ui->comboBox_Axes->setCurrentIndex(0);
}

void VariableInput::setAxisMode(int axis_mode)
{
    switch(axis_mode){
    case 0:
        m_axisMode = PlotHorizontal;
        ui->comboBox_Axes->setCurrentIndex(PlotHorizontal);
        break;
    case 1:
        m_axisMode = PlotVertical;
        ui->comboBox_Axes->setCurrentIndex(PlotVertical);
        break;
    case 2:
        m_axisMode = PlotConstant;
        ui->comboBox_Axes->setCurrentIndex(PlotConstant);
        break;
    default:
        assert(false && "VariableInput: Unknown axis mode. " && __LINE__ && __FILE__);
        break;
    }
}

void VariableInput::setNumber(int gui_number){ m_guiNumber = gui_number; }

int VariableInput::getAxisMode(){ return m_axisMode; }

int VariableInput::getNumber(){ return m_guiNumber; }

QString VariableInput::getUnits(){ return ui->lineEdit_Units->text(); }

QString VariableInput::toJSON()
{
    stringstream buffer;
    createVariable();	// ensure that variable is not a constant

    buffer <<	"{\"name\":\""	<< m_variable.name()		<< "\"," <<
                "\"min\":"		<< m_variable.min()			<< "," <<
                "\"max\":"		<< m_variable.max()			<< "," <<
                "\"elements\":"	<< m_variable.elements()	<< "," <<
                "\"units\":\""	<< getUnits().toStdString()	<< "\"";

    if (ui->comboBox_Axes->currentIndex() == PlotConstant)
        buffer << ",\"slider constant\":" << ui->horizontalSlider_Point->value();
    if (ui->comboBox_Axes->currentIndex() == PlotHorizontal)
        buffer << ",\"axis\":\"x\"";
    if (ui->comboBox_Axes->currentIndex() == PlotVertical)
        buffer << ",\"axis\":\"y\"";

    buffer << "}";
    return QString::fromStdString( buffer.str() );
}

Variable VariableInput::getVariable()
{
    if (ui->comboBox_Axes->currentIndex() == PlotConstant) { createConstant(); }
    else { createVariable(); }
    return m_variable;
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

// TODO: Make this more elegant
void VariableInput::fromJSON(string json)
{
   BlueJSON parser = BlueJSON(json);
    string token;
    double value_double;
    int value_int;

    parser.getNextKeyValue("name", token);
    parser.getStringToken(token);
    ui->lineEdit_Name->setText(QString::fromStdString(token));

    parser.getNextKeyValue("min", token);
    parser.getDoubleToken(value_double);
    ui->lineEdit_Min->setText(QString::number(value_double));

    parser.getNextKeyValue("max", token);
    parser.getDoubleToken(value_double);
    ui->lineEdit_Max->setText(QString::number(value_double));

    parser.getNextKeyValue("elements", token);
    parser.getIntToken(value_int);
    ui->lineEdit_Elements->setText(QString::number(value_int));

    parser.getNextKeyValue("units", token);
    parser.getStringToken(token);
    ui->lineEdit_Units->setText(QString::fromStdString(token));

    parser.getNextKeyValue("slider constant", token);
    parser.getIntToken(value_int);
    sliderCheck();
    ui->horizontalSlider_Point->setValue(value_int);
    value_double = m_variable.min() + value_int*(m_variable.max()-m_variable.min())/m_variable.elements();
    ui->label_Constant->setNum(value_double);

    parser.getNextKeyValue("axis", token);
    parser.getStringToken(token);
    if (token == "x")
        ui->comboBox_Axes->setCurrentIndex(PlotHorizontal);
    if (token == "y")
        ui->comboBox_Axes->setCurrentIndex(PlotVertical);
}

void VariableInput::enableRemoveButton(bool flag_enable)
{
    ui->pushButton_Delete->setEnabled(flag_enable);
}

void VariableInput::setSplitterSizes(QList<int> sizes)
{
    ui->splitter->setSizes(sizes);
}

void VariableInput::formatLineEdit(QLineEdit* edit, QString fg, QString bg){
    QString str = 	"QLineEdit{ color: " + fg + "; background: " + bg + ";}";
    edit->setStyleSheet(str);
}

void VariableInput::formatLineEditNormal(QLineEdit* edit){
    formatLineEdit(edit, DefaultColors[NormalForeground], DefaultColors[NormalBackground]);
}

void VariableInput::formatLineEditError(QLineEdit* edit){
    formatLineEdit(edit, DefaultColors[ErrorForeground], DefaultColors[ErrorBackground]);
}


///	Private Functions
///	==================

void VariableInput::sliderCheck()
{
    resetSlider();
    if (ui->lineEdit_Min->text().isEmpty() || ui->lineEdit_Max->text().isEmpty() || ui->lineEdit_Elements->text().isEmpty())
        return;
    if (!checkInput())
        return;
    createVariable();
    if (ui->comboBox_Axes->currentIndex() == PlotConstant){
       ui->horizontalSlider_Point->setEnabled(true);
       ui->horizontalSlider_Point->setTickInterval(1);
       ui->label_Constant->setNum(m_variable.min());
       ui->horizontalSlider_Point->setMaximum(m_variable.elements());
    }
}

void VariableInput::createVariable()
{
    m_variable = Variable(	ui->lineEdit_Name->text().toStdString(),
                            ui->lineEdit_Min->text().toDouble(),
                            ui->lineEdit_Max->text().toDouble(),
                            ui->lineEdit_Elements->text().toInt());
    flag_initialized = true;
}

void VariableInput::createConstant()
{
    double dSelectedValue = m_variable.min() + ui->horizontalSlider_Point->value()*(m_variable.max()-m_variable.min())/m_variable.elements();
    m_variable = Variable(	ui->lineEdit_Name->text().toStdString(),
                            dSelectedValue,
                            dSelectedValue,
                            1);
}

void VariableInput::resetSlider()
{
       ui->horizontalSlider_Point->setValue(0);
       ui->label_Constant->setText("-");
       ui->horizontalSlider_Point->setEnabled(false);
}


///	Public Slots
/// =============

void VariableInput::splitterResize(QList<int> sizes){
    int tot = 0;
    int tot2 = 0;
    ui->splitter->setSizes(sizes);
    QList<int> var = ui->splitter->sizes();
    for (int i = 0; i < var.size(); i++){
        tot += var.value(i);
        tot2 += sizes.value(i);
    }
}


///	Private Slots
/// ==============

void VariableInput::on_comboBox_Axes_currentIndexChanged(int index)
{
   clearFormatting();
   switch(index) {
   case 0:
        m_axisMode = PlotHorizontal;
        resetSlider();
        emit axisModeChanged(m_guiNumber);
        break;
   case 1:
        m_axisMode = PlotVertical;
        resetSlider();
        emit axisModeChanged(m_guiNumber);
        break;
   case 2:
        m_axisMode = PlotConstant;
        sliderCheck();
        emit axisModeChanged(m_guiNumber);
        break;
   }
}

void VariableInput::on_horizontalSlider_Point_sliderMoved(int position)
{
    createVariable();
    double dSelectedValue = m_variable.min() + position*(m_variable.max()-m_variable.min())/m_variable.elements();
    ui->label_Constant->setNum(dSelectedValue);
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
        formatLineEdit(ui->lineEdit_Name, COLOR_ERROR_FG, COLOR_ERROR_BG);
    } else {
        formatLineEdit(ui->lineEdit_Name, COLOR_DEFAULT_FG, COLOR_DEFAULT_BG);
    }
}

void VariableInput::on_pushButton_Delete_clicked() { emit killThis(m_guiNumber); }
