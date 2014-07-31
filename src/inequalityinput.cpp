///	Includes
///	=========

#include "include/inequalityinput.h"
#include "ui_inequalityinput.h"


///	Third Party Functions
///	======================

static void setLineEditTextFormat(QLineEdit* lineEdit, const QList<QTextLayout::FormatRange>& formats)
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

static void clearLineEditTextFormat(QLineEdit* lineEdit)
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
    m_error_message(""),
    m_bugmail("emerick.bosch+bugmail@gmail.com")
{
    ui->setupUi(this);
    setAccessibleDescription("input");

    //	set input validation
    QDoubleValidator *dValidator = new QDoubleValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEdit_Precision->setValidator(dValidator);

    //	initialize precision line edit
    m_precisionIndex = ui->splitter_Inequality->indexOf(ui->lineEdit_Precision);
    on_comboBox_Inequality_currentIndexChanged(0);

    //	splitters
    for(int i = 0; i < ui->splitter_InequalityInput->count(); i++){
        ui->splitter_InequalityInput->handle(i)->setEnabled(false);
    }
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
    m_VariableX = mX;
    m_VariableY = mY;
}

void InequalityInput::setX(QVector<double> vX){ m_X = vX; }

void InequalityInput::setY(QVector<double> vY){ m_Y = vY; }

bool InequalityInput::createInequality(){
    m_error_message = "";
    // input expressions
    m_Inequality = Inequality(ui->lineEdit_Left->text().toStdString(),
                             getSymbol(),
                             ui->lineEdit_Right->text().toStdString());
    m_Inequality.setPrecision(atof(ui->lineEdit_Precision->text().toStdString().c_str()));
    // remove whitespace
    ui->lineEdit_Left->setText(QString::fromStdString(m_Inequality.getExpressionLHS()));
    ui->lineEdit_Right->setText(QString::fromStdString(m_Inequality.getExpressionRHS()));
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
    stringstream buffer;
    buffer <<	"\"inequality\":{" <<
                "\"left expression\":" << "\""<< getLeftExpression() << "\"," <<
                "\"symbol\":" << "\"" << ui->comboBox_Inequality->currentText().toStdString() << "\",";
    if (ui->comboBox_Inequality->currentIndex() == ApproxEqual){
        buffer << "\"precision\":"	<< ui->lineEdit_Precision->text().toStdString() << ",";
    }
    buffer <<  	"\"right expression\":" << "\"" << getRightExpression() << "\"," <<
                "\"combination\":" << "\"" <<getCombination() <<
                "\"}";

    return buffer.str();
}

string InequalityInput::dataToJSON(){
    stringstream buffer;
    buffer << "\"data\":[";
    for (int j = 0; j < m_X.size(); j++){
        buffer << "{"
                   "\"x\":"<< m_X[j] << ","
                   "\"y\":" << m_Y[j] <<
                   "}";
        if (j != m_X.size()-1){
            buffer << ",";
            buffer << "\n";
        }
    }
    buffer << "]\n";
    return buffer.str();
}

string InequalityInput::problemDataToJSON(){
    stringstream buffer;
    buffer << "\"problem data\":[";

    for (int j = 0; j < m_XProblem.size(); j++){
        buffer << "{"
                   "\"x\":"<< m_XProblem[j] << ","
                   "\"y\":" << m_YProblem[j] <<
                   "}";

        if (j != m_XProblem.size()-1){
            buffer << ",";
            buffer << "\n";
        }
    }

    buffer << "]\n";
    return buffer.str();
}

void InequalityInput::fromJSON(string sInput){
    string token;
    stringstream iss;
    iss << sInput;
    while (getline (iss, token, '"')){
        if (token == "left expression")	{
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"'))
                    ui->lineEdit_Left->setText(QString::fromStdString(token));
        } else if (token == "symbol") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"')){
                    InequalitySymbol symbol;
                    if (token == "<")
                        symbol = SmallerThan;
                    if (token == ">")
                        symbol = GreaterThan;
                    if (token == "<=")
                        symbol = SmallerThanEqual;
                    if (token == ">=")
                        symbol = GreaterThanEqual;
                    if (token == "≈")
                        symbol = ApproxEqual;
                    ui->comboBox_Inequality->setCurrentIndex(symbol);
                }
        } else if (token == "right expression") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"'))
                    ui->lineEdit_Right->setText(QString::fromStdString(token));

        } else if (token == "combination") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"')){
                    stringstream ss;
                    int index;
                    ss << token;
                    if(!(ss >> index))
                        index = 0;
                    ui->comboBox_Interact->setCurrentIndex(index);
                }
        }
    }
}


//	Getters: UI
//	------------

int InequalityInput::getNumber(){ return m_guiNumber; }

int InequalityInput::getShapeIndex(){ return ui->comboBox_Shape->currentIndex(); }

int InequalityInput::getCombination(){ return ui->comboBox_Interact->currentIndex(); }

double InequalityInput::getPrecision() { return atof(ui->lineEdit_Precision->text().toStdString().c_str()); }

