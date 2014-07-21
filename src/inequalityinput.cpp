//	"""""""""""""""""""""""""""""""""
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""

#include "include/inequalityinput.h"
#include "ui_inequalityinput.h"


//	"""""""""""""""""""""""""""""""""
//	"	  Third Party Functions		"
//	"""""""""""""""""""""""""""""""""

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


//	"""""""""""""""""""""""""""""""""
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""

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
    m_precisionIndex = ui->horizontalLayout->indexOf(ui->lineEdit_Precision);
    on_comboBoxInequality_currentIndexChanged(0);
    //	center text in comboboxes
    setStyleSheet( 	"QComboBox:!editable, QLabel, QListView, QPushButton{"
                        "text-align: center;"
                        "font: bold;"
                    "}"
                  );

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
    m_Inequality = Inequality(ui->lineEditLeft->text().toStdString(),
                             ui->comboBoxInequality->currentIndex(),
                             ui->lineEditRight->text().toStdString());
    m_Inequality.setPrecision(atof(ui->lineEdit_Precision->text().toStdString().c_str()));
    // remove whitespace
    ui->lineEditLeft->setText(QString::fromStdString(m_Inequality.getExpressionLHS()));
    ui->lineEditRight->setText(QString::fromStdString(m_Inequality.getExpressionRHS()));
    //check expression
    if(highlightInvalidExpressionTerms())
        return false;
    return true;
}

void InequalityInput::enablePositionButtons(bool flag_enable){
    ui->pushButtonDown->setEnabled(flag_enable);
    ui->pushButtonUp->setEnabled(flag_enable);
    ui->comboBoxInteract->setEnabled(flag_enable);
    ui->pushButtonDelete->setEnabled(flag_enable);
}

void InequalityInput::enableCombinations(bool flag_enable){ ui->comboBoxInteract->setEnabled(flag_enable); }

void InequalityInput::resetCombinations(){ ui->comboBoxInteract->setCurrentIndex(CombinationNone); }



//	Parsers
//	-------

string InequalityInput::expressionToJSON() {
    stringstream buffer;
    buffer <<	"\"inequality\":{" <<
                "\"left expression\":" << "\""<< getLeftExpression() << "\"," <<
                "\"symbol\":" << "\"" << ui->comboBoxInequality->currentText().toStdString() << "\",";
    if (ui->comboBoxInequality->currentIndex() == ApproxEqual){
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
    // close plot data
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
                    ui->lineEditLeft->setText(QString::fromStdString(token));
        } else if (token == "symbol") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"')){
                    int symbol;
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
                    ui->comboBoxInequality->setCurrentIndex(symbol);
                }
        } else if (token == "right expression") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"'))
                    ui->lineEditRight->setText(QString::fromStdString(token));

        } else if (token == "combination") {
            if (getline (iss, token, '"'))
                if (getline (iss, token, '"')){
                    stringstream ss;
                    int index;
                    ss << token;
                    if(!(ss >> index))
                        index = 0;
                    ui->comboBoxInteract->setCurrentIndex(index);
                }
        }
    }
}


//	Getters
//	-------

int InequalityInput::getNumber(){ return m_guiNumber; }

int InequalityInput::getColorIndex(){ return ui->comboBoxColor->currentIndex(); }

int InequalityInput::getShapeIndex(){ return ui->comboBoxShape->currentIndex(); }

int InequalityInput::getCombination(){ return ui->comboBoxInteract->currentIndex(); }

double InequalityInput::getPrecision() { return atof(ui->lineEdit_Precision->text().toStdString().c_str()); }

bool InequalityInput::getSkip(){ return flag_skip; }

string InequalityInput::getLeftExpression(){ return m_Inequality.getExpressionLHS(); }

string InequalityInput::getRightExpression(){ return m_Inequality.getExpressionRHS(); }

string InequalityInput::getErrors(){ return m_error_message + m_Inequality.getErrors(); }

QVector<double> InequalityInput::getX(){ return m_X; }

QVector<double> InequalityInput::getY(){ return m_Y; }

QVector<double> InequalityInput::getXProblem(){ return m_XProblem; }

QVector<double> InequalityInput::getYProblem(){ return m_YProblem; }

QWidget* InequalityInput::getFocusInWidget() { return ui->lineEditLeft; }

