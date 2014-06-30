/*	TASK LIST
    ---------
*/


///	Includes
/// ========

#include "bareminimumplotter.h"
#include "ui_bareminimumplotter.h"


///	Enumerated Types
///	=================

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


///	Namespaces
/// ===========

using namespace std;


///	Exprimental Functions
/// ======================

void elideLable(QLabel *label){
    QString text = label->text();
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
}


///	Public Functions
/// =================

//	Constructor
//	-----------

BareMinimumPlotter::BareMinimumPlotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BareMinimumPlotter),
    nPrevCombination (0),
    flag_Combination (false),
    nLatestVariableInput(0),
    nLatestInequalityInput(0),
    flag_Empty (true),
    sDefaultDirectory ("~/Documents/code/qt/proto/bare_minimum_plotter/")
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
    addVariableInput();
    addVariableInput();
    vVariableInputs[0]->setAxisMode(MODE_X_AXIS);
    vVariableInputs[1]->setAxisMode(MODE_Y_AXIS);
    vVariableInputs[0]->enableRemoveButton(false);
    vVariableInputs[1]->enableRemoveButton(false);
    // add original inequality input
    ui->layout_Inequality->setAlignment(Qt::AlignTop);
    ui->layout_groupBox_Inequalities->setAlignment(Qt::AlignTop);
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

void BareMinimumPlotter::plot()
{
    clearFormatting();
    int nProgress = 0; // progress bar counter

    // reload loaders (in case previous plotting changed its data through combinations)
    for (int i = 0; i < static_cast<int>(vInequalityLoaders.size()); i++){
        vInequalityLoaders[i]->loadCase(vInequalityLoaders[i]->getFile());
    }

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

    for (int i = 1; i < static_cast<int>(ui->layout_Inequality->count()+1); i++){
        //	check skip
        bool flag_skip;
        for (int j = 0; j < static_cast<int>(vInequalityInputs.size()); j++){
            if(vInequalityInputs[j]->getNumber() == i){
                if (vInequalityInputs[j]->getSkip()) { flag_skip = true; break; }
                plotNew(j, sUnitsX, sUnitsY, nProgress);
            }
        }
        for (int j = 0; j < static_cast<int>(vInequalityLoaders.size()); j++){
            if(vInequalityLoaders[j]->getNumber() == i){
                if (vInequalityLoaders[j]->getSkip()) { flag_skip = true; break; }
                plotOld(j, sUnitsX, sUnitsY, nProgress);
            }
        }
        if (flag_skip)
            continue;

    }
}