bool InequalityInput::getSkip(){ return flag_skip; }

string InequalityInput::getLeftExpression(){ return m_Inequality.getExpressionLHS(); }

string InequalityInput::getRightExpression(){ return m_Inequality.getExpressionRHS(); }

QWidget* InequalityInput::getFocusInWidget() { return ui->lineEdit_Left; }

QWidget* InequalityInput::getFocusOutWidget()
{
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
    }
    return QwtSymbol::NoSymbol;
}


//	Getters: Data
//	--------------

string InequalityInput::getErrors(){ return m_error_message + m_Inequality.getErrors(); }

QVector<double> InequalityInput::getX(){ return m_X; }

QVector<double> InequalityInput::getY(){ return m_Y; }

QVector<double> InequalityInput::getXProblem(){ return m_XProblem; }

QVector<double> InequalityInput::getYProblem(){ return m_YProblem; }


//	Validation
//	----------

bool InequalityInput::highlightInvalidExpressionTerms()
{
    // Highlights invalid expression terms, returns 1 if highlighting has been done or 0 if no hightlighting has been done.
    bool flag_highlight = false;
    // check LHS
    if (!m_Inequality.isValidLHS()){
        flag_highlight = true;
        vector<int> vInputErrorsLHS = m_Inequality.getProblemElements_ExpressionLHS();
        int nFormatRangeCounter = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;
        for (int nTerm = 0; nTerm < m_Inequality.getNumTermsLHS(); nTerm++){
            string sTerm = m_Inequality.getTermLHS(nTerm);
            f.setFontWeight(QFont::Normal);
            f.setForeground(QBrush(Qt::black));
            fr.start = nFormatRangeCounter;
            fr.length = sTerm.length();
            fr.format = f;
            for (int i = 0; i < static_cast<int>(vInputErrorsLHS.size()); i++){
                if (nTerm == vInputErrorsLHS[i]){
                    f.setFontWeight(QFont::Bold);
                    f.setForeground(QBrush(Qt::red));
                    fr.format = f;
                }
            }
            nFormatRangeCounter += sTerm.length();
            formats.append(fr);
        }
        setLineEditTextFormat(ui->lineEdit_Left, formats);
    }

    // check RHS
    if (!m_Inequality.isValidRHS()){
        flag_highlight = true;
        vector<int> vInputErrorsRHS = m_Inequality.getProblemElements_ExpressionRHS();
        int nFormatRangeCounter = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;
        for (int nTerm = 0; nTerm < m_Inequality.getNumTermsRHS(); nTerm++){
            string sTerm = m_Inequality.getTermRHS(nTerm);
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
    if (!m_Inequality.variableIsValid(variable))  {
        return false;
    }
    // add variable
    m_Inequality.addVariable(variable);
    return true;
}

bool InequalityInput::evaluate(){
    // do maths
    try{
        m_plotSpace = m_Inequality.evaluate();
    }
    catch(INPUT_ERROR_CODES e){ // catch errors that happen during evaluation
        switch(e){
        case InputErrorInvalidExpression:
            m_error_message += "Input | Invalid expression.\n";
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        case InputErrorUninitializedVariable:
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        default:
            m_error_message += "Bug | Unhandled input error. Please report this bug to the following email address:\n" + m_bugmail + "\n";
            cerr << e << endl;
            return false;
        }
    }

    m_problemSpace = m_Inequality.getProblemElements_ResultsCombined();

    // create QVectors (for plotting)
    m_VariableX.resetPosition();		// reset iterators
    m_VariableY.resetPosition();

    m_X.clear();	// clear previous plotting vectors
    m_Y.clear();
    m_XProblem.clear();
    m_YProblem.clear();

    bool flag_XBeforeY = m_Inequality.getXBeforeY(m_VariableX, m_VariableY);

    vector<int>::iterator it_ProblemSpace = m_problemSpace.begin();
    for(int i = 0; i < static_cast<int>(m_plotSpace.size()); i++){
        if(!m_problemSpace.empty() && i == *it_ProblemSpace){ 	// problem point - add to problem vectors
            m_XProblem.push_back(m_VariableX.getCurrentValue());
            m_YProblem.push_back(m_VariableY.getCurrentValue());
            it_ProblemSpace++;
        }
        else if (m_plotSpace[i]) { 	// not a problem point - add to the normal graph vectors
            m_X.push_back(m_VariableX.getCurrentValue());
            m_Y.push_back(m_VariableY.getCurrentValue());
        }

        //	TODO: XBeforeY more elegantly
        if (flag_XBeforeY){ 		// accounts for the fact that evaluation happens in the order in which variables were *added*
            if ((i+1) % m_VariableY.elements() == 0){
                m_VariableX.nextPosition();
            }
            m_VariableY.nextPosition();
        } else {
            if ((i+1) % m_VariableX.elements() == 0){
                m_VariableY.nextPosition();
            }
            m_VariableX.nextPosition();
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