QWidget* InequalityInput::getFocusOutWidget()
{
    QWidget::setTabOrder(ui->lineEditLeft, ui->comboBoxInequality);
    QWidget::setTabOrder(ui->comboBoxInequality, ui->lineEditRight);
    if (ui->comboBoxInequality->currentIndex() == ApproxEqual){
        QWidget::setTabOrder(ui->lineEditRight, ui->lineEdit_Precision);
        return ui->lineEdit_Precision;
    } else {
        return ui->lineEditRight;
    }
}

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
        setLineEditTextFormat(ui->lineEditLeft, formats);
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

        setLineEditTextFormat(ui->lineEditRight, formats);
    }
    return flag_highlight;
}


//	Formatting
//	----------

void InequalityInput::clearFormatting(){
    clearLineEditTextFormat(ui->lineEditLeft);
    clearLineEditTextFormat(ui->lineEditRight);
}

void InequalityInput::clearFields(){
    ui->lineEditLeft->clear();
    ui->lineEditRight->clear();
    ui->comboBoxInequality->setCurrentIndex(0);
    ui->comboBoxShape->setCurrentIndex(0);
    ui->comboBoxColor->setCurrentIndex(0);
    ui->comboBoxInteract->setCurrentIndex(0);
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
            if ((i+1) % m_VariableY.getElements() == 0){
                m_VariableX.nextPosition();
            }
            m_VariableY.nextPosition();
        } else {
            if ((i+1) % m_VariableX.getElements() == 0){
                m_VariableY.nextPosition();
            }
            m_VariableX.nextPosition();
        }
    }
    return true;
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Slots			"
//	"""""""""""""""""""""""""""""""""

void InequalityInput::splitterResize(QList<int> sizes){
    ui->splitter->setSizes(sizes);
}


//	"""""""""""""""""""""""""""""""""
//	"		Private Slots			"
//	"""""""""""""""""""""""""""""""""

void InequalityInput::on_pushButtonUp_clicked() { emit moveUp(m_guiNumber); }

void InequalityInput::on_pushButtonDown_clicked() { emit moveDown(m_guiNumber); }

void InequalityInput::on_pushButtonDelete_clicked() { emit killThis(m_guiNumber); }

void InequalityInput::on_lineEditLeft_textChanged(const QString&) { clearLineEditTextFormat(ui->lineEditLeft); }

void InequalityInput::on_lineEditRight_textChanged(const QString&) { clearLineEditTextFormat(ui->lineEditRight); }

void InequalityInput::on_comboBoxInteract_currentIndexChanged(int index)
{
   switch (index) {
    case CombinationNone:
       ui->comboBoxColor->setEnabled(true);
       ui->comboBoxShape->setEnabled(true);
       break;
   default:
       ui->comboBoxColor->setEnabled(false);
       ui->comboBoxShape->setEnabled(false);
       break;
   }
}

void InequalityInput::on_checkBoxSkip_toggled(bool checked)
{
   if(checked) {
       flag_skip = true;
       ui->comboBoxColor->setEnabled(false);
       ui->comboBoxShape->setEnabled(false);
       ui->lineEditLeft->setEnabled(false);
       ui->lineEditRight->setEnabled(false);
       ui->comboBoxInequality->setEnabled(false);
       ui->comboBoxInteract->setEnabled(false);

   }
   else {
       flag_skip = false;
       ui->comboBoxColor->setEnabled(true);
       ui->comboBoxShape->setEnabled(true);
       ui->lineEditLeft->setEnabled(true);
       ui->lineEditRight->setEnabled(true);
       ui->comboBoxInequality->setEnabled(true);
       if(ui->pushButtonDown->isEnabled())
           ui->comboBoxInteract->setEnabled(true);
   }
}

void InequalityInput::on_comboBoxInequality_currentIndexChanged(int index)
{
    if (index == ApproxEqual){
        ui->horizontalLayout->insertWidget(m_precisionIndex, ui->lineEdit_Precision);
        ui->lineEdit_Precision->setEnabled(true);
        ui->lineEdit_Precision->setVisible(true);
    }
    else{
        if (ui->lineEdit_Precision->isEnabled()){
            ui->horizontalLayout->takeAt(m_precisionIndex);
            ui->lineEdit_Precision->setEnabled(false);
            ui->lineEdit_Precision->setVisible(false);
        }
    }
}