//	TODO: nProgress might roll back (nGraphIndex will stay the same for combined graphs)
void BareMinimumPlotter::plotNew(int nIneq, string sUnitsX, string sUnitsY, int nProgress){
        nProgress = floor((0.0+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setFormat("Initializing...");
        // 	create inequality
        if(!vInequalityInputs[nIneq]->createInequality()){
            printError();
            return;
        }
        // 	create and add variables
        nProgress = floor((0.1+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setFormat("Creating Variables...");
        for (int j = 0; j < static_cast<int>(vVariableInputs.size()); j++){
            if (!vVariableInputs[j]->checkInput()) {	// check legal
                flag_Problem = true;
                return; // next check not needed
            }
            Variable tmpVariable = vVariableInputs[j]->getVariable();
            if(!vInequalityInputs[nIneq]->addVariable(tmpVariable)){
                vVariableInputs[j]-> highlightName();
                flag_Problem = true;
            }
        }
        if (flag_Problem){
            printError();
            return;
        }
        nProgress = floor((0.1+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Evaluating...");
        //do math
        vInequalityInputs[nIneq]->setXYVariables(mVariableX, mVariableY);
        if(!vInequalityInputs[nIneq]->evaluate()){
            printError();
            return;
        }

        nProgress = floor((0.6+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Combining results...");

        // get plotting vectors
        switch(nPrevCombination){
        case COMBINE_NONE:
            vectorCombineNone(vInequalityInputs[nIneq]->getNumber()-1);
            break;
        case COMBINE_INTERSECTION:
            vectorCombineIntersection(vInequalityInputs[nIneq]->getNumber()-1);
            break;
        case COMBINE_UNION:
            vectorCombineUnion(vInequalityInputs[nIneq]->getNumber()-1);
            break;
        case COMBINE_SUBTRACTION:
            vectorCombineSubtraction(vInequalityInputs[nIneq]->getNumber()-1);
            break;
        }
        nPrevCombination = vInequalityInputs[nIneq]->getCombination();
        if (nPrevCombination != COMBINE_NONE)
            return;

        nProgress = floor((0.8+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Plotting results...");

        // add normal graph
        ui->plotter->addGraph();
        formatGraph(vInequalityInputs[nIneq]->getShapeIndex(), vInequalityInputs[nIneq]->getColorIndex());
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
        nProgress = floor((1+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Done.");
        flag_Empty = false;

}


void BareMinimumPlotter::plotOld(int nIneq, string sUnitsX, string sUnitsY, int nProgress){
    vInequalityLoaders[nIneq]->beginPlot();

    while (!vInequalityLoaders[nIneq]->isEnd()){
        nProgress = floor((0.6+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Combining results...");

        // get plotting vectors
        switch(nPrevCombination){
        case COMBINE_NONE:
            vectorCombineNone(vInequalityLoaders[nIneq]->getNumber()-1);
            break;
        case COMBINE_INTERSECTION:
            vectorCombineIntersection(vInequalityLoaders[nIneq]->getNumber()-1);
            break;
        case COMBINE_UNION:
            vectorCombineUnion(vInequalityLoaders[nIneq]->getNumber()-1);
            break;
        case COMBINE_SUBTRACTION:
            vectorCombineSubtraction(vInequalityLoaders[nIneq]->getNumber()-1);
            break;
        }
        nPrevCombination = vInequalityLoaders[nIneq]->getCombination();
        if (nPrevCombination != COMBINE_NONE)
            return;

        nProgress = floor((0.8+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Plotting results...");

        // add normal graph
        ui->plotter->addGraph();
        formatGraph(vInequalityLoaders[nIneq]->getShapeIndex(), vInequalityLoaders[nIneq]->getColorIndex());
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
        nProgress = floor((1+nGraphIndex)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(nProgress);
        ui->progressBar->setFormat("Done.");
        flag_Empty = false;

        vInequalityLoaders[nIneq]->nextPlot();
    }
}

void BareMinimumPlotter::vectorCombineNone(int nIndex){
    if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "input"){
        InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvX = mCurrentInequality->getX();
        qvY = mCurrentInequality->getY();
        qvX_problem = mCurrentInequality->getXProblem();
        qvY_problem = mCurrentInequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvX = mCurrentInequality->getX();
        qvY = mCurrentInequality->getY();
//        qvX_problem = mCurrentInequality->getXProblem();
//        qvY_problem = mCurrentInequality->getYProblem();
    }
}

void BareMinimumPlotter::vectorCombineIntersection(int nIndex){
    flag_Combination = true;
    QVector<double> qvXOld = qvX;
    QVector<double> qvYOld = qvY;
    QVector<double> qvXNew;
    QVector<double> qvYNew;

    if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "input"){
        InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
        qvX_problem = mCurrentInequality->getXProblem();
        qvY_problem = mCurrentInequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
//        qvX_problem = mCurrentInequality->getXProblem();
//        qvY_problem = mCurrentInequality->getYProblem();
    }

    qvX.clear();
    qvY.clear();

    for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
       for (int j = 0; j < static_cast<int>(qvXNew.size()); j++) {
           if (Expression::approxEqual(qvXOld[i], qvXNew[j], APPROX_EQUAL_PRECISION) && Expression::approxEqual(qvYOld[i], qvYNew[j], APPROX_EQUAL_PRECISION)) {
               qvX.push_back(qvXOld[i]);
               qvY.push_back(qvYOld[i]);
           }
       }
    }

    if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "input"){
        InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        if (mCurrentInequality->getCombination() == COMBINE_NONE){
            mCurrentInequality->setX(qvX);
            mCurrentInequality->setY(qvY);
        }
    }
    else if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        if (mCurrentInequality->getCombination() == COMBINE_NONE){
            mCurrentInequality->setX(qvX);
            mCurrentInequality->setY(qvY);
        }
    }

}

// [TODO] prevent user from changing '-' in last inequality.
// [TODO] progress bar
void BareMinimumPlotter::vectorCombineUnion(int nIndex){
    QVector<double> qvXOld = qvX;
    QVector<double> qvYOld = qvY;
    QVector<double> qvXNew;
    QVector<double> qvYNew;

    if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "input"){
        InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
        qvX_problem = mCurrentInequality->getXProblem();
        qvY_problem = mCurrentInequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
//        qvX_problem = mCurrentInequality->getXProblem();
//        qvY_problem = mCurrentInequality->getYProblem();
    }

    qvX.clear();
    qvY.clear();

    for (mVariableX.resetPosition(); !mVariableX.isEnd(); mVariableX.nextPosition()){
        for (mVariableY.resetPosition(); !mVariableY.isEnd(); mVariableY.nextPosition()){
            for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
                if (Expression::approxEqual(mVariableX.getCurrentValue(), qvXOld[i], APPROX_EQUAL_PRECISION) && Expression::approxEqual(mVariableY.getCurrentValue(), qvYOld[i], APPROX_EQUAL_PRECISION)){
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

void BareMinimumPlotter::vectorCombineSubtraction(int nIndex){
    flag_Combination = true;
    QVector<double> qvXOld = qvX;
    QVector<double> qvYOld = qvY;
    QVector<double> qvXNew;
    QVector<double> qvYNew;

    if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "input"){
        InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
        qvX_problem = mCurrentInequality->getXProblem();
        qvY_problem = mCurrentInequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(nIndex)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nIndex)->widget());
        qvXNew = mCurrentInequality->getX();
        qvYNew = mCurrentInequality->getY();
//        qvX_problem = mCurrentInequality->getXProblem();
//        qvY_problem = mCurrentInequality->getYProblem();
    }

    qvX.clear();
    qvY.clear();
    bool flag_keep;

    for (int i = 0; i < static_cast<int>(qvXOld.size()); i++){
       flag_keep = true;
       for (int j = 0; j < static_cast<int>(qvXNew.size()); j++) {
           if ((Expression::approxEqual(qvXOld[i], qvXNew[j], APPROX_EQUAL_PRECISION) && Expression::approxEqual(qvYOld[i], qvYNew[j], APPROX_EQUAL_PRECISION))) {
               flag_keep = false;
           }
       }
       if (flag_keep){
           qvX.push_back(qvXOld[i]);
           qvY.push_back(qvYOld[i]);
       }
    }
}

void BareMinimumPlotter::formatGraph(int nShape, int nColor){
        ui->plotter->graph(nGraphIndex)->setData(qvX,qvY);
        ui->plotter->graph(nGraphIndex)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
        QCPScatterStyle style;
        //	- set marker
        switch(nShape){
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
        switch(nColor){
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
    ui->layout_Inequality->addWidget(vInequalityInputs.back());
    vInequalityInputs.back()->setNumber(ui->layout_Inequality->count());
    // connect slots to signals
    QObject::connect(vInequalityInputs.back(), SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(vInequalityInputs.back(), SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    // enable/disable combination menu
    setCombinationInputs();
}

void BareMinimumPlotter::addInequalityLoader(){
    vInequalityLoaders.push_back(new InequalityLoader);
    ui->layout_Inequality->addWidget(vInequalityLoaders.back());
    vInequalityLoaders.back()->setNumber(ui->layout_Inequality->count());
    // load data
    QString filename = QFileDialog::getOpenFileName(this, "Open plot", ".", "JSON (*.json)");
    vInequalityLoaders.back()->loadCase(filename.toStdString());
    //	connect slots to signals
    QObject::connect(vInequalityLoaders.back(), SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(vInequalityLoaders.back(), SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(vInequalityLoaders.back(), SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    // enable/disable combination menu
    setCombinationInputs();
}

void BareMinimumPlotter::reOrderInequalityInputs(){
    vector<InequalityInput*> tmpVecInput;
    vector<InequalityLoader*> tmpVecLoader;
    for (int i = 0; i < static_cast<int>(ui->layout_Inequality->count()); i++){
        if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "input"){
           tmpVecInput.push_back(qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget()));
           qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget())->setNumber(i+1);
        }
        if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "loader"){
           tmpVecLoader.push_back(qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(i)->widget()));
           qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(i)->widget())->setNumber(i+1);
        }
    }
    vInequalityInputs = tmpVecInput;
    vInequalityLoaders = tmpVecLoader;
}

// [BREAK] 30 June 2014 | fixing enabled/disabled buttons when adding and removing inequalities
void BareMinimumPlotter::setCombinationInputs(){
    int nSize = ui->layout_Inequality->count()-1;
    //	TODO: make this more elegant
    // disable the combinations of last item
    if(ui->layout_Inequality->itemAt(nSize)->widget()->accessibleDescription() == "input"){
        qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize)->widget())->enableCombinations(false);
        qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize)->widget())->resetCombinations();
    }
    if(ui->layout_Inequality->itemAt(nSize)->widget()->accessibleDescription() == "loader"){
        qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nSize)->widget())->enableCombinations(false);
        qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nSize)->widget())->resetCombinations();
    }
    // enable combinations of second last item
    if (ui->layout_Inequality->count() > 1){
        if(ui->layout_Inequality->itemAt(nSize-1)->widget()->accessibleDescription() == "input")
            qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(nSize-1)->widget())->enableCombinations(true);
        if(ui->layout_Inequality->itemAt(nSize-1)->widget()->accessibleDescription() == "loader")
            qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(nSize-1)->widget())->enableCombinations(true);
    }
    // disable movement buttons if only 1 widget in layout
    if (ui->layout_Inequality->count() < 2){
        if(ui->layout_Inequality->itemAt(0)->widget()->accessibleDescription() == "input")
            vInequalityInputs.front()->enablePositionButtons(false);
        else if(ui->layout_Inequality->itemAt(0)->widget()->accessibleDescription() == "loader")
            vInequalityLoaders.front()->enablePositionButtons(false);
    }
    // enable position buttons if more than 1 widget
    if (ui->layout_Inequality->count() > 1){
        if(ui->layout_Inequality->itemAt(nSize)->widget()->accessibleDescription() == "input")
            vInequalityInputs.front()->enablePositionButtons(true);
        if(ui->layout_Inequality->itemAt(nSize)->widget()->accessibleDescription() == "loader")
            vInequalityLoaders.front()->enablePositionButtons(true);
    }
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
    while (ui->layout_Inequality->count() > 1){
        removeInequalityInput(1);
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


//	Parsing and File IO
//	--------------------

void BareMinimumPlotter::save_JSON(QString filename){

    if (filename.isEmpty())
        return;
    ofstream outFile(filename.toStdString().c_str());
    if (outFile.is_open()){
        outFile << "{";	 // beginning of file
        for (int i = 0; i < ui->layout_Inequality->count(); i++){
            if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "input"){
                InequalityInput *mCurrentInequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget());
                // inequality
                outFile << "\"inequality\":{\n";
                // expression
                outFile << "\"expression\":" << mCurrentInequality->toJSON() << ",\n";
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
                if (mCurrentInequality->getCombination() == COMBINE_NONE){ // save only combined results
                    outFile << "\"plot\":[";
                    QVector<double> X = mCurrentInequality->getX();
                    QVector<double> Y = mCurrentInequality->getY();
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
                }
                outFile << "}"; 	// inequality
                if(i != vInequalityInputs.size()-1)
                    outFile << ",\n";
            }
            if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "loader"){
                InequalityLoader *mCurrentInequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(i)->widget());
                outFile << mCurrentInequality->toJSON();
            }
        }
        outFile << "}"; 	// file
        outFile.close();
    }
}


///	Public Slots
/// =============

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
    if (ui->layout_Inequality->count() < 2) // at least 1 inequality input needed
        return;

    // 	Using for loop to find exact element within the vector to delete
    //	else, would have used the qobjectcast method

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
    for (int i = 0; i < static_cast<int>(vInequalityLoaders.size()); i++){
        if (vInequalityLoaders[i]->getNumber() == nInequalityNumber){
            ui->layout_Inequality->removeWidget(vInequalityLoaders[i]);
            delete vInequalityLoaders[i];
            if (i < static_cast<int>(vInequalityLoaders.size()))
                vInequalityLoaders.erase(vInequalityLoaders.begin()+i);
            // enable/disable position buttons
            reOrderInequalityInputs();
            setCombinationInputs();
            return;
        }
    }

}

