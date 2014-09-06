///	Includes
///	=========

#include "include/inequalityinput.h"
#include "ui_inequalityinput.h"


///	Third Party Functions
///	======================

void InequalityInput::setLineEditTextFormat(QLineEdit* lineEdit, const QList<QTextLayout::FormatRange>& formats)
{
    if(!lineEdit)
        return;

    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const QTextLayout::FormatRange& fr, formats)
    {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }
    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);

    // -- Reference:	Vasaka
    // -- link:			http://stackoverflow.com/questions/14417333/how-can-i-change-color-of-part-of-the-text-in-qlineedit
}

void InequalityInput::clearLineEditTextFormat(QLineEdit* lineEdit)
{
    setLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());

    // -- Reference:	Vasaka
    // -- link:			http://stackoverflow.com/questions/14417333/how-can-i-change-color-of-part-of-the-text-in-qlineedit
}


///	Public Functions
/// =================

InequalityInput::InequalityInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityInput),
    flag_skip(false),
    m_errorMessage(""),
    m_bugMail("emerick.bosch+bugmail@gmail.com")
{
    ui->setupUi(this);
    setAccessibleDescription("input");

    //	set input validation
    QDoubleValidator *dValidator = new QDoubleValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEdit_Precision->setValidator(dValidator);

    //	initialize tolerance line edit
    m_precisionIndex = ui->splitter_Inequality->indexOf(ui->lineEdit_Precision);
    on_comboBox_Inequality_currentIndexChanged(0);

    //	splitters
    for(int i = 0; i < ui->splitter_InequalityInput->count(); i++){
        ui->splitter_InequalityInput->handle(i)->setEnabled(false);
    }

    cancelFlagLeft = new bool(false);
    cancelFlagRight = new bool(false);
}

InequalityInput::~InequalityInput()
{
    delete ui;
}


//	Setters
//	-------

void InequalityInput::setNumber(int nNumber){
    m_guiNumber = nNumber;
    ui->label_Index->setNum(nNumber+1);
}

void InequalityInput::setXYVariables(Variable mX, Variable mY){
    m_xVariable = mX;
    m_yVariable = mY;
}

void InequalityInput::setX(QVector<double> vX){ m_x = vX; }

void InequalityInput::setY(QVector<double> vY){ m_y = vY; }

bool InequalityInput::createInequality(){
    m_errorMessage = "";

    m_name = ui->lineEdit_Name->text();

    // input expressions
    m_inequality = Inequality(ui->lineEdit_Left->text().toStdString(),
                             getSymbol(),
                             ui->lineEdit_Right->text().toStdString());
    m_inequality.setPrecision(atof(ui->lineEdit_Precision->text().toStdString().c_str()));

    cancelFlagLeft = m_inequality.cancelFlagLeft;
    cancelFlagRight = m_inequality.cancelFlagRight;

    // remove whitespace
    ui->lineEdit_Left->setText(QString::fromStdString(m_inequality.getExpressionLHS()));
    ui->lineEdit_Right->setText(QString::fromStdString(m_inequality.getExpressionRHS()));

    //check expression
    if(highlightInvalidExpressionTerms())
        return false;
    return true;
}

void InequalityInput::enablePositionButtons(bool flag_enable){
    ui->pushButton_Down->setEnabled(flag_enable);
    ui->pushButton_Up->setEnabled(flag_enable);
    ui->comboBox_Interact->setEnabled(flag_enable);
    ui->pushButton_Delete->setEnabled(flag_enable);
}

void InequalityInput::enableCombinations(bool flag_enable){ ui->comboBox_Interact->setEnabled(flag_enable); }

void InequalityInput::resetCombinations(){ ui->comboBox_Interact->setCurrentIndex(CombinationNone); }



//	Parsers
//	-------

