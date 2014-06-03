/*	TASK LIST
    ---------
    BREAK: 2 June 2014 | adding color and shape selection to inequality
*/


//	"""""""""""""""""""""""""""""""""
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""

#include "bareminimumplotter.h"
#include "ui_bareminimumplotter.h"
#include "deplot/include/inequality.h"
#include <qvalidator.h>
#include <qlocale.h>
#include <stdlib.h>
#include <string>
#include <vector>

//	"""""""""""""""""""""""""""""""""
//	"		Enumerated Types		"
//	"""""""""""""""""""""""""""""""""
enum COLOR {
    BLUE 		= 0,
    GREEN 		= 1,
    RED 		= 2,
    DARK_BLUE 	= 3,
    DARK_GREEN 	= 4,
    DARK_RED 	= 5,
    GREY 		= 6,
    BLACK 		= 7,
    WHITE 		= 8,
};

enum SHAPE {
    CROSS 		= 0,
    CIRCLE 		= 1,
    TRIANGLE 	= 2,
    SQUARE 		= 3,
};

//	"""""""""""""""""""""""""""""""""
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""

using namespace std;

//	"""""""""""""""""""""""""""""""""
//	"		3rd Party Functions		"
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
BareMinimumPlotter::BareMinimumPlotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BareMinimumPlotter)
{
    ui->setupUi(this);
    QDoubleValidator *dValidator = new QDoubleValidator;
    QIntValidator *iValidator = new QIntValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEditElement1->setValidator(iValidator);
    ui->lineEditMax1->setValidator(dValidator);
    ui->lineEditMin1->setValidator(dValidator);
    ui->lineEditElement2->setValidator(iValidator);
    ui->lineEditMax2->setValidator(dValidator);
    ui->lineEditMin2->setValidator(dValidator);
}

//	Destructor
//	----------
BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}


//	Core Functions
//	---------------
void BareMinimumPlotter::checkFields(){

    // check not empty
    if (isEmpty_InputFields()){
        QMessageBox *msg = new QMessageBox(ui->centralWidget);
        msg->setText("Please fill in all input fields.");
        msg->setWindowTitle("Input error.");
        msg->show();
        return;
    }

    // check values ok
    if 	(ui->lineEditElement1->text().toInt() < 1)
        setQLineEditBackground(ui->lineEditElement1, "white", "red");
    if  (ui->lineEditMax1->text().toDouble() == ui->lineEditMin1->text().toDouble())
    {
        setQLineEditBackground(ui->lineEditMax1, "white", "red");
        setQLineEditBackground(ui->lineEditMin1, "white", "red");
    }

    if 	(ui->lineEditElement2->text().toInt() < 1)
        setQLineEditBackground(ui->lineEditElement2, "white", "red");
    if  (ui->lineEditMax2->text().toDouble() == ui->lineEditMin2->text().toDouble())
    {
        setQLineEditBackground(ui->lineEditMax2, "white", "red");
        setQLineEditBackground(ui->lineEditMin2, "white", "red");
    }
}


void BareMinimumPlotter::clearFormatting(){
    clearLineEditTextFormat(ui->lineEditInequalityLeft);
    clearLineEditTextFormat(ui->lineEditInequalityRight);
    setQLineEditBackground(ui->lineEditElement1, "black", "white");
    setQLineEditBackground(ui->lineEditMax1, "black", "white");
    setQLineEditBackground(ui->lineEditMin1, "black", "white");
    setQLineEditBackground(ui->lineEditElement2, "black", "white");
    setQLineEditBackground(ui->lineEditMax2, "black", "white");
    setQLineEditBackground(ui->lineEditMin2, "black", "white");
}

