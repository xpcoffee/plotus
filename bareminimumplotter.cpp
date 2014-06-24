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
#include <iostream>
#include <fstream>
#include <sstream>


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
//	"			Namespaces			"
//	"""""""""""""""""""""""""""""""""

using namespace std;


//	"""""""""""""""""""""""""""""""""
//	"		Public Functions		"
//	"""""""""""""""""""""""""""""""""

void elideLable(QLabel *label){
    QString text = label->text();
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
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
    // format ui elements - things that can't be done in designer
    QString style_GroupBox = "QGroupBox {\
                                border-width: 1px;\
                                border-style: solid;\
                                border-radius: 10px;\
                                border-color: lightGray;\
                                font-weight: bold;\
                                color: gray;}\
                             QGroupBox::title {\
                                 background-color: transparent;}";
    ui->groupBox_Inequalities->setStyleSheet(style_GroupBox);
    ui->groupBox_Variables->setStyleSheet(style_GroupBox);
    QString style_TextEdit = "QTextEdit {\
                                border-width: 1px;\
                                border-style: solid;\
                                border-color: lightGray;\
                                border-radius: 10px;}";
    ui->textEditError->setStyleSheet(style_TextEdit);
    elideLable(ui->label_VariableElements);
    elideLable(ui->label_VariableChosenPoint);
    // add original x & y variable inputs
    ui->layout_Variable->setAlignment(Qt::AlignTop);
    ui->layout_groupBox_Variables->setAlignment(Qt::AlignTop);
    nLatestVariableInput = 0;
    addVariableInput();
    addVariableInput();
    vVariableInputs[0]->setAxisMode(MODE_X_AXIS);
    vVariableInputs[1]->setAxisMode(MODE_Y_AXIS);
    vVariableInputs[0]->enableRemoveButton(false);
    vVariableInputs[1]->enableRemoveButton(false);
    // add original inequality input
    ui->layout_Inequality->setAlignment(Qt::AlignTop);
    ui->layout_groupBox_Inequalities->setAlignment(Qt::AlignTop);
    nLatestInequalityInput = 0;
    addInequalityInput();
    vInequalityInputs.front()->enablePositionButtons(false);
    // initialize global variables
    flag_Empty = true;
    flag_Combination = false;
    nPrevCombination = 0;
    sDefaultDirectory = "~/Documents/code/qt/proto/bare_minimum_plotter/";
}


//	Destructor
//	----------

BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}


//	Core
//	----