string InequalityInput::expressionToJSON() {

    // object properties
    vector<string> properties;
    properties.push_back( BlueJSON::jsonKeyValue("description", getName().toStdString()) );
    properties.push_back( BlueJSON::jsonKeyValue("left expression", getLeftExpression().toStdString()) );
    properties.push_back( BlueJSON::jsonKeyValue("symbol", ui->comboBox_Inequality->currentText().toStdString()) );
    if (ui->comboBox_Inequality->currentIndex() == ApproxEqual)
        properties.push_back( BlueJSON::jsonKeyValue("precision", ui->lineEdit_Precision->text().toStdString()) );
    properties.push_back( BlueJSON::jsonKeyValue("right expression", getRightExpression().toStdString()) );
    properties.push_back( BlueJSON::jsonKeyValue("combination", s_combinations[getCombination()]) );

    // create & return object
    string json = BlueJSON::jsonObject(properties, Flat);
    json = BlueJSON::jsonKeyValue("inequality", json);

    return json;
}

string InequalityInput::dataToJSON(){
    vector<string> data_array;

    // create point objects
    for (int j = 0; j < m_x.size(); j++){
        vector<string> point;
        point.push_back( BlueJSON::jsonKeyValue("x", m_x[j]) );
        point.push_back( BlueJSON::jsonKeyValue("y", m_y[j]) );
        data_array.push_back( BlueJSON::jsonObject(point, Flat));
    }

    // create & return data array
    string json = BlueJSON::jsonArray(data_array);
    json = BlueJSON::jsonKeyValue("data", json);
    return json;
}

string InequalityInput::problemDataToJSON()
{
    vector<string> data_array;

    // create point objects
    for (int j = 0; j < m_x.size(); j++){
        vector<string> point;
        point.push_back( BlueJSON::jsonKeyValue("x", m_x_problem[j]) );
        point.push_back( BlueJSON::jsonKeyValue("y", m_y_problem[j]) );
        data_array.push_back( BlueJSON::jsonObject(point, Flat));
    }

    // create & return data array
    string json = BlueJSON::jsonArray(data_array);
    json = BlueJSON::jsonKeyValue("problem data", json);
    return json;
}

void InequalityInput::fromJSON(string json)
{
    BlueJSON parser = BlueJSON(json);

    string desc, leftexp, symbol, rightexp, comb;
    parser.getNextKeyValue("description", desc);
    parser.getNextKeyValue("left expression", leftexp);
    parser.getNextKeyValue("symbol", symbol);
    parser.getNextKeyValue("right expression", rightexp);
    parser.getNextKeyValue("combination", comb);

    parser.getStringToken(desc);
    parser.getStringToken(leftexp);
    parser.getStringToken(symbol);
    parser.getStringToken(rightexp);
    parser.getStringToken(comb);

    ui->lineEdit_Name->setText(QString::fromStdString(desc));
    ui->lineEdit_Left->setText(QString::fromStdString(leftexp));
    ui->lineEdit_Right->setText(QString::fromStdString(rightexp));

    ui->comboBox_Inequality->setCurrentIndex(symbolFromString(symbol));

    int match = 0;
    int options = sizeof(s_combinations)/sizeof(s_combinations[0]);

    for (int i = 0; i < options; i++){
        if (comb == s_combinations[i]) match = i;
    }

    ui->comboBox_Interact->setCurrentIndex(match);

    return;
}


InequalitySymbol InequalityInput::symbolFromString(string value)
{
    if (value == "<")
        return SmallerThan;
    if (value == ">")
        return GreaterThan;
    if (value == "<=")
        return SmallerThanEqual;
    if (value == ">=")
        return GreaterThanEqual;
    if (value == "≈")
        return ApproxEqual;

    // default
    return SmallerThan;
}


//	Getters: UI
//	------------

QString InequalityInput::getName(){ return m_name; }

int InequalityInput::getNumber(){ return m_guiNumber; }

int InequalityInput::getShapeIndex(){ return ui->comboBox_Shape->currentIndex(); }

