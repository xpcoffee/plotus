#include "bareminimumplotter.h"
#include "ui_bareminimumplotter.h"
#include "deplot/include/inequality.h"
#include <qvalidator.h>
#include <qlocale.h>
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

    // create variables
    mVariable1 = Variable(ui->lineEditName1->text().toStdString(),
                          ui->lineEditMin1->text().toDouble(),
                          ui->lineEditMax1->text().toDouble(),
                          ui->lineEditElement1->text().toInt());

    mVariable2 = Variable(ui->lineEditName2->text().toStdString(),
                          ui->lineEditMin2->text().toDouble(),
                          ui->lineEditMax2->text().toDouble(),
                          ui->lineEditElement2->text().toInt());

    // create inequality
    cout << "combo box string: " << ui->comboBox->currentText().toStdString() << endl;
    mInequality = Inequality(ui->lineEditInequalityLeft->text().toStdString(),
                             ui->comboBox->currentText().toStdString()[0],
                             ui->lineEditInequalityRight->text().toStdString());

    mInequality.addVariable(mVariable1);
    mInequality.addVariable(mVariable2);

    // do maths
    vector<bool> space = mInequality.evaluate();

    // create plotting vectors
    QVector<double> x, y;
    mVariable1.resetPosition(); // reset iterators
    mVariable2.resetPosition();
        // iterate through boolean results, only copy matches
    for(unsigned int i = 0; i < space.size(); i++){
        cout << "space[" << i <<  "] : " << space[i] << endl;
        if (space[i]){
            x.push_back(mVariable1.getCurrentValue());
            y.push_back(mVariable2.getCurrentValue());
            cout << "QVector[" << (x.size()) << "]" <<
                    "\t| x: " << mVariable1.getCurrentValue() << // *x.end() <<
                    "\t| y: " << mVariable2.getCurrentValue() << //*y.end() <<
                    endl;
        }
        if ((i+1) % mVariable2.getGrid() == 0){
            mVariable1.nextPosition();
        }
        mVariable2.nextPosition();
    }

    // plot
    ui->plotter->clearGraphs();
    ui->plotter->addGraph();
    ui->plotter->graph(0)->setData(x,y);
    ui->plotter->xAxis->setLabel(QString::fromStdString(mVariable1.getName()));
    ui->plotter->yAxis->setLabel(QString::fromStdString(mVariable2.getName()));
    ui->plotter->xAxis->setRange(mVariable1.getMin(), mVariable1.getMax());
    ui->plotter->yAxis->setRange(mVariable2.getMin(), mVariable2.getMax());
    ui->plotter->graph(0)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
    ui->plotter->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
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
