//	"""""""""""""""""""""""""""""""""
//	"			Includes			"
//	"""""""""""""""""""""""""""""""""

#include "inequalityinput.h"
#include "ui_inequalityinput.h"


//	"""""""""""""""""""""""""""""""""
//	"			Enumerators			"
//	"""""""""""""""""""""""""""""""""


enum COMBINE
{
    NONE 			= 0,
    INTERSECTION 	= 1,
    UNION 			= 2,
    SUBTRACTION 	= 3,

};

//	"""""""""""""""""""""""""""""""""
//	"	  Third Party Functions		"
//	"""""""""""""""""""""""""""""""""

// -- Reference:	Vasaka
// -- link:			http://stackoverflow.com/questions/14417333/how-can-i-change-color-of-part-of-the-text-in-qlineedit
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
}

// -- Reference:	Vasaka
// -- link:			http://stackoverflow.com/questions/14417333/how-can-i-change-color-of-part-of-the-text-in-qlineedit
static void clearLineEditTextFormat(QLineEdit* lineEdit)
{
    setLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""

InequalityInput::InequalityInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityInput)
{
    ui->setupUi(this);
}

InequalityInput::~InequalityInput()
{
    delete ui;
}


//	Setters
//	-------

void InequalityInput::setNumber(int nNumber){
    nInequalityInputNumber = nNumber;
}

void InequalityInput::setXYVariables(Variable mX, Variable mY){
    mVariableX = mX;
    mVariableY = mY;
}

bool InequalityInput::createInequality(){
    // input expressions
    mInequality = Inequality(ui->lineEditLeft->text().toStdString(),
                             ui->comboBoxInequality->currentText().toStdString(),
                             ui->lineEditRight->text().toStdString());

    // remove whitespace
    ui->lineEditLeft->setText(QString::fromStdString(mInequality.getExpressionLHS()));
    ui->lineEditRight->setText(QString::fromStdString(mInequality.getExpressionRHS()));

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


//	Getters
//	-------

int InequalityInput::getNumber(){
    return nInequalityInputNumber;
}

int InequalityInput::getColorIndex(){
    return ui->comboBoxColor->currentIndex();
}

int InequalityInput::getShapeIndex(){
    return ui->comboBoxShape->currentIndex();
}

int InequalityInput::getCombination(){
    return ui->comboBoxInteract->currentIndex();
}

QVector<double> InequalityInput::getX(){
    return qvX;
}

QVector<double> InequalityInput::getY(){
    return qvY;
}

QVector<double> InequalityInput::getXProblem(){
    return qvX_problem;
}

QVector<double> InequalityInput::getYProblem(){
    return qvY_problem;
}

//	Validation
//	----------

bool InequalityInput::highlightInvalidExpressionTerms(){
    // Highlights invalid expression terms, returns 1 if highlighting has been done or 0 if no hightlighting has been done.
    bool flag_highlight = false;
    // check LHS
    if (!mInequality.isValidLHS()){
        flag_highlight = true;
        vector<int> vInputErrorsLHS = mInequality.getProblemElements_ExpressionLHS();
        int nFormatRangeCounter = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;
        for (int nTerm = 0; nTerm < mInequality.getNumTermsLHS(); nTerm++){
            string sTerm = mInequality.getTermLHS(nTerm);
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

        cerr << "[ERROR] BareMinimumPlotter | highlightInvalidExpressionTerms | " << "LHS inequality is invalid" << endl;
    }

    // check RHS
    if (!mInequality.isValidRHS()){
        flag_highlight = true;
        vector<int> vInputErrorsRHS = mInequality.getProblemElements_ExpressionRHS();
        int nFormatRangeCounter = 0;
        QList<QTextLayout::FormatRange> formats;
        QTextCharFormat f;
        QTextLayout::FormatRange fr;
        for (int nTerm = 0; nTerm < mInequality.getNumTermsRHS(); nTerm++){
            string sTerm = mInequality.getTermRHS(nTerm);
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

        cerr << "[ERROR] BareMinimumPlotter | highlightInvalidExpressionTerms | " << "RHS inequality is invalid" << endl;
    }
    return flag_highlight;
}


//	Formatting
//	----------

void InequalityInput::clearFormatting(){
    clearLineEditTextFormat(ui->lineEditLeft);
    clearLineEditTextFormat(ui->lineEditRight);
}


//	Core
//	----

bool InequalityInput::addVariable(Variable mVariable){
    //	check variable unique
    if (!mInequality.variableIsValid(mVariable)) // check for uniqueness
        return false;
    // add variable
    mInequality.addVariable(mVariable);
    return true;
}

bool InequalityInput::evaluate(){
    // do maths
    try{
        vPlotSpace = mInequality.evaluate();
    }
    catch(INPUT_ERROR_CODES e){ // catch errors that happen during evaluation
        switch(e){
        case INPUT_ERROR_INVALID_EXPRESSION:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Invalid expression." << endl;
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        case INPUT_ERROR_UNINITIALIZED_VARIABLE:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Uninitialized variable." << endl;
            if(highlightInvalidExpressionTerms())
                return false;
            break;
        default:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Unhandled INPUT_ERROR_CODE exception caught: ";
            cerr << e << endl;
            return false;
        }
    }

    vProblemSpace = mInequality.getProblemElements_ResultsCombined();

    // create QVectors (for plotting)
    mVariableX.resetPosition();		// reset iterators
    mVariableY.resetPosition();

    qvX.clear();	// clear previous plotting vectors
    qvY.clear();
    qvX_problem.clear();
    qvY_problem.clear();

    vector<int>::iterator it_ProblemSpace = vProblemSpace.begin();
    for(int i = 0; i < static_cast<int>(vPlotSpace.size()); i++){
        if(!vProblemSpace.empty() && i == *it_ProblemSpace){ 	// problem point - add to problem vectors
            qvX_problem.push_back(mVariableX.getCurrentValue());
            qvY_problem.push_back(mVariableY.getCurrentValue());
            it_ProblemSpace++;
        }
        else if (vPlotSpace[i]) { 	// not a problem point - add to the normal graph vectors
            qvX.push_back(mVariableX.getCurrentValue());
            qvY.push_back(mVariableY.getCurrentValue());
        }

        if ((i+1) % mVariableY.getElements() == 0){
            mVariableX.nextPosition();
        }
        mVariableY.nextPosition();
    }
    return true;
}


//	"""""""""""""""""""""""""""""""""
//	"		Private Slots			"
//	"""""""""""""""""""""""""""""""""

void InequalityInput::on_pushButtonUp_clicked()
{
    emit moveUp(nInequalityInputNumber);
}

void InequalityInput::on_pushButtonDown_clicked()
{
    emit moveDown(nInequalityInputNumber);
}

void InequalityInput::on_pushButtonDelete_clicked()
{
    emit killThis(nInequalityInputNumber);
}

void InequalityInput::on_lineEditLeft_textChanged(const QString&)
{
   clearLineEditTextFormat(ui->lineEditLeft);
}

void InequalityInput::on_lineEditRight_textChanged(const QString&)
{
   clearLineEditTextFormat(ui->lineEditRight);
}