int InequalityInput::getCombination(){ return ui->comboBox_Interact->currentIndex(); }

double InequalityInput::getPrecision() { return atof(ui->lineEdit_Precision->text().toStdString().c_str()); }

bool InequalityInput::getSkip(){
    bool flag_empty = 	ui->lineEdit_Left->text().isEmpty() ||
                        ui->lineEdit_Right->text().isEmpty();
    if (flag_empty)
        m_errorMessage += "Input Warning | Inequality | Empty fields, skipping.\n";

    return flag_skip || flag_empty;
}

QString InequalityInput::getLeftExpression(){ return QString::fromStdString( m_inequality.getExpressionLHS() ); }

QString InequalityInput::getRightExpression(){ return QString::fromStdString( m_inequality.getExpressionRHS() ); }

QString InequalityInput::getErrors()
{
    QString errors = m_errorMessage + QString::fromStdString(m_inequality.getErrors());
    return errors.isEmpty() ? "" : "Inequality " + QString::number(getNumber() + 1) + ": " + errors;

}

QWidget* InequalityInput::getFocusInWidget() { return ui->lineEdit_Name; }

QWidget* InequalityInput::getFocusOutWidget()
{
    QWidget::setTabOrder(ui->lineEdit_Name, ui->lineEdit_Left);
    QWidget::setTabOrder(ui->lineEdit_Left, ui->comboBox_Inequality);
    QWidget::setTabOrder(ui->comboBox_Inequality, ui->lineEdit_Right);

    if (ui->comboBox_Inequality->currentIndex() == ApproxEqual){
        QWidget::setTabOrder(ui->lineEdit_Right, ui->lineEdit_Precision);
        return ui->lineEdit_Precision;
    } else {
        return ui->lineEdit_Right;
    }
}


QColor InequalityInput::getColor()
{
        switch(ui->comboBox_Color->currentIndex()){
        case Red:
            return QColor(Qt::red);
            break;
        case Green:
            return QColor(Qt::green);
            break;
        case Blue:
            return QColor(Qt::blue);
            break;
        case DarkRed:
            return QColor(Qt::darkRed);
            break;
        case DarkGreen:
            return QColor(Qt::darkGreen);
            break;
        case DarkBlue:
            return QColor(Qt::darkBlue);
            break;
        case Grey:
            return QColor(Qt::lightGray);
            break;
        case Black:
            return QColor(Qt::black);
            break;
        }
        return QColor(Qt::transparent);
}

InequalitySymbol InequalityInput::getSymbol()
{
    switch(ui->comboBox_Inequality->currentIndex()){
    case SmallerThan:
        return SmallerThan;
        break;
    case GreaterThan:
        return GreaterThan;
        break;
    case GreaterThanEqual:
        return GreaterThanEqual;
        break;
    case SmallerThanEqual:
        return SmallerThanEqual;
        break;
    case ApproxEqual:
        return ApproxEqual;
        break;
    }
    return NoSymbol;
}

QwtSymbol::Style InequalityInput::getShape()
{
    switch(ui->comboBox_Shape->currentIndex()){
    case CrossX:
        return QwtSymbol::XCross;
        break;
    case CrossPlus:
        return QwtSymbol::Cross;
        break;
    case Circle:
        return QwtSymbol::Ellipse;
        break;
    case UpTriangle:
        return QwtSymbol::Triangle;
        break;
    case DownTriangle:
        return QwtSymbol::DTriangle;
        break;
    case Square:
        return QwtSymbol::Rect;
        break;
    case Diamond:
        return QwtSymbol::Diamond;
        break;
    case Dots:
        // Dots is a line style, not a symbol,
        // must send a signal to change line style
        return QwtSymbol::UserStyle;
        break;
    }
    return QwtSymbol::NoSymbol;
}


//	Getters: Data
//	--------------


QVector<double> InequalityInput::getX(){ return m_x; }

QVector<double> InequalityInput::getY(){ return m_y; }

