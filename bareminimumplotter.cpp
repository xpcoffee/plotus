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
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""


//	Constructor
//	-----------

BareMinimumPlotter::BareMinimumPlotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BareMinimumPlotter)
{
    ui->setupUi(this);

    // add original x & y variable inputs
    nLatestVariableInput = 0;
    nLatestInequalityInput = 0;
    addVariableInput();
    addVariableInput();
    vVariableInputs[0]->setAxisMode(MODE_X_AXIS);
    vVariableInputs[1]->setAxisMode(MODE_Y_AXIS);
    vVariableInputs[0]->enableRemoveButton(false);
    vVariableInputs[1]->enableRemoveButton(false);

    // add original inequality input
    addInequalityInput();
    vInequalityInputs.front()->enablePositionButtons(false);

}


//	Destructor
//	----------

BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}


//	Core
//	----

void BareMinimumPlotter::clearFormatting(){
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
       vInequalityInputs[i]->clearFormatting();
    }
    for (vector<VariableInput*>::iterator it = vVariableInputs.begin(); it != vVariableInputs.end(); it++){
        (*it)->clearFormatting();
    }
}

void BareMinimumPlotter::plot()
{
    clearFormatting();

    cout << "Creating Inequality" << endl;

    // create inequality
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
       if(!vInequalityInputs[i]->createInequality())
           return;
    }

    cout << "Creating and adding variables." << endl;
    // create and add variables
    for (int j = 0; j < static_cast<int>(vInequalityInputs.size()); j++){
        for (int i = 0; i < static_cast<int>(vVariableInputs.size()); i++){
            if (!vVariableInputs[i]->checkInput()) // check legal
                return;
            Variable tmpVariable = vVariableInputs[i]->getVariable();
            if(!vInequalityInputs[j]->addVariable(tmpVariable))
                return;
        }
    }

    cout << "Selecting plotting variables." << endl;

    string sUnitsX, sUnitsY;

    // select plotting variables
    int tmpCheck = 0;
    for (int i = 0; i < static_cast<int>(vVariableInputs.size()); i++){
        if (vVariableInputs[i]->getAxisMode() == MODE_X_AXIS){
            mVariableX = vVariableInputs[i]->getVariable();
            sUnitsX = vVariableInputs[i]-> getUnits();
            tmpCheck++;
        } else if (vVariableInputs[i]->getAxisMode() == MODE_Y_AXIS){
            mVariableY = vVariableInputs[i]->getVariable();
            sUnitsY = vVariableInputs[i]-> getUnits();
            tmpCheck++;
        }
    }
    if (tmpCheck != 2){
        cerr << "problem getting x and y plotting vector variables" << endl;
        return;
    }

    // graph and plot
    int nGraphIndex = 0;
    ui->plotter->clearGraphs();

    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        cout << "Doing math: " << i << endl;
        //do math
        vInequalityInputs[i]->setXYVariables(mVariableX, mVariableY);
        if(!vInequalityInputs[i]->evaluate())
            return;

        cout << "Get Plotting Vectors" << endl;
        // get plotting vectors
        qvX = vInequalityInputs[i]->getX();

        cout << "X | size: " << qvX.size() << endl;
        for (int j = 0; j < qvX.size(); j++){
            cout << "\t" << qvX[j] << endl;
        }

        qvY = vInequalityInputs[i]->getY();
        qvX_problem = vInequalityInputs[i]->getXProblem();
        qvY_problem = vInequalityInputs[i]->getYProblem();

        cout << "Adding normal graph." << endl;

        // add normal graph
        ui->plotter->addGraph();
        ui->plotter->graph(nGraphIndex)->setData(qvX,qvY);
        ui->plotter->graph(nGraphIndex)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
        QCPScatterStyle style1;
        //	- set marker
        switch(vInequalityInputs[i]->getShapeIndex()){
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
        switch(vInequalityInputs[i]->getColorIndex()){
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

        ui->plotter->graph(nGraphIndex)->setScatterStyle(style1);
        nGraphIndex++;

        // add problem graph (if needed)
        if (!qvX_problem.isEmpty()){
            cout << "Adding error graph." << endl;
            ui->plotter->addGraph();
            ui->plotter->graph(nGraphIndex)->setData(qvX_problem,qvY_problem);
            ui->plotter->graph(nGraphIndex)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
            QCPScatterStyle style2;
            style2.setShape(QCPScatterStyle::ssCross);
            style2.setSize(5);
            style2.setPen(QPen(Qt::red));
            ui->plotter->graph(nGraphIndex)->setScatterStyle(style2);
            nGraphIndex++;
        }

        cout << "Plotting." << endl;

        // set general options, plot
        ui->plotter->xAxis->setLabel(QString::fromStdString(mVariableX.getName() + " [" + sUnitsX + "]"));
        ui->plotter->yAxis->setLabel(QString::fromStdString(mVariableY.getName() + " [" + sUnitsY + "]"));
        ui->plotter->xAxis->setRange(mVariableX.getMin(), mVariableX.getMax());
        ui->plotter->yAxis->setRange(mVariableY.getMin(), mVariableY.getMax());
        ui->plotter->replot();
    }

}