void BareMinimumPlotter::moveInequalityInputUp (int nInequalityNumber){
    int nCurrentPos = nInequalityNumber-1;
    if (ui->layout_Inequality->itemAt(nCurrentPos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < vInequalityInputs.size(); i++){
            if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
                if (nCurrentPos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
                ui->layout_Inequality->insertWidget(--nCurrentPos, vInequalityInputs[i]);
                reOrderInequalityInputs();
                setCombinationInputs();
            }
        }
    if (ui->layout_Inequality->itemAt(nCurrentPos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < vInequalityLoaders.size(); i++){
            if (vInequalityLoaders[i]->getNumber() == nInequalityNumber){
                if (nCurrentPos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(vInequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(--nCurrentPos, vInequalityLoaders[i]);
                reOrderInequalityInputs();
                setCombinationInputs();
            }
        }
}

void BareMinimumPlotter::moveInequalityInputDown (int nInequalityNumber){
    int nCurrentPos = nInequalityNumber-1;
    if (ui->layout_Inequality->itemAt(nCurrentPos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < vInequalityInputs.size(); i++){
            if (vInequalityInputs[i]->getNumber() == nInequalityNumber){
                if (nCurrentPos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(vInequalityInputs[i]);
                ui->layout_Inequality->insertWidget(++nCurrentPos, vInequalityInputs[i]);
                reOrderInequalityInputs();
                setCombinationInputs();
                return;
            }
        }
    if (ui->layout_Inequality->itemAt(nCurrentPos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < vInequalityLoaders.size(); i++){
            if (vInequalityLoaders[i]->getNumber() == nInequalityNumber){
                if (nCurrentPos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(vInequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(++nCurrentPos, vInequalityLoaders[i]);
                reOrderInequalityInputs();
                setCombinationInputs();
                return;
            }
        }
}


///	Private Slots
/// ==============

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

void BareMinimumPlotter::menu_new(){ clearGUI(); }

void BareMinimumPlotter::on_button_Plot_clicked() { plot(); }

void BareMinimumPlotter::on_button_AddVariable_clicked() { addVariableInput(); }

void BareMinimumPlotter::on_button_AddInequality_clicked() { addInequalityInput(); }

void BareMinimumPlotter::on_pushButton_AddInequalityLoader_clicked() { addInequalityLoader(); }