QVector<double> InequalityInput::getXProblem(){ return m_x_problem; }

QVector<double> InequalityInput::getYProblem(){ return m_y_problem; }


//	Validation
//	----------

bool InequalityInput::highlightInvalidExpressionTerms()
{
    // Highlights invalid expression terms, returns 1 if highlighting has been done or 0 if no hightlighting has been done.
    bool flag_highlight = false;
    // check LHS
    if (!m_inequality.isValidLHS()){
        flag_highlight = true;
        vector<int> errors = m_inequality.getProblemElements_ExpressionLHS();
        int pos = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;

        // entire string must be formatted
        for (int j = 0; j < m_inequality.getNumTermsLHS(); j++){
            string term = m_inequality.getTermLHS(j);
            f.setFontWeight(QFont::Normal);
            f.setForeground(QBrush(Qt::black));
            fr.start = pos;
            fr.length = term.length();
            fr.format = f;
            for (int i = 0; i < static_cast<int>(errors.size()); i++){
                if (j == errors[i]){
                    f.setFontWeight(QFont::Bold);
                    f.setForeground(QBrush(Qt::red));
                    fr.format = f;
                }
            }
            pos += term.length();
            formats.append(fr);
        }
        setLineEditTextFormat(ui->lineEdit_Left, formats);
    }

    // check RHS
    if (!m_inequality.isValidRHS()){
        flag_highlight = true;
        vector<int> vInputErrorsRHS = m_inequality.getProblemElements_ExpressionRHS();
        int nFormatRangeCounter = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;
        for (int nTerm = 0; nTerm < m_inequality.getNumTermsRHS(); nTerm++){
            string sTerm = m_inequality.getTermRHS(nTerm);
            f.setFontWeight(QFont::Normal);
            f.setForeground(QBrush(Qt::black));
            fr.start = nFormatRangeCounter;
            fr.length = sTerm.length();
            fr.format = f;
            for (int i = 0; i < static_cast<int>(vInputErrorsRHS.size()); i++){
                if (nTerm == vInputErrorsRHS[i]){
                    f.setFontWeight(QFont::Bold);
                    f.setForeground(QBrush(Qt::red));
                    fr.format = f;
                }
            }
            nFormatRangeCounter += sTerm.length();
            formats.append(fr);
        }

        setLineEditTextFormat(ui->lineEdit_Right, formats);
    }
    return flag_highlight;
}

bool InequalityInput::checkVariablesInit() { return m_inequality.variablesInit(); }


//	Formatting
//	----------

void InequalityInput::clearFormatting(){
    clearLineEditTextFormat(ui->lineEdit_Left);
    clearLineEditTextFormat(ui->lineEdit_Right);
}

void InequalityInput::clearFields(){
    ui->lineEdit_Left->clear();
    ui->lineEdit_Right->clear();
    ui->comboBox_Inequality->setCurrentIndex(0);
    ui->comboBox_Shape->setCurrentIndex(0);
    ui->comboBox_Color->setCurrentIndex(0);
    ui->comboBox_Interact->setCurrentIndex(0);
}


//	Core
//	----

bool InequalityInput::addVariable(Variable variable){
    //	check variable unique
    if (!m_inequality.variableIsValid(variable))  {
        return false;
    }
    // add variable
    m_inequality.addVariable(variable);
    return true;
}