//	Validation
//	----------

//	GUI
//	---

void BareMinimumPlotter::addVariableInput(){
    vVariableInputs.push_back(new VariableInput());
    vVariableInputs.back()->setNumber(nLatestVariableInput++);
    ui->layout_Variable->addWidget(vVariableInputs.back());
    // connect slots to signals
    QObject::connect(vVariableInputs.back(), SIGNAL(axisModeChanged(int)), 	// axis mode synch
                     this, SLOT(checkAxisMode(int)));
    QObject::connect(vVariableInputs.back(), SIGNAL(killThis(int)),			// delete
                     this, SLOT(removeVariableInput(int)));
    if (vVariableInputs.size() > 2){
        vVariableInputs.back()->setAxisMode(MODE_POINT);
        vVariableInputs[0]->enableRemoveButton(true);
        vVariableInputs[1]->enableRemoveButton(true);
    }
}

void BareMinimumPlotter::addInequalityInput(){
    vInequalityInputs.push_back(new InequalityInput());
    vInequalityInputs.back()->setNumber(nLatestInequalityInput++);
    ui->layout_Inequality->addWidget(vInequalityInputs.back());
    // connect slots to signals
    QObject::connect(vInequalityInputs.back(), SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    // enable/disable position buttons
    if (vInequalityInputs.size() > 1){
        vInequalityInputs.front()->enablePositionButtons(true);
    }

}


//	"""""""""""""""""""""""""""""""""
//	"		Public Slots			"
//	"""""""""""""""""""""""""""""""""

void BareMinimumPlotter::checkAxisMode(int nVariableInputNumber){
    int nX = 0, nY = 0, nXPos, nYPos;
    for (int i = 0; i < static_cast<int>(vVariableInputs.size()); i++){
       if(vVariableInputs[i]->getAxisMode() == MODE_X_AXIS) {
            nX++;
            if (vVariableInputs[i]->getNumber() != nVariableInputNumber)
                nXPos = i;
       }
       if(vVariableInputs[i]->getAxisMode() == MODE_Y_AXIS) {
            nY++;
            if (vVariableInputs[i]->getNumber() != nVariableInputNumber)
                nYPos = i;
       }
    }
    if (nX == 2 && nY == 1){
        vVariableInputs[nXPos]->setAxisMode(MODE_POINT);
    } else if (nX == 2 && nY == 0) {
        vVariableInputs[nXPos]->setAxisMode(MODE_Y_AXIS);
    } else if (nX == 0 && nY == 2) {
        vVariableInputs[nYPos]->setAxisMode(MODE_X_AXIS);
    } else if (nX == 1 && nY == 2) {
        vVariableInputs[nYPos]->setAxisMode(MODE_POINT);
    } else if (nX == 1 && nY == 0) {
        vVariableInputs[nVariableInputNumber]->setAxisMode(MODE_Y_AXIS);
    } else if (nX == 0 && nY == 1) {
        vVariableInputs[nVariableInputNumber]->setAxisMode(MODE_X_AXIS);
    }
}

void BareMinimumPlotter::removeVariableInput(int nVariableInputNumber){
    if (vVariableInputs.size() < 3) // at least 2 variable inputs needed
        return;
    for (unsigned int i = 0; i < vVariableInputs.size(); i++){
        if (vVariableInputs[i]->getNumber() == nVariableInputNumber){
            int nAxisMode = vVariableInputs[i]->getAxisMode();
            // delete
            ui->layout_Variable->removeWidget(vVariableInputs[i]);
            delete vVariableInputs[i];
            if (i < vVariableInputs.size())
                vVariableInputs.erase(vVariableInputs.begin()+i);
            // ensure correct axis modes
            // [TODO]	make this more elegant
            if (nAxisMode != MODE_POINT){
                if (i < vVariableInputs.size()){
                    if(vVariableInputs[i]->getAxisMode() != MODE_POINT){
                        if (i+1 < vVariableInputs.size()){
                            vVariableInputs[i+1]->setAxisMode(nAxisMode);
                        } else if (i != 0){
                            vVariableInputs[i-1]->setAxisMode(nAxisMode);
                        }
                    } else {
                        vVariableInputs[i]->setAxisMode(nAxisMode);
                    }
                } else {
                    if(vVariableInputs[i-1]->getAxisMode() != MODE_POINT){
                        if (i != 0){
                            vVariableInputs[i-2]->setAxisMode(nAxisMode);
                        }
                    } else {
                        vVariableInputs[i-1]->setAxisMode(nAxisMode);
                    }

                }
            }
            // enable removal buttons
            if (vVariableInputs.size() < 3){
                vVariableInputs.front()->enableRemoveButton(false);
                vVariableInputs.back()->enableRemoveButton(false);
            }
            return;
        }
    }
}

void BareMinimumPlotter::removeInequalityInput(int nInequalityNumber){
    if (vInequalityInputs.size() < 2) // at least 1 inequality input needed
        return;
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
            ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
            delete vInequalityInputs[i];
            if (i < static_cast<int>(vInequalityInputs.size()))
                vInequalityInputs.erase(vInequalityInputs.begin()+i);
            // enable/disable position buttons
            if (vInequalityInputs.size() < 2)
                vInequalityInputs.back()->enablePositionButtons(false);
            return;
        }
    }
}

void BareMinimumPlotter::moveInequalityInputUp (int nInequalityNumber){
    int nCurrentPos;
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
            nCurrentPos = ui->layout_Inequality->indexOf(vInequalityInputs[i]);
            if (nCurrentPos == 0) // if at top
                return;
            ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
            ui->layout_Inequality->insertWidget(--nCurrentPos, vInequalityInputs[i]);
        }
    }
}

void BareMinimumPlotter::moveInequalityInputDown (int nInequalityNumber){
    int nCurrentPos;
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
            nCurrentPos = ui->layout_Inequality->indexOf(vInequalityInputs[i]);
            if (nCurrentPos == ui->layout_Inequality->count()) // if at bottom
                return;
            ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
            ui->layout_Inequality->insertWidget(++nCurrentPos, vInequalityInputs[i]);
        }
    }
}

//	"""""""""""""""""""""""""""""""""
//	"		Private Slots	 		"
//	"""""""""""""""""""""""""""""""""

void BareMinimumPlotter::on_button_Plot_clicked()
{
    plot();
}

void BareMinimumPlotter::on_button_AddVariable_clicked()
{
    addVariableInput();
}

void BareMinimumPlotter::on_button_AddInequality_clicked()
{
    addInequalityInput();
}
