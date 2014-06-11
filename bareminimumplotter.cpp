/*	TASK LIST
    ---------
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
//#include <qpalette.h>

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
    ui->layout_Variable->setAlignment(Qt::AlignTop);
    nLatestVariableInput = 0;
    addVariableInput();
    addVariableInput();
    vVariableInputs[0]->setAxisMode(MODE_X_AXIS);
    vVariableInputs[1]->setAxisMode(MODE_Y_AXIS);
    vVariableInputs[0]->enableRemoveButton(false);
    vVariableInputs[1]->enableRemoveButton(false);
    // add original inequality input
    ui->layout_Inequality->setAlignment(Qt::AlignTop);
    nLatestInequalityInput = 0;
    addInequalityInput();
    vInequalityInputs.front()->enablePositionButtons(false);
    // initialize global variables
    flag_Combination = false;
    nPrevCombination = 0;
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

    // 	determine plotting variables
    string sUnitsX, sUnitsY;
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

    //	evaluate and plot each inequality
    nGraphIndex = 0;
    nPrevCombination = 0;
    ui->plotter->clearGraphs();

    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        //	check skip
        if (vInequalityInputs[i]->getSkip())
            continue;

        // 	create inequality
        if(!vInequalityInputs[i]->createInequality())
           return;

        // 	create and add variables
        for (int j = 0; j < static_cast<int>(vVariableInputs.size()); j++){
            if (!vVariableInputs[j]->checkInput()) // check legal
                return;
            Variable tmpVariable = vVariableInputs[j]->getVariable();
            if(!vInequalityInputs[i]->addVariable(tmpVariable))
                return;
        }

        //do math
        vInequalityInputs[i]->setXYVariables(mVariableX, mVariableY);
        if(!vInequalityInputs[i]->evaluate())
            return;

        // get plotting vectors
        switch(nPrevCombination){
        case COMBINE_NONE:
            vectorCombineNone(i);
            break;
        case COMBINE_INTERSECTION:
            vectorCombineIntersection(i);
            break;
        case COMBINE_UNION:
            vectorCombineUnion(i);
            break;
        case COMBINE_SUBTRACTION:
            vectorCombineSubtraction(i);
            break;
        }
        nPrevCombination = vInequalityInputs[i]->getCombination();
        if (nPrevCombination != COMBINE_NONE)
            continue;

        // add normal graph
        ui->plotter->addGraph();
        formatGraph(i);
        nGraphIndex++;

        // add problem graph (if needed)
        if (!qvX_problem.isEmpty()){
            ui->plotter->addGraph();
            formatErrorGraph();
            nGraphIndex++;
        }

        // set general options, plot
        ui->plotter->xAxis->setLabel(QString::fromStdString(mVariableX.getName() + " [" + sUnitsX + "]"));
        ui->plotter->yAxis->setLabel(QString::fromStdString(mVariableY.getName() + " [" + sUnitsY + "]"));
        ui->plotter->xAxis->setRange(mVariableX.getMin(), mVariableX.getMax());
        ui->plotter->yAxis->setRange(mVariableY.getMin(), mVariableY.getMax());
        ui->plotter->replot();
    }
}

void BareMinimumPlotter::vectorCombineNone(int nInequality){
        qvX = vInequalityInputs[nInequality]->getX();
        qvY = vInequalityInputs[nInequality]->getY();
        qvX_problem = vInequalityInputs[nInequality]->getXProblem();
        qvY_problem = vInequalityInputs[nInequality]->getYProblem();
}

void BareMinimumPlotter::vectorCombineIntersection(int nInequality){
    flag_Combination = true;
    QVector<double> qvXOld = qvX;
    QVector<double> qvXNew = vInequalityInputs[nInequality]->getX();
    QVector<double> qvYOld = qvY;
    QVector<double> qvYNew = vInequalityInputs[nInequality]->getY();

    qvX.clear();
    qvY.clear();

    for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
       for (int j = 0; j < static_cast<int>(qvXNew.size()); j++) {
           if ((qvXOld[i] == qvXNew[j]) && (qvYOld[i] == qvYNew[j])) {
               qvX.push_back(qvXOld[i]);
               qvY.push_back(qvYOld[i]);
           }
       }
    }

    qvX_problem = vInequalityInputs[nInequality]->getXProblem();
    qvY_problem = vInequalityInputs[nInequality]->getYProblem();
}

// [TODO] prevent user from changing '-' in last inequality.
// [TODO] progress bar
void BareMinimumPlotter::vectorCombineUnion(int nInequality){
    QVector<double> qvXOld = qvX;
    QVector<double> qvYOld = qvY;
    QVector<double> qvXNew = vInequalityInputs[nInequality]->getX();
    QVector<double> qvYNew = vInequalityInputs[nInequality]->getY();

    qvX.clear();
    qvY.clear();

    for (mVariableX.resetPosition(); !mVariableX.isEnd(); mVariableX.nextPosition()){
        for (mVariableY.resetPosition(); !mVariableY.isEnd(); mVariableY.nextPosition()){
            for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
                if ((mVariableX.getCurrentValue() == qvXOld[i]) && (mVariableY.getCurrentValue() == qvYOld[i])){
                    qvX.push_back(mVariableX.getCurrentValue());
                    qvY.push_back(mVariableY.getCurrentValue());
                }
            }
            for (int i = 0; i < static_cast<int>(qvXNew.size()); i++){
                if ((mVariableX.getCurrentValue() == qvXNew[i]) && (mVariableY.getCurrentValue() == qvYNew[i])){
                    qvX.push_back(mVariableX.getCurrentValue());
                    qvY.push_back(mVariableY.getCurrentValue());
                }
            }
        }
    }
}

void BareMinimumPlotter::vectorCombineSubtraction(int nInequality){
    flag_Combination = true;
    QVector<double> qvXOld = qvX;
    QVector<double> qvXNew = vInequalityInputs[nInequality]->getX();
    QVector<double> qvYOld = qvY;
    QVector<double> qvYNew = vInequalityInputs[nInequality]->getY();

    qvX.clear();
    qvY.clear();
    bool flag_keep;

    for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
       flag_keep = true;
       for (int j = 0; j < static_cast<int>(qvXNew.size()); j++) {
           if (((qvXOld[i] == qvXNew[j]) && (qvYOld[i] == qvYNew[j]))) {
               flag_keep = false;
           }
       }
       if (flag_keep){
           qvX.push_back(qvXOld[i]);
           qvY.push_back(qvYOld[i]);
       }
    }

    qvX_problem = vInequalityInputs[nInequality]->getXProblem();
    qvY_problem = vInequalityInputs[nInequality]->getYProblem();
}

void BareMinimumPlotter::formatGraph(int i){
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
}

void BareMinimumPlotter::formatErrorGraph(){
            ui->plotter->graph(nGraphIndex)->setData(qvX_problem,qvY_problem);
            ui->plotter->graph(nGraphIndex)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
            QCPScatterStyle style2;
            style2.setShape(QCPScatterStyle::ssCross);
            style2.setSize(5);
            style2.setPen(QPen(Qt::red));
            ui->plotter->graph(nGraphIndex)->setScatterStyle(style2);
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
    vInequalityInputs.back()->setIndex(vInequalityInputs.size());
    ui->layout_Inequality->addWidget(vInequalityInputs.back());
    // connect slots to signals
    QObject::connect(vInequalityInputs.back(), SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    // enable position buttons of first element
    if (vInequalityInputs.size() > 1){
        vInequalityInputs.front()->enablePositionButtons(true);
    }
    // enable/disable combination fields
    setCombinationInputs();
}

void BareMinimumPlotter::reOrderInequalityInputs(){
    vector<InequalityInput*> tmpVec;
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
       tmpVec.push_back(qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget()));
       qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget())->setIndex(i+1);
    }
    vInequalityInputs = tmpVec;
}

void BareMinimumPlotter::setCombinationInputs(){
    int nSize = ui->layout_Inequality->count()-1;
    // disable the combinations of last item
    qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize)->widget())->enableCombinations(false);
    qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize)->widget())->resetCombinations();
    // enable combinations of second last item
    if (ui->layout_Inequality->count() > 1)
        qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize-1)->widget())->enableCombinations(true);
}


//	"""""""""""""""""""""""""""""""""
//	"		Public Slots			"
//	"""""""""""""""""""""""""""""""""

void BareMinimumPlotter::checkAxisMode(int nVariableInputNumber){
    int nX = 0, nY = 0, nXPos, nYPos;
    for (int i = 0; i < static_cast<int>(vVariableInputs.size()); i++){ 	// find x-axis and y-axis labels
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

    if (nX == 2 && nY == 1){ vVariableInputs[nXPos]->setAxisMode(MODE_POINT); } 	// sort such that only one x and one y
    else if (nX == 2 && nY == 0) { vVariableInputs[nXPos]->setAxisMode(MODE_Y_AXIS); }
    else if (nX == 0 && nY == 2) { vVariableInputs[nYPos]->setAxisMode(MODE_X_AXIS); }
    else if (nX == 1 && nY == 2) { vVariableInputs[nYPos]->setAxisMode(MODE_POINT); }
    else if (nX == 1 && nY == 0) { vVariableInputs[nVariableInputNumber]->setAxisMode(MODE_Y_AXIS); }
    else if (nX == 0 && nY == 1) { vVariableInputs[nVariableInputNumber]->setAxisMode(MODE_X_AXIS); }
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
            reOrderInequalityInputs();
            setCombinationInputs();
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
            reOrderInequalityInputs();
            setCombinationInputs();
        }
    }
}

void BareMinimumPlotter::moveInequalityInputDown (int nInequalityNumber){
    int nCurrentPos;
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
            nCurrentPos = ui->layout_Inequality->indexOf(vInequalityInputs[i]);
            if (nCurrentPos == ui->layout_Inequality->count()) 		// if at bottom
                return;
            ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
            ui->layout_Inequality->insertWidget(++nCurrentPos, vInequalityInputs[i]);
            reOrderInequalityInputs();
            setCombinationInputs();
            return;
        }
    }
}


//	"""""""""""""""""""""""""""""""""
//	"		Private Slots	 		"
//	"""""""""""""""""""""""""""""""""

void BareMinimumPlotter::on_button_Plot_clicked() { plot(); }

void BareMinimumPlotter::on_button_AddVariable_clicked() { addVariableInput(); }

void BareMinimumPlotter::on_button_AddInequality_clicked() { addInequalityInput(); }