void BareMinimumPlotter::plot()
{
    clearFormatting();
    int nProgress = 0; // progress bar counter


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
        sErrorMessage =+ "Internal Check | Problem getting variables for horizontal and vertical axes. Please report this to:\n emerick.bosch+bugmail@gmail.com\n";
        printError();
        return;
    }

    //	evaluate and plot each inequality
    nGraphIndex = 0;
    nPrevCombination = 0;
    flag_Problem = false; // flag only causes return after all checks completed
    ui->plotter->clearGraphs();

    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        //	check skip
        if (vInequalityInputs[i]->getSkip())
            continue;

        nProgress = floor((0.0+i)/vInequalityInputs.size()*100);
        ui->progressBar->setFormat("Initializing...");

        // 	create inequality
        if(!vInequalityInputs[i]->createInequality()){
            printError();
            return;
        }


        // 	create and add variables
        nProgress = floor((0.1+i)/vInequalityInputs.size()*100);
        ui->progressBar->setFormat("Creating Variables...");
        for (int j = 0; j < static_cast<int>(vVariableInputs.size()); j++){
            if (!vVariableInputs[j]->checkInput()) {	// check legal
                flag_Problem = true;
                continue; // next check not needed
            }
            Variable tmpVariable = vVariableInputs[j]->getVariable();
            if(!vInequalityInputs[i]->addVariable(tmpVariable)){
                vVariableInputs[j]-> highlightName();
                flag_Problem = true;
            }
        }
        if (flag_Problem){
            printError();
            return;
        }

        nProgress = floor((0.1+i)/vInequalityInputs.size()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Evaluating...");

        //do math
        vInequalityInputs[i]->setXYVariables(mVariableX, mVariableY);
        if(!vInequalityInputs[i]->evaluate()){
            printError();
            return;
        }

        nProgress = floor((0.6+i)/vInequalityInputs.size()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Combining results...");

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

        nProgress = floor((0.8+i)/vInequalityInputs.size()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Plotting results...");

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
        if (!sUnitsX.empty())
            sUnitsX = " [" + sUnitsX + "]";
        if (!sUnitsY.empty())
            sUnitsY = " [" + sUnitsY + "]";

        ui->plotter->xAxis->setLabel(QString::fromStdString(mVariableX.getName() + sUnitsX));
        ui->plotter->yAxis->setLabel(QString::fromStdString(mVariableY.getName() + sUnitsY));
        ui->plotter->xAxis->setRange(mVariableX.getMin(), mVariableX.getMax());
        ui->plotter->yAxis->setRange(mVariableY.getMin(), mVariableY.getMax());
        ui->plotter->replot();

        printError();
        nProgress = floor((1+i)/vInequalityInputs.size()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Done.");
        flag_Empty = false;
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
        QCPScatterStyle style;
        //	- set marker
        switch(vInequalityInputs[i]->getShapeIndex()){
        case CROSS:
            style.setShape(QCPScatterStyle::ssCross);
            break;
        case CIRCLE:
            style.setShape(QCPScatterStyle::ssCircle);
            break;
        case TRIANGLE:
            style.setShape(QCPScatterStyle::ssTriangle);
            break;
        case SQUARE:
            style.setShape(QCPScatterStyle::ssSquare);
            break;
        }

        //	- set size
        style.setSize(5);
        //	- set color
        switch(vInequalityInputs[i]->getColorIndex()){
        case RED:
            style.setPen(QPen(Qt::red));
            break;
        case GREEN:
            style.setPen(QPen(Qt::green));
            break;
        case BLUE:
            style.setPen(QPen(Qt::blue));
            break;
        case DARK_RED:
            style.setPen(QPen(Qt::darkRed));
            break;
        case DARK_GREEN:
            style.setPen(QPen(Qt::darkGreen));
            break;
        case DARK_BLUE:
            style.setPen(QPen(Qt::darkBlue));
            break;
        case GREY:
            style.setPen(QPen(Qt::lightGray));
            break;
        case BLACK:
            style.setPen(QPen(Qt::black));
            break;
        }

        ui->plotter->graph(nGraphIndex)->setScatterStyle(style);
}

void BareMinimumPlotter::formatErrorGraph(){
            ui->plotter->graph(nGraphIndex)->setData(qvX_problem,qvY_problem);
            ui->plotter->graph(nGraphIndex)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
            QCPScatterStyle style;
            style.setShape(QCPScatterStyle::ssCross);
            style.setSize(5);
            style.setPen(QPen(Qt::red));
            ui->plotter->graph(nGraphIndex)->setScatterStyle(style);
}

//	Validation
//	----------
void BareMinimumPlotter::print(string sMessage){
    sErrorMessage += sMessage;
    ui->textEditError->setText(QString::fromStdString(sErrorMessage));
}

void BareMinimumPlotter::printclr(){
    sErrorMessage = "";
    ui->textEditError->setText(QString::fromStdString(sErrorMessage));
}

void BareMinimumPlotter::printError(){
    // show in display
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
        sErrorMessage += vInequalityInputs[i]->getErrors();
    }
    ui->textEditError->setText(QString::fromStdString(sErrorMessage));
    // show on progress bar
    ui->progressBar->setValue(100);
    ui->progressBar->setFormat("Error.");
    flag_Empty = true;
}

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
    vInequalityInputs.back()->setNumber(vInequalityInputs.size());
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
       qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget())->setNumber(i+1);
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

void BareMinimumPlotter::clearFormatting(){
    sErrorMessage = "";
    for (int i = 0; i < static_cast<int>(vInequalityInputs.size()); i++){
       vInequalityInputs[i]->clearFormatting();
    }
    for (vector<VariableInput*>::iterator it = vVariableInputs.begin(); it != vVariableInputs.end(); it++){
        (*it)->clearFormatting();
    }
    ui->progressBar->setValue(0);
}

void BareMinimumPlotter::clearGUI(){
    while (vInequalityInputs.size() > 1){
        removeInequalityInput(0);
    }
    while (vVariableInputs.size() > 2){
        removeVariableInput(0);
    }
    clearFormatting();
    vInequalityInputs.back()->clearFields();
    vVariableInputs.back()->clearFields();
    vVariableInputs.front()->clearFields();
    ui->plotter->clearGraphs();
    ui->plotter->replot();
    ui->plotter->xAxis->setLabel("");
    ui->plotter->yAxis->setLabel("");
}


void BareMinimumPlotter::save_JSON(QString filename){

    if (filename.isEmpty())
        return;
    ofstream outFile(filename.toStdString().c_str());
    if (outFile.is_open()){
        outFile << "{";	 // open file
        for (unsigned int i = 0; i < vInequalityInputs.size(); i++){ // for all inequalities
            // inequality
            outFile << "\"inequality\":{\n";
            // expression
            outFile << "\"expression\":" << vInequalityInputs[i]->toJSON() << ",\n";
            // variables
            outFile << "\"variables\":[\n";
            for(unsigned int j = 0; j < vVariableInputs.size(); j++){
                outFile << vVariableInputs[j]->toJSON();
                if (j != vVariableInputs.size() - 1)
                    outFile << ",";
                outFile << "\n";
            }
            outFile << "],\n";
            // plot data
            outFile << "\"plot\":[";
            QVector<double> X = vInequalityInputs[i]->getX();
            QVector<double> Y = vInequalityInputs[i]->getY();
            for (int j = 0; j < X.size(); j++){
                outFile << "{"
                           "\"x\":"<< X[j] << ","
                           "\"y\":" << Y[j] <<
                           "}";
                if (j != X.size()-1)
                    outFile << ",";
                outFile << "\n";
            }
            outFile << "]\n"; 	// plot
            outFile << "}"; 	// inequality
            if(i != vInequalityInputs.size()-1)
                outFile << ",\n";
        }
        outFile << "}"; 	// file
        outFile.close();
    }
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
            if (vInequalityInputs.size() < 2)
                vInequalityInputs.front()->enablePositionButtons(false);
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

void BareMinimumPlotter::menu_about(){
    QString title = "BareMinimumPlotter :: About";
    QString info = "<h1><b>B</b>are<b>M</b>inimum<b>P</b>lotter</h1><br/> \
                    Prototype - build 0.3<br/> \
                    <br/>\
                    Emerick Bosch<br/>\
                    June 2014<br/>\
                    <br/>\
                    Send questions and feedback to: <a href=\"emerick.bosch+bugmail@gmail.com\">bugmail</a>";

    QMessageBox::about(ui->centralWidget, title, info);
}

void BareMinimumPlotter::menu_open(){
    QString filename = QFileDialog::getOpenFileName(this, "Open plot", QString::fromStdString(sDefaultDirectory), "JSON (*.json)");
    if (filename.isEmpty())
        return;

    clearGUI();
    unsigned int nIneq = 0;

    string line;
    ifstream inFile(filename.toStdString().c_str());

    if(inFile.is_open()){
        while( getline(inFile, line) ){ // x value
            string token;
            stringstream ss;
            ss << line;
            while ( getline(ss,token, '"') ){
                if (token == "inequality"){
                    if ((nIneq + 1) > vInequalityInputs.size())
                        addInequalityInput();
                    nIneq++;
                }
                if (token == "expression"){
                    if(getline(ss, token,'{'))
                        if(getline(ss, token,'}'))
                            vInequalityInputs.back()->fromJSON(token);
                }
                if (token =="variables"){
                    unsigned int nVar = 0;
                    if(getline(ss, token, '['))
                        if(getline(inFile, token, ']')){
                            string var_token;
                            stringstream var_ss;
                            var_ss << token;
                            while ( getline (var_ss, var_token, '{')){
                                if ( getline (var_ss, var_token, '}')){
                                    if ((nVar+1) > vVariableInputs.size()){
                                        addVariableInput();
                                    }
                                    vVariableInputs[nVar]->fromJSON(var_token);
                                }
                                nVar++;
                            }
                        }
                }
            }
        }
    }

}

void BareMinimumPlotter::menu_saveAs(){
    if (flag_Empty){
        sErrorMessage = "Error | Input | Plot not complete. Cannot save.";
        printError();
        return;
    }
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString filename = dialog.getSaveFileName(this, "Save configuration", QString::fromStdString(sDefaultDirectory), "JSON (*.json)");
    save_JSON(filename);
}

void BareMinimumPlotter::menu_new(){
    clearGUI();
}

void BareMinimumPlotter::on_button_Plot_clicked() { plot(); }

void BareMinimumPlotter::on_button_AddVariable_clicked() { addVariableInput(); }

void BareMinimumPlotter::on_button_AddInequality_clicked() { addInequalityInput(); }