void BareMinimumPlotter::plot()
{
    clearFormatting();
    checkFields();

    // create inequality
    mInequality = Inequality(ui->lineEditInequalityLeft->text().toStdString(),
                             ui->comboBoxInequality->currentText().toStdString(),
                             ui->lineEditInequalityRight->text().toStdString());

    ui->lineEditInequalityLeft->setText(QString::fromStdString(mInequality.getExpressionLHS()));
    ui->lineEditInequalityRight->setText(QString::fromStdString(mInequality.getExpressionRHS()));

    // check expressions
    if(highlightInvalidExpressionTerms(mInequality, ui->lineEditInequalityLeft, ui->lineEditInequalityRight))
        return;

    // create and add variables
    ui->lineEditName1->setText(ui->lineEditName1->text().replace(QString(" "), QString("")));
    ui->lineEditName2->setText(ui->lineEditName2->text().replace(QString(" "), QString("")));


    mVariable1 = Variable(ui->lineEditName1->text().toStdString(),
                          ui->lineEditMin1->text().toDouble(),
                          ui->lineEditMax1->text().toDouble(),
                          ui->lineEditElement1->text().toInt());

    mVariable2 = Variable(ui->lineEditName2->text().toStdString(),
                          ui->lineEditMin2->text().toDouble(),
                          ui->lineEditMax2->text().toDouble(),
                          ui->lineEditElement2->text().toInt());

    if (!mInequality.variableIsValid(mVariable1))
        return;
    mInequality.addVariable(mVariable1);

    if (!mInequality.variableIsValid(mVariable2))
        return;
    mInequality.addVariable(mVariable2);

    // do maths
    vector<bool> vPlotSpace;
    try{
        vPlotSpace = mInequality.evaluate();
    }
    catch(INPUT_ERROR_CODES e){ // catch errors that happen during evaluation
        switch(e){
        case INPUT_ERROR_INVALID_EXPRESSION:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Invalid expression." << endl;
            if(highlightInvalidExpressionTerms(mInequality, ui->lineEditInequalityLeft, ui->lineEditInequalityRight))
                return;
            break;
        case INPUT_ERROR_UNINITIALIZED_VARIABLE:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Uninitialized variable." << endl;
            if(highlightInvalidExpressionTerms(mInequality, ui->lineEditInequalityLeft, ui->lineEditInequalityRight))
                return;
            break;
        default:
            cerr << "[ERROR] BareMinimumPlotter | plot | do maths | "<< "Unhandled INPUT_ERROR_CODE exception caught: ";
            cerr << e << endl;
            return;
        }

    }

    vector<int> vProblemSpace = mInequality.getProblemElements_ResultsCombined();
    vector<int>::iterator it_ProblemSpace = vProblemSpace.begin();

    // create plotting vectors
    QVector<double> x, y, x_problem, y_problem;
    mVariable1.resetPosition(); // reset iterators
    mVariable2.resetPosition();
    // 	iterate through boolean results,
    // 	create plotting vectors at points where results are 1
    for(int i = 0; i < static_cast<int>(vPlotSpace.size()); i++){
        // problem point - add to problem vectors
        // (the logic assumes that the problem points are added in order)
        if(!vProblemSpace.empty() && i == *it_ProblemSpace){
            x_problem.push_back(mVariable1.getCurrentValue());
            y_problem.push_back(mVariable2.getCurrentValue());
            it_ProblemSpace++;
        }
        else if (vPlotSpace[i]) {
            // not a problem point - add to the normal graph vectors
            x.push_back(mVariable1.getCurrentValue());
            y.push_back(mVariable2.getCurrentValue());
        }

        if ((i+1) % mVariable2.getGrid() == 0){
            mVariable1.nextPosition();
        }
        mVariable2.nextPosition();
    }

    // add normal graph
    ui->plotter->clearGraphs();
    ui->plotter->addGraph();
    ui->plotter->graph(0)->setData(x,y);
    ui->plotter->graph(0)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
    QCPScatterStyle style1;
    //	- set marker
    switch(ui->comboBoxShape->currentIndex()){
    case CROSS:
        style1.setShape(QCPScatterStyle::ssCross);
        break;
    case CIRCLE:
        style1.setShape(QCPScatterStyle::ssCircle);
        break;
    case TRIANGLE:
        style1.setShape(QCPScatterStyle::ssTriangle);
        break;
    case SQUARE:
        style1.setShape(QCPScatterStyle::ssSquare);
        break;
    }

    //	- set size
    style1.setSize(5);
    //	- set color
    switch(ui->comboBoxColor->currentIndex()){
    case RED:
        style1.setPen(QPen(Qt::red));
        break;
    case GREEN:
        style1.setPen(QPen(Qt::green));
        break;
    case BLUE:
        style1.setPen(QPen(Qt::blue));
        break;
    case DARK_RED:
        style1.setPen(QPen(Qt::darkRed));
        break;
    case DARK_GREEN:
        style1.setPen(QPen(Qt::darkGreen));
        break;
    case DARK_BLUE:
        style1.setPen(QPen(Qt::darkBlue));
        break;
    case GREY:
        style1.setPen(QPen(Qt::lightGray));
        break;
    case BLACK:
        style1.setPen(QPen(Qt::black));
        break;
    }

    ui->plotter->graph(0)->setScatterStyle(style1);

    // add problem graph (if needed)
    if (!x_problem.isEmpty()){
        ui->plotter->addGraph();
        ui->plotter->graph(1)->setData(x_problem,y_problem);
        ui->plotter->graph(1)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
        QCPScatterStyle style2;
        style2.setShape(QCPScatterStyle::ssCross);
        style2.setSize(5);
        style2.setPen(QPen(Qt::red));
        ui->plotter->graph(1)->setScatterStyle(style2);
    }

    // set general options, plot
    ui->plotter->xAxis->setLabel(QString::fromStdString(mVariable1.getName()));
    ui->plotter->yAxis->setLabel(QString::fromStdString(mVariable2.getName()));
    ui->plotter->xAxis->setRange(mVariable1.getMin(), mVariable1.getMax());
    ui->plotter->yAxis->setRange(mVariable2.getMin(), mVariable2.getMax());
    ui->plotter->replot();
}

