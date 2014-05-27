#include "bareminimumplotter.h"
#include "ui_bareminimumplotter.h"
#include "deplot/include/inequality.h"
#include <qvalidator.h>
#include <qlocale.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

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

BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}

void BareMinimumPlotter::plot()
{
    // check input not empty
    if (isEmpty_InputFields()){
        QMessageBox *msg = new QMessageBox(ui->centralWidget);
        msg->setText("Please fill in all input fields.");
        msg->setWindowTitle("Input error.");
        msg->show();
        return;
    }

    // create inequality
    cout << "combo box string: " << ui->comboBox->currentText().toStdString() << endl;
    mInequality = Inequality(ui->lineEditInequalityLeft->text().toStdString(),
                             ui->comboBox->currentText().toStdString()[0],
                             ui->lineEditInequalityRight->text().toStdString());

    // check expressions
    vector<int> vInputErrorsLHS = mInequality.getProblemElements_Expression1();
    vector<int> vInputErrorsRHS = mInequality.getProblemElements_Expression2();

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

    mInequality.addVariable(mVariable1);
    mInequality.addVariable(mVariable2);

    // do maths
    vector<bool> vPlotSpace = mInequality.evaluate();
    vector<int> vProblemSpace = mInequality.getProblemElements_Result();
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
//        cout << "i = " << i << ", *it = " << *it_ProblemSpace << " | ";
        if(!vProblemSpace.empty() && i == *it_ProblemSpace){
            cout << endl;
            cout << "[DEBUG] plot() | " << "adding problem point" << endl;
            x_problem.push_back(mVariable1.getCurrentValue());
            y_problem.push_back(mVariable2.getCurrentValue());
            it_ProblemSpace++;
        }
        else if (vPlotSpace[i]) {
//                cout << "[DEBUG] plot() | " << "adding normal point" << endl;
            x.push_back(mVariable1.getCurrentValue());
            y.push_back(mVariable2.getCurrentValue());
        }

//            cout << "[INFO] plot() | " << "QVector[" << (x.size()) << "]" <<
//                    "\t| x: " << mVariable1.getCurrentValue() << // *x.end() <<
//                    "\t| y: " << mVariable2.getCurrentValue() << //*y.end() <<
//                    endl;

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

void BareMinimumPlotter::on_buttonPlot_clicked()
{
    plot();
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
