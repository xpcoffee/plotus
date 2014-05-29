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
    ui->lineEditMax1->setValidator(dValidator);
    ui->lineEditMin1->setValidator(dValidator);
    ui->lineEditMax2->setValidator(dValidator);
    ui->lineEditMin2->setValidator(dValidator);
    ui->lineEditElement1->setValidator(iValidator);
    ui->lineEditElement2->setValidator(iValidator);
}

//	Destructor
//	----------

BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}


//	Core Functions
//	---------------

void BareMinimumPlotter::plot()
{
    // check input - GUI
    if (isEmpty_InputFields()){
        cerr << "[ERROR] BareMinimumPlotter | create expressions| " << "Empty input field(s)." << endl;
        QMessageBox *msg = new QMessageBox(ui->centralWidget);
        msg->setText("Please fill in all input fields.");
        msg->setWindowTitle("Input error.");
        msg->show();
        return;
    }

    if (!charsValid(ui->lineEditInequalityLeft)){
        cerr << "[ERROR] BareMinimumPlotter | create expressions| " << "Left expression has invalid char(s)." << endl;
        QMessageBox *msg = new QMessageBox(ui->centralWidget);
        msg->setText("Left expression has invalid char(s).");
        msg->setWindowTitle("Input error.");
        msg->show();
        return;
    }

    if (!charsValid(ui->lineEditInequalityRight)){
        cerr << "[ERROR] BareMinimumPlotter | create expressions| " << "Right expression has invalid char(s)." << endl;
        QMessageBox *msg = new QMessageBox(ui->centralWidget);
        msg->setText("Right expression has invalid char(s).");
        msg->setWindowTitle("Input error.");
        msg->show();
        return;
    }

    // create inequality
    mInequality = Inequality(ui->lineEditInequalityLeft->text().toStdString(),
                             ui->comboBox->currentText().toStdString(),
                             ui->lineEditInequalityRight->text().toStdString());

    // check expressions
    if(expressionsInvalid(mInequality, ui->lineEditInequalityLeft, ui->lineEditInequalityRight))
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
    vector<bool> vPlotSpace = mInequality.evaluate();
    vector<int> vProblemSpace = mInequality.getProblemElements_ResultsCombined();
    vector<int>::iterator it_ProblemSpace = vProblemSpace.begin();

    // create plotting vectors
    QVector<double> x, y, x_problem, y_problem;
    mVariable1.resetPosition(); // reset iterators
    mVariable2.resetPosition();
        // it_ProblemSpaceerate through boolean results, only copy matches
    for(int i = 0; i < static_cast<int>(vPlotSpace.size()); i++){
        // if this is a problem point, add to problem vectors
        // otherwise add to the normal graph vectors
        // (the logic assumes that the problem points are added in order)
        if(!vProblemSpace.empty() && i == *it_ProblemSpace){
            x_problem.push_back(mVariable1.getCurrentValue());
            y_problem.push_back(mVariable2.getCurrentValue());
            it_ProblemSpace++;
        }
        else if (vPlotSpace[i]) {
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
    style1.setShape(QCPScatterStyle::ssCross);
    style1.setSize(5);
    style1.setPen(QPen(Qt::blue));
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

bool BareMinimumPlotter::expressionsInvalid(Inequality mInequality, QLineEdit * qLineEditLHS, QLineEdit * qLineEditRHS){
    bool flag_invalid = false;
    // check LHS
    if (mInequality.isInvalid()){
        flag_invalid = true;
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

        cout << "[ERROR] Inequality | checking expressions | " << "inequality is invalid" << endl;
    }

    // check RHS
    if (mInequality.isInvalid()){
        flag_invalid = true;
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

        cout << "[ERROR] Inequality | checking expressions | " << "inequality is invalid" << endl;
    }
    return flag_invalid;
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