//	Validation
//	----------

bool BareMinimumPlotter::highlightInvalidExpressionTerms(Inequality mInequality, QLineEdit * qLineEditLHS, QLineEdit * qLineEditRHS){
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
        setLineEditTextFormat(qLineEditLHS, formats);

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

        setLineEditTextFormat(qLineEditRHS, formats);

        cerr << "[ERROR] BareMinimumPlotter | highlightInvalidExpressionTerms | " << "RHS inequality is invalid" << endl;
    }
    return flag_highlight;
}

bool BareMinimumPlotter::isEmpty_InputFields(){
    return 	ui->lineEditElement1->text().isEmpty() ||
            ui->lineEditElement2->text().isEmpty() ||
            ui->lineEditInequalityLeft->text().isEmpty() ||
            ui->lineEditInequalityRight->text().isEmpty() ||
            ui->lineEditMax1->text().isEmpty() ||
            ui->lineEditMax2->text().isEmpty() ||
            ui->lineEditMin1->text().isEmpty() ||
            ui->lineEditMin2->text().isEmpty() ||
            ui->lineEditName1->text().isEmpty() ||
            ui->lineEditName2->text().isEmpty();
}

//	"""""""""""""""""""""""""""""""""
//	"		Private Functions		"
//	"""""""""""""""""""""""""""""""""


bool BareMinimumPlotter::charsValid(QLineEdit* qLineEdit){
    Expression tempExp;
    string s = qLineEdit->text().toStdString();
    for (string::iterator it = s.begin(); it != s.end(); it ++){
        // if a char is invalid
        if (!tempExp.charIsValid(*it))
            return false;
    }
    // if all chars valid
    return true;
}

//	"""""""""""""""""""""""""""""""""
//	"	Private Slot Functions		"
//	"""""""""""""""""""""""""""""""""

void BareMinimumPlotter::on_buttonPlot_clicked()
{
    plot();
}

void BareMinimumPlotter::on_lineEditInequalityLeft_textChanged(const QString &arg1)
{
   clearLineEditTextFormat(ui->lineEditInequalityLeft);
}

void BareMinimumPlotter::on_lineEditInequalityRight_textChanged(const QString &arg1)
{
   clearLineEditTextFormat(ui->lineEditInequalityRight);
}