bool InequalityInput::evaluate(){
    // do maths
    try{
        m_points = m_inequality.evaluate();
    }
    catch(InputErrorCode e){ // catch errors that happen during evaluation
        switch(e){
        case InputErrorInvalidExpression:
            m_errorMessage += "Input | Invalid expression.\n";
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        case InputErrorUninitializedVariable:
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        default:
            m_errorMessage += 	"Bug | Unhandled input error. Please report this bug to the following email address:\n" +
                                m_bugMail + "\n";
            return false;
        }
    }

    m_points_problem = m_inequality.getProblemElements_ResultsCombined();

    // create QVectors (for plotting)
    m_xVariable.resetPosition();		// reset iterators
    m_yVariable.resetPosition();

    m_x.clear();	// clear previous plotting vectors
    m_y.clear();
    m_x_problem.clear();
    m_y_problem.clear();

    bool flag_XBeforeY = m_inequality.getXBeforeY(m_xVariable, m_yVariable);

    vector<int>::iterator it_ProblemSpace = m_points_problem.begin();
    for(int i = 0; i < static_cast<int>(m_points.size()); i++){
        if(!m_points_problem.empty() && i == *it_ProblemSpace){ 	// problem point - add to problem vectors
            m_x_problem.push_back(m_xVariable.currentValue());
            m_y_problem.push_back(m_yVariable.currentValue());
            it_ProblemSpace++;
        }
        else if (m_points[i]) { 	// not a problem point - add to the normal graph vectors
            m_x.push_back(m_xVariable.currentValue());
            m_y.push_back(m_yVariable.currentValue());
        }

        //	TODO: XBeforeY more elegantly
        if (flag_XBeforeY){ 		// accounts for the fact that evaluation happens in the order in which variables were *added*
            if ((i+1) % m_yVariable.elements() == 0){
                m_xVariable.nextPosition();
            }
            m_yVariable.nextPosition();
        } else {
            if ((i+1) % m_xVariable.elements() == 0){
                m_yVariable.nextPosition();
            }
            m_xVariable.nextPosition();
        }
    }
    return true;
}


///	Public Slots
///	=============

void InequalityInput::splitterResize(QList<int> sizes){ ui->splitter_InequalityInput->setSizes(sizes); }


///	Private Slots
///	==============

void InequalityInput::on_pushButton_Up_clicked() { emit moveUp(m_guiNumber); }

void InequalityInput::on_pushButton_Down_clicked() { emit moveDown(m_guiNumber); }

void InequalityInput::on_pushButton_Delete_clicked() { emit killThis(m_guiNumber); }

void InequalityInput::on_lineEdit_Left_textChanged(const QString&) { clearLineEditTextFormat(ui->lineEdit_Left); }

void InequalityInput::on_lineEdit_Right_textChanged(const QString&) { clearLineEditTextFormat(ui->lineEdit_Right); }

void InequalityInput::on_comboBox_Interact_currentIndexChanged(int index)
{
   switch (index) {
    case CombinationNone:
       ui->comboBox_Color->setEnabled(true);
       ui->comboBox_Shape->setEnabled(true);
       break;
   default:
       ui->comboBox_Color->setEnabled(false);
       ui->comboBox_Shape->setEnabled(false);
       break;
   }
}

void InequalityInput::on_checkBox_Skip_toggled(bool checked)
{
   if(checked) {
       flag_skip = true;
       ui->comboBox_Color->setEnabled(false);
       ui->comboBox_Shape->setEnabled(false);
       ui->lineEdit_Left->setEnabled(false);
       ui->lineEdit_Right->setEnabled(false);
       ui->comboBox_Inequality->setEnabled(false);
       ui->comboBox_Interact->setEnabled(false);

   }
   else {
       flag_skip = false;
       ui->comboBox_Color->setEnabled(true);
       ui->comboBox_Shape->setEnabled(true);
       ui->lineEdit_Left->setEnabled(true);
       ui->lineEdit_Right->setEnabled(true);
       ui->comboBox_Inequality->setEnabled(true);
       if(ui->pushButton_Down->isEnabled())
           ui->comboBox_Interact->setEnabled(true);
   }
}

void InequalityInput::on_comboBox_Inequality_currentIndexChanged(int index)
{
    if (index == ApproxEqual){
        ui->splitter_Inequality->widget(m_precisionIndex)->show();
    }
    else if (ui->lineEdit_Precision->isEnabled()){
            ui->splitter_Inequality->widget(m_precisionIndex)->hide();
    }
}
