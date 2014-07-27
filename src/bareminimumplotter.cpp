/*	TASK LIST
    ---------
*/


///	Includes
///	========

#include "include/bareminimumplotter.h"
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
///	===========

using namespace std;


///	Static Functions
///	======================

void elideLable(QLabel *label, QString text){
    QFontMetrics metrics(label->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, label->width());
    label->setText(elidedText);
}

void fitLineEditToContents(QLineEdit* edit){
    QString text = edit->text();
    QFontMetrics fm = edit->fontMetrics();
    int width = fm.boundingRect(text).width();
    edit->setMinimumWidth(width+10);
    edit->setMaximumWidth(width+10);
}


///	Public Functions
/// =================

//	Constructor
//	-----------

BareMinimumPlotter::BareMinimumPlotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BareMinimumPlotter),
    m_Title("untitled_case"),
    m_prev_combination (0),
    flag_Combination (false),
    m_variable_count(0),
    m_inequality_count(0),
    flag_Saved(true),
    flag_Empty (true)
{
    ui->setupUi(this);

    //	SETUP UI CONTINUED
    //	general - things that can't be done in designer
    ui->verticalLayout_VariableButtons->setAlignment(Qt::AlignVCenter);
    ui->verticalLayout_InequalityButtons->setAlignment(Qt::AlignVCenter);
    QDoubleValidator *dValidator = new QDoubleValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEdit_SettingsTolerance->setValidator(dValidator);

    //	make plot window fill the tab on startup
    resetPlotWindow();

    //	account for scrollbar sizes in ui
    int new_size = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QSpacerItem *scroll_bar_spacer = ui->horizontalSpacer_VariableScrollBar;
    QSize old_size = scroll_bar_spacer->sizeHint();
    scroll_bar_spacer->changeSize(new_size, old_size.height());
    ui->horizontalSpacer_InequalityScrollBar->changeSize(new_size, old_size.height());

    //	buttons
    ui->toolButton_AddInequality->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddInequality->setIcon(QPixmap("../bare_minimum_plotter/rsc/add-cross-white.png"));
    ui->toolButton_AddInequality->setIconSize(QSize(22,22));
    ui->toolButton_AddInequalityLoader->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddInequalityLoader->setIcon(QPixmap("../bare_minimum_plotter/rsc/load-white.png"));
    ui->toolButton_AddInequalityLoader->setIconSize(QSize(30,22));
    ui->toolButton_AddVariable->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddVariable->setIcon(QPixmap("../bare_minimum_plotter/rsc/add-cross-white.png"));
    ui->toolButton_AddVariable->setIconSize(QSize(22,22));

    //	disable splitter handles for inequality
    for (int i = 0; i < ui->splitter_Inequality->count(); i++){
        ui->splitter_Inequality->handle(i)->setEnabled(false);
    }

    //	LOAD SETTINGS
    //	tolerance
    stringstream buffer;
    buffer << ui->lineEdit_SettingsTolerance->text().toStdString();
    if (!(buffer >> m_tolerance))
        m_tolerance = 0;

    //	directory
    m_default_directory = QDir::currentPath().toStdString();

    // 	INITIAL SETUP
    // 	add original x & y variable inputs
    ui->layout_Variable->setAlignment(Qt::AlignTop);
    addVariableInput();
    addVariableInput();
    m_VariableInputs[0]->setAxisMode(MODE_X_AXIS);
    m_VariableInputs[1]->setAxisMode(MODE_Y_AXIS);
    m_VariableInputs[0]->enableRemoveButton(false);
    m_VariableInputs[1]->enableRemoveButton(false);
    m_VariableInputs[0]->setSplitterSizes(ui->splitter_Variable->sizes());
    m_VariableInputs[1]->setSplitterSizes(ui->splitter_Variable->sizes());

    // 	add original inequality input
    ui->layout_Inequality->setAlignment(Qt::AlignTop);
    addInequalityInput();
    m_InequalityInputs.front()->enablePositionButtons(false);


    flag_Saved = true;
}


//	Destructor
//	----------

BareMinimumPlotter::~BareMinimumPlotter()
{
    delete ui;
}


//	Overrides
//	-----------

void BareMinimumPlotter::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    //	gui things that can only be done once gui is shown

    //	align splitters
    variableSplitterMoved(ui->splitter_Variable->sizes());
    inequalitySplitterMoved(ui->splitter_Inequality->sizes());
}


//	Core
//	----

void BareMinimumPlotter::plot()
{
    clearFormatting();
    int progress = 0; // progress bar counter

    // reload loaders (in case previous plotting changed its data through combinations)
    for (int i = 0; i < static_cast<int>(m_InequalityLoaders.size()); i++){
        m_InequalityLoaders[i]->loadCase(m_InequalityLoaders[i]->getFile());
    }

    // 	determine plotting variables
    string x_units, y_units;
    int check = 0;
    for (int i = 0; i < static_cast<int>(m_VariableInputs.size()); i++){
        VariableInput *varinput = m_VariableInputs[i];
        if (varinput->getAxisMode() == MODE_X_AXIS){
            m_XVariable = varinput->getVariable();
            x_units = varinput-> getUnits();
            check++;
        } else if (varinput->getAxisMode() == MODE_Y_AXIS){
            m_YVariable = varinput->getVariable();
            y_units = varinput-> getUnits();
            check++;
        }
    }
    if (check != 2){
        m_error_message =+ "Internal Check | Problem getting variables for horizontal and vertical axes. Please report this to:\n emerick.bosch+bugmail@gmail.com\n";
        printError();
        return;
    }

    // legend
    ui->plotter->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->plotter->legend->setSelectableParts(QCPLegend::spLegendBox | QCPLegend::spItems);
    ui->plotter->legend->setVisible(true);

    //	evaluate and plot each inequality
    m_graph_count = 0;
    m_prev_combination = 0;
    flag_Problem = false; // flag only causes return after all checks completed
    ui->plotter->clearGraphs();

    for (int i = 0; i < static_cast<int>(ui->layout_Inequality->count()+1); i++){
        //	check skip
        bool flag_skip;
        for (int j = 0; j < static_cast<int>(m_InequalityInputs.size()); j++){
            if(m_InequalityInputs[j]->getNumber() == i){
                if (m_InequalityInputs[j]->getSkip()) { flag_skip = true; break; }
                plotNew(j, x_units, y_units, progress);
            }
        }
        for (int j = 0; j < static_cast<int>(m_InequalityLoaders.size()); j++){
            if(m_InequalityLoaders[j]->getNumber() == i){
                if (m_InequalityLoaders[j]->getSkip()) { flag_skip = true; break; }
                plotOld(j, x_units, y_units, progress);
            }
        }
        if (flag_skip)
            continue;

    }
    ui->tabWidget->setCurrentIndex(1);
    flag_Saved = false;
}

//	TODO: nProgress might roll back (m_graph_count will stay the same for combined graphs)
void BareMinimumPlotter::plotNew(int gui_number, string x_units, string y_units, int progress)
{
        InequalityInput *input = m_InequalityInputs[gui_number];
        progress = floor((0.0+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setFormat("Initializing...");
        // 	create inequality
        if(!input->createInequality()){
            printError();
            return;
        }
        // 	create and add variables
        progress = floor((0.1+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setFormat("Creating Variables...");
        for (int j = 0; j < static_cast<int>(m_VariableInputs.size()); j++){
            if (!m_VariableInputs[j]->checkInput()) {	// check legal
                flag_Problem = true;
                return; // next check not needed
            }
            Variable tmpVariable = m_VariableInputs[j]->getVariable();
            if(!input->addVariable(tmpVariable)){
                m_VariableInputs[j]-> highlightName();
                flag_Problem = true;
            }
        }
        if (flag_Problem){
            printError();
            return;
        }
        progress = floor((0.1+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(progress);
        ui->progressBar->setFormat("Evaluating...");
        //do math
        input->setXYVariables(m_XVariable, m_YVariable);
        if(!input->evaluate()){
            printError();
            return;
        }

        progress = floor((0.6+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(progress);
        ui->progressBar->setFormat("Combining results...");

        // get plotting vectors
        switch(m_prev_combination){
        case CombinationNone:
            vectorCombineNone(input->getNumber());
            break;
        case CombinationIntersect:
            vectorCombineIntersection(input->getNumber());
            break;
        case CombinationUnion:
            vectorCombineUnion(input->getNumber());
            break;
        case CombinationSubtract:
            vectorCombineSubtraction(input->getNumber());
            break;
        }
        m_prev_combination = input->getCombination();
        if (m_prev_combination != CombinationNone)
            return;

        progress = floor((0.8+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(progress);
        ui->progressBar->setFormat("Plotting results...");

        // add normal graph
        ui->plotter->addGraph();
        formatGraph(input->getShapeIndex(), input->getColorIndex());
        m_graph_count++;

        // add problem graph (if needed)
        if (!x_results_problem.isEmpty()){
            ui->plotter->addGraph();
            formatErrorGraph();
            m_graph_count++;
        }

        // set general options, plot
        if (!x_units.empty())
            x_units = " [" + x_units + "]";
        if (!y_units.empty())
            y_units = " [" + y_units + "]";

        ui->plotter->xAxis->setLabel(QString::fromStdString(m_XVariable.getName() + x_units));
        ui->plotter->yAxis->setLabel(QString::fromStdString(m_YVariable.getName() + y_units));
        ui->plotter->xAxis->setRange(m_XVariable.getMin(), m_XVariable.getMax());
        ui->plotter->yAxis->setRange(m_YVariable.getMin(), m_YVariable.getMax());
        ui->plotter->replot();




        printError();
        progress = floor((1+m_graph_count)/ui->layout_Inequality->count()*100);
        ui->progressBar->setValue(progress);
        ui->progressBar->setFormat("Done.");
        flag_Empty = false;

}


void BareMinimumPlotter::plotOld(int gui_number, string x_units, string y_units, int progress)
{
    InequalityLoader* loader = m_InequalityLoaders[gui_number];
    loader->setPlot();

    progress = floor((0.6+m_graph_count)/ui->layout_Inequality->count()*100);
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat("Combining results...");

    // get plotting vectors
    switch(m_prev_combination){
    case CombinationNone:
        vectorCombineNone(loader->getNumber());
        break;
    case CombinationIntersect:
        vectorCombineIntersection(loader->getNumber());
        break;
    case CombinationUnion:
        vectorCombineUnion(loader->getNumber());
        break;
    case CombinationSubtract:
        vectorCombineSubtraction(loader->getNumber());
        break;
    }
    m_prev_combination = loader->getCombination();
    if (m_prev_combination != CombinationNone)
        return;

    progress = floor((0.8+m_graph_count)/ui->layout_Inequality->count()*100);
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat("Plotting results...");

    // add normal graph
    ui->plotter->addGraph();
    formatGraph(loader->getShapeIndex(), loader->getColorIndex());
    m_graph_count++;

    // add problem graph (if needed)
    if (!x_results_problem.isEmpty()){
        ui->plotter->addGraph();
        formatErrorGraph();
        m_graph_count++;
    }

    // set general options, plot
    if (!x_units.empty())
        x_units = " [" + x_units + "]";
    if (!y_units.empty())
        y_units = " [" + y_units + "]";

    ui->plotter->xAxis->setLabel(QString::fromStdString(m_XVariable.getName() + x_units));
    ui->plotter->yAxis->setLabel(QString::fromStdString(m_YVariable.getName() + y_units));
    ui->plotter->xAxis->setRange(m_XVariable.getMin(), m_XVariable.getMax());
    ui->plotter->yAxis->setRange(m_YVariable.getMin(), m_YVariable.getMax());
    ui->plotter->replot();

    printError();
    progress = floor((1+m_graph_count)/ui->layout_Inequality->count()*100);
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat("Done.");
    flag_Empty = false;

}

void BareMinimumPlotter::vectorCombineNone(int gui_number)
{
    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_results = current_inequality->getX();
        y_results = current_inequality->getY();
        x_results_problem = current_inequality->getXProblem();
        y_results_problem = current_inequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_results = current_inequality->getX();
        y_results = current_inequality->getY();
//        x_results_problem = current_inequality->getXProblem();
//        y_results_problem = current_inequality->getYProblem();
    }
}

void BareMinimumPlotter::vectorCombineIntersection(int gui_number)
{
    flag_Combination = true;
    QVector<double> x_resultsOld = x_results;
    QVector<double> y_resultsOld = y_results;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
        x_results_problem = current_inequality->getXProblem();
        y_results_problem = current_inequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
//        x_results_problem = current_inequality->getXProblem();
//        y_results_problem = current_inequality->getYProblem();
    }

    x_results.clear();
    y_results.clear();

    for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
       for (int j = 0; j < static_cast<int>(x_resultsNew.size()); j++) {
           // ApproxEqualPrecision is in case you are comparing results of different precisions.
           if (Expression::approxEqual(x_resultsOld[i], x_resultsNew[j], m_tolerance) && Expression::approxEqual(y_resultsOld[i], y_resultsNew[j], m_tolerance)) {
               x_results.push_back(x_resultsOld[i]);
               y_results.push_back(y_resultsOld[i]);
           }
       }
    }

    // save the combined result vector as the results of the last inequality in the chain
    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }

}

// [TODO] prevent user from changing '-' in last inequality.
// [TODO] progress bar
void BareMinimumPlotter::vectorCombineUnion(int gui_number)
{
    QVector<double> x_resultsOld = x_results;
    QVector<double> y_resultsOld = y_results;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
        x_results_problem = current_inequality->getXProblem();
        y_results_problem = current_inequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
//        x_results_problem = current_inequality->getXProblem();
//        y_results_problem = current_inequality->getYProblem();
    }

    x_results.clear();
    y_results.clear();

    for (m_XVariable.resetPosition(); !m_XVariable.isEnd(); m_XVariable.nextPosition()){
        for (m_YVariable.resetPosition(); !m_YVariable.isEnd(); m_YVariable.nextPosition()){
            for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
                if (Expression::approxEqual(m_XVariable.getCurrentValue(), x_resultsOld[i], m_tolerance) && Expression::approxEqual(m_YVariable.getCurrentValue(), y_resultsOld[i], m_tolerance)){
                    x_results.push_back(m_XVariable.getCurrentValue());
                    y_results.push_back(m_YVariable.getCurrentValue());
                }
            }
            for (int i = 0; i < static_cast<int>(x_resultsNew.size()); i++){
                if ((m_XVariable.getCurrentValue() == x_resultsNew[i]) && (m_YVariable.getCurrentValue() == y_resultsNew[i])){
                    x_results.push_back(m_XVariable.getCurrentValue());
                    y_results.push_back(m_YVariable.getCurrentValue());
                }
            }
        }
    }

    // save the combined result vector as the results of the last inequality in the chain
    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }
}

void BareMinimumPlotter::vectorCombineSubtraction(int gui_number)
{
    flag_Combination = true;
    QVector<double> x_resultsOld = x_results;
    QVector<double> y_resultsOld = y_results;
    QVector<double> x_resultsNew;
    QVector<double> y_resultsNew;

    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
        x_results_problem = current_inequality->getXProblem();
        y_results_problem = current_inequality->getYProblem();
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        x_resultsNew = current_inequality->getX();
        y_resultsNew = current_inequality->getY();
//        x_results_problem = current_inequality->getXProblem();
//        y_results_problem = current_inequality->getYProblem();
    }

    x_results.clear();
    y_results.clear();
    bool flag_keep;

    for (int i = 0; i < static_cast<int>(x_resultsOld.size()); i++){
       flag_keep = true;
       for (int j = 0; j < static_cast<int>(x_resultsNew.size()); j++) {
           if ((Expression::approxEqual(x_resultsOld[i], x_resultsNew[j], m_tolerance) && Expression::approxEqual(y_resultsOld[i], y_resultsNew[j], m_tolerance))) {
               flag_keep = false;
           }
       }
       if (flag_keep){
           x_results.push_back(x_resultsOld[i]);
           y_results.push_back(y_resultsOld[i]);
       }
    }

    // save the combined result vector as the results of the last inequality in the chain
    if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "input"){
        InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }
    else if (ui->layout_Inequality->itemAt(gui_number)->widget()->accessibleDescription() == "loader"){
        InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(gui_number)->widget());
        if (current_inequality->getCombination() == CombinationNone){
            current_inequality->setX(x_results);
            current_inequality->setY(y_results);
        }
    }
}

void BareMinimumPlotter::formatGraph(int shape, int color)
{
    QCPGraph *plot = ui->plotter->graph(m_graph_count);
        plot->setData(x_results,y_results);
        plot->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
        QCPScatterStyle style;
        //	- set marker
        switch(shape){
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
        switch(color){
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

        //	[BREAK] 25 July 2014 | working on graph interactions
        QPen selected_pen;
        selected_pen.setWidth(style.pen().width() + 2);
        selected_pen.setColor(style.pen().color());
        plot->setScatterStyle(style);
        plot->setSelectable(true);
        plot->setSelectedPen(selected_pen);
        ui->plotter->setInteraction(QCP::iSelectLegend, true);
        ui->plotter->setInteraction(QCP::iSelectPlottables, true);
        ui->plotter->setInteraction(QCP::iSelectItems, true);
}

void BareMinimumPlotter::formatErrorGraph()
{
            ui->plotter->graph(m_graph_count)->setData(x_results_problem,y_results_problem);
            ui->plotter->graph(m_graph_count)->setLineStyle(QCPGraph::LineStyle(QCPGraph::lsNone));
            QCPScatterStyle style;
            style.setShape(QCPScatterStyle::ssCross);
            style.setSize(5);
            style.setPen(QPen(Qt::red));
            ui->plotter->graph(m_graph_count)->setScatterStyle(style);
}

//	Validation
//	----------
void BareMinimumPlotter::print(string message)
{
    m_error_message += message;
    ui->textEdit_Error->setText(QString::fromStdString(m_error_message));
}

void BareMinimumPlotter::printclr()
{
    m_error_message = "";
    ui->textEdit_Error->setText(QString::fromStdString(m_error_message));
}

void BareMinimumPlotter::printError()
{
    // show in display
    for (int i = 0; i < static_cast<int>(m_InequalityInputs.size()); i++){
        m_error_message += m_InequalityInputs[i]->getErrors();
    }
    for (int i = 0; i < static_cast<int>(m_InequalityLoaders.size()); i++){
        m_error_message += m_InequalityLoaders[i]->getErrors();
    }

    ui->textEdit_Error->setText(QString::fromStdString(m_error_message));
    // show on progress bar
    ui->progressBar->setValue(100);
    ui->progressBar->setFormat("Error.");
    flag_Empty = true;
}

//	GUI
//	---

void BareMinimumPlotter::clearGUI()
{
    addInequalityInput();
    while (ui->layout_Inequality->count() > 1){
        removeInequalityInput(0);
    }
    while (m_VariableInputs.size() > 2){
        removeVariableInput(0);
    }
    clearFormatting();
    m_VariableInputs.back()->clearFields();
    m_VariableInputs.front()->clearFields();
    resetPlotWindow();
}

void BareMinimumPlotter::clearFormatting()
{
    m_error_message = "";
    for (int i = 0; i < static_cast<int>(m_InequalityInputs.size()); i++){
       m_InequalityInputs[i]->clearFormatting();
    }
    for (vector<VariableInput*>::iterator it = m_VariableInputs.begin(); it != m_VariableInputs.end(); it++){
        (*it)->clearFormatting();
    }
    ui->progressBar->setValue(0);
}

void BareMinimumPlotter::resetPlotWindow()
{
    ui->plotter->clearGraphs();
    ui->plotter->replot();
    ui->plotter->xAxis->setLabel("");
    ui->plotter->yAxis->setLabel("");

    int winwidth = ui->tab_plot->width();
    int winheight = ui->tab_plot->height();
    QList<int> horsizes, versizes;
    horsizes << 0 << winwidth - ui->splitter_HorizontalPlot->handleWidth() << 0;
    versizes << ui->container_AbovePlot->height() << winheight - ui->splitter_VerticalPlot->handleWidth() << 0;
    ui->splitter_HorizontalPlot->setSizes(horsizes);
    ui->splitter_VerticalPlot->setSizes(versizes);
}

void BareMinimumPlotter::addVariableInput()
{
    m_VariableInputs.push_back(new VariableInput());
    VariableInput *new_variable = m_VariableInputs.back();
    //	add to gui
    new_variable->setNumber(m_variable_count++);
    ui->layout_Variable->addWidget(new_variable);
    //	connect slots to signals
    QObject::connect(new_variable, SIGNAL(axisModeChanged(int)), 	// axis mode synch
                     this, SLOT(checkAxisMode(int)));
    QObject::connect(new_variable, SIGNAL(killThis(int)),			// delete
                     this, SLOT(removeVariableInput(int)));
    QObject::connect(this, SIGNAL(variableSplitterMoved(QList<int>)), 		// splitter resize
                     new_variable, SLOT(splitterResize(QList<int>)));
    //	enable remove buttons
    if (m_VariableInputs.size() > 2){
        new_variable->setAxisMode(MODE_POINT);
        m_VariableInputs[0]->enableRemoveButton(true);
        m_VariableInputs[1]->enableRemoveButton(true);
    }
    //	set tab order
    determineTabOrder();
    //	resize according to splitter
    variableSplitterMoved(ui->splitter_Variable->sizes());
    flag_Saved = false;
}

void BareMinimumPlotter::addInequalityInput()
{
    m_InequalityInputs.push_back(new InequalityInput());
    InequalityInput *new_inequality = m_InequalityInputs.back();
    //	add to gui
    ui->layout_Inequality->addWidget(new_inequality);
    new_inequality->setNumber(ui->layout_Inequality->count()-1);
    //	connect slots to signals
    QObject::connect(new_inequality, SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(new_inequality, SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(new_inequality, SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    QObject::connect(this, SIGNAL(inequalitySplitterMoved(QList<int>)),
                        new_inequality, SLOT(splitterResize(QList<int>)));
    //	enable/disable combination menu
    determineButtonStates();
    //	set tab order
    determineTabOrder();
    //	resize according to splitter
    inequalitySplitterMoved(ui->splitter_Inequality->sizes());
    flag_Saved = false;
}

void BareMinimumPlotter::addInequalityLoader(string filename)
{
    m_InequalityLoaders.push_back(new InequalityLoader);
    InequalityLoader *new_inequality = m_InequalityLoaders.back();
    //	add to gui
    ui->layout_Inequality->addWidget(new_inequality);
    new_inequality->setNumber(ui->layout_Inequality->count()-1);
    //	connect slots to signals
    QObject::connect(new_inequality, SIGNAL(killThis(int)),
                        this, SLOT(removeInequalityInput(int)));
    QObject::connect(new_inequality, SIGNAL(moveUp(int)),
                        this, SLOT(moveInequalityInputUp(int)));
    QObject::connect(new_inequality, SIGNAL(moveDown(int)),
                        this, SLOT(moveInequalityInputDown(int)));
    QObject::connect(this, SIGNAL(inequalitySplitterMoved(QList<int>)),
                        new_inequality, SLOT(splitterResize(QList<int>)));
    // load data
    if (filename == ""){
        filename = QFileDialog::getOpenFileName(this, "Open plot", ".", "JSON (*.json)").toStdString();
    }
    new_inequality->loadCase(filename);
    // enable/disable combination menu
    determineButtonStates();
    //	set tab order
    determineTabOrder();
    //	resize according to splitter
    inequalitySplitterMoved(ui->splitter_Inequality->sizes());

    flag_Saved = false;
}

void BareMinimumPlotter::determineInequalityOrder()
{
    vector<InequalityInput*> tmpVecInput;
    vector<InequalityLoader*> tmpVecLoader;
    for (int i = 0; i < static_cast<int>(ui->layout_Inequality->count()); i++){
        QWidget *layout_item = ui->layout_Inequality->itemAt(i)->widget();
        if (layout_item->accessibleDescription() == "input"){
           tmpVecInput.push_back(qobject_cast<InequalityInput*>(layout_item));
           qobject_cast<InequalityInput*>(layout_item)->setNumber(i);
        }
        if (layout_item->accessibleDescription() == "loader"){
           tmpVecLoader.push_back(qobject_cast<InequalityLoader*>(layout_item));
           qobject_cast<InequalityLoader*>(layout_item)->setNumber(i);
        }
    }
    m_InequalityInputs = tmpVecInput;
    m_InequalityLoaders = tmpVecLoader;
}

void BareMinimumPlotter::determineButtonStates()
{
    int nSize = ui->layout_Inequality->count()-1;
    QWidget *front_item = ui->layout_Inequality->itemAt(0)->widget();
    //	TODO: make this more elegant
    // disable movement buttons if only 1 widget in layout
    if (ui->layout_Inequality->count() < 2){
        if(front_item->accessibleDescription() == "input")
            m_InequalityInputs.front()->enablePositionButtons(false);
        if(front_item->accessibleDescription() == "loader")
            m_InequalityLoaders.front()->enablePositionButtons(false);
    }
    // enable position buttons if more than 1 widget
    if (ui->layout_Inequality->count() > 1){
        if(front_item->accessibleDescription() == "input")
            m_InequalityInputs.front()->enablePositionButtons(true);
        if(front_item->accessibleDescription() == "loader")
            m_InequalityLoaders.front()->enablePositionButtons(true);
    }
    // disable the combinations of last item
    QWidget *back_item = ui->layout_Inequality->itemAt(nSize)->widget();
    if(back_item->accessibleDescription() == "input"){
        qobject_cast<InequalityInput*>(back_item)->enableCombinations(false);
        qobject_cast<InequalityInput*>(back_item)->resetCombinations();
    }
    if(back_item->accessibleDescription() == "loader"){
        qobject_cast<InequalityLoader*>(back_item)->enableCombinations(false);
        qobject_cast<InequalityLoader*>(back_item)->resetCombinations();
    }
    // enable combinations of second last item
    if (ui->layout_Inequality->count() > 1){
        QWidget *second_last_item = ui->layout_Inequality->itemAt(nSize)->widget();
        if(second_last_item->accessibleDescription() == "input"){
            qobject_cast<InequalityInput*>(second_last_item)->enableCombinations(true);
        }
        if(second_last_item->accessibleDescription() == "loader"){
            qobject_cast<InequalityLoader*>(second_last_item)->enableCombinations(true);
        }
    }
}

void BareMinimumPlotter::determineTabOrder()
{
    QWidget *widget;
    QWidget *current;
    QWidget *prev;
    prev = ui->tabWidget;
    for (int i = 0; i < ui->layout_Inequality->count(); i++){
        widget = ui->layout_Inequality->itemAt(i)->widget();
        current = getFocusInWidget(widget);
        QWidget::setTabOrder(prev, current);
        prev = getFocusOutWidget(widget);
    }
    for (int i = 0; i < ui->layout_Variable->count(); i++){
        widget = ui->layout_Variable->itemAt(i)->widget();
        current = getFocusInWidget(widget);
        QWidget::setTabOrder(prev, current);
        prev = getFocusOutWidget(widget);
    }
    QWidget::setTabOrder(prev, ui->toolButton_Plot);
    QWidget::setTabOrder(ui->toolButton_Plot, ui->toolButton_AddInequality);
    QWidget::setTabOrder(ui->toolButton_AddInequality, ui->toolButton_AddInequalityLoader);
    QWidget::setTabOrder(ui->toolButton_AddInequalityLoader, ui->toolButton_AddVariable);
    QWidget::setTabOrder(ui->toolButton_AddVariable, ui->pushButton_Cancel);
    QWidget::setTabOrder(ui->pushButton_Cancel, ui->lineEdit_SettingsTolerance);
}

QWidget* BareMinimumPlotter::getFocusInWidget(QWidget* widget)
{
    if (widget->accessibleDescription() == "input"){
        return qobject_cast<InequalityInput*>(widget)->getFocusInWidget();
    } else if (widget->accessibleDescription() == "loader"){
        return qobject_cast<InequalityLoader*>(widget)->getFocusInWidget();
    } else if (widget->accessibleDescription() == "variable-input"){
        return qobject_cast<VariableInput*>(widget)->getFocusInWidget();
    }
    return widget;
}

QWidget* BareMinimumPlotter::getFocusOutWidget(QWidget* widget)
{
    if (widget->accessibleDescription() == "input"){
        return qobject_cast<InequalityInput*>(widget)->getFocusOutWidget();
    } else if (widget->accessibleDescription() == "loader"){
        return qobject_cast<InequalityLoader*>(widget)->getFocusOutWidget();
    } else if (widget->accessibleDescription() == "variable-input"){
        return qobject_cast<VariableInput*>(widget)->getFocusOutWidget();
    }
    return widget;
}


//	Parsing and File IO
//	--------------------

void BareMinimumPlotter::save_JSON(QString filename)
{
    stringstream case_buffer, case_element_buffer, case_subelement_buffer;
    if (filename.isEmpty())
        return;

    //	- variables
    //		-- all plots in case have the same variable parameters
    for(unsigned int j = 0; j < m_VariableInputs.size(); j++){
        case_element_buffer << m_VariableInputs[j]->toJSON();
        if (j != m_VariableInputs.size() - 1)
            case_element_buffer << ",";
        case_element_buffer << "\n";
    }
    // encapsulate variables
    case_buffer << "\"variables\":[\n" << case_element_buffer.str() << "],\n";
    case_element_buffer.str("");

    // 	- plots
    for (int i = 0; i < ui->layout_Inequality->count(); i++){
        if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "input"){
            InequalityInput *current_inequality = qobject_cast<InequalityInput*>(ui->layout_Inequality->itemAt(i)->widget());
            // expression
            case_subelement_buffer << current_inequality->expressionToJSON();
            if (current_inequality->getCombination() == CombinationNone){ // save only combined results
                // start plot brace
                case_element_buffer << "\"plot\":{";
                // encapsulate expressions
                case_element_buffer << "\"expressions\":{\n" <<  case_subelement_buffer.str()<< "\n},\n";
                case_subelement_buffer.str("");
                // encapsulate plot data
                case_element_buffer << current_inequality->dataToJSON();
                // end plot brace
                case_element_buffer << "}";
                // comma after plot if not at end
                if (i != ui->layout_Inequality->count()-1){
                    case_element_buffer << ",\n";
                }
            } else {
                // new expression
                case_subelement_buffer << ",\n";
            }
        }
        if (ui->layout_Inequality->itemAt(i)->widget()->accessibleDescription() == "loader"){
            InequalityLoader *current_inequality = qobject_cast<InequalityLoader*>(ui->layout_Inequality->itemAt(i)->widget());
            // expression
            case_subelement_buffer << current_inequality->expressionToJSON();
            if (current_inequality->getCombination() == CombinationNone){ // save only combined results
                // start plot brace
                case_element_buffer << "\"plot\":{";
                // encapsulate expressions
                case_element_buffer << "\"expressions\":{\n" <<  case_subelement_buffer.str()<< "\n},\n";
                case_subelement_buffer.str("");
                // encapsulate plot data
                case_element_buffer << current_inequality->dataToJSON();
                // end plot brace
                case_element_buffer << "}";
                // comma after plot if not at end
                if (i != ui->layout_Inequality->count()-1){
                    case_element_buffer << ",\n";
                }
            } else {
                // new expression
                case_subelement_buffer << ",\n";
            }
        }

        if(i == static_cast<int>(ui->layout_Inequality->count())-1){ 	// not the end
            case_buffer << case_element_buffer.str() << "\n";
            case_element_buffer.str("");
        }
    }


    // encapsulate case, write to file
    ofstream outFile(filename.toStdString().c_str());
    if (outFile.is_open()){
        outFile << "{\"name\":\"" << m_Title.toStdString() << "\",\n" << case_buffer.str() << "}";
        outFile.close();
    }
}

void BareMinimumPlotter::open_variables(string json)
{
    string token;
    stringstream ss;
    unsigned int nVar = 0;
    ss << json;
    while ( getline (ss, token, '{')){
        if ( getline (ss, token, '}')){
            if ((nVar+1) > m_VariableInputs.size()){
                addVariableInput();
            }
            m_VariableInputs[nVar]->fromJSON(token);
        }
        nVar++;
    }
}


///	Public Slots
/// =============

void BareMinimumPlotter::checkAxisMode(int gui_number)
{
    int xcount = 0, ycount = 0, xpos, ypos;
    for (int i = 0; i < static_cast<int>(m_VariableInputs.size()); i++){ 	// find x-axis and y-axis labels
       if(m_VariableInputs[i]->getAxisMode() == MODE_X_AXIS) {
            xcount++;
            if (m_VariableInputs[i]->getNumber() != gui_number)
                xpos = i;
       }
       if(m_VariableInputs[i]->getAxisMode() == MODE_Y_AXIS) {
            ycount++;
            if (m_VariableInputs[i]->getNumber() != gui_number)
                ypos = i;
       }
    }

    if (xcount == 2 && ycount == 1){ m_VariableInputs[xpos]->setAxisMode(MODE_POINT); } 	// sort such that only one x and one y
    else if (xcount == 2 && ycount == 0) { m_VariableInputs[xpos]->setAxisMode(MODE_Y_AXIS); }
    else if (xcount == 0 && ycount == 2) { m_VariableInputs[ypos]->setAxisMode(MODE_X_AXIS); }
    else if (xcount == 1 && ycount == 2) { m_VariableInputs[ypos]->setAxisMode(MODE_POINT); }
    else if (xcount == 1 && ycount == 0) { m_VariableInputs[gui_number]->setAxisMode(MODE_Y_AXIS); }
    else if (xcount == 0 && ycount == 1) { m_VariableInputs[gui_number]->setAxisMode(MODE_X_AXIS); }
}

void BareMinimumPlotter::removeVariableInput(int gui_number)
{
    if (m_VariableInputs.size() < 3) // at least 2 variable inputs needed
        return;
    for (unsigned int i = 0; i < m_VariableInputs.size(); i++){
        if (m_VariableInputs[i]->getNumber() == gui_number){
            int axis_mode = m_VariableInputs[i]->getAxisMode();
            // delete
            ui->layout_Variable->removeWidget(m_VariableInputs[i]);
            delete m_VariableInputs[i];
            if (i < m_VariableInputs.size())
                m_VariableInputs.erase(m_VariableInputs.begin()+i);
            // ensure correct axis modes
            // [TODO]	make this more elegant
            if (axis_mode != MODE_POINT){
                if (i < m_VariableInputs.size()){
                    if(m_VariableInputs[i]->getAxisMode() != MODE_POINT){
                        if (i+1 < m_VariableInputs.size()){
                            m_VariableInputs[i+1]->setAxisMode(axis_mode);
                        } else if (i != 0){
                            m_VariableInputs[i-1]->setAxisMode(axis_mode);
                        }
                    } else {
                        m_VariableInputs[i]->setAxisMode(axis_mode);
                    }
                } else {
                    if(m_VariableInputs[i-1]->getAxisMode() != MODE_POINT){
                        if (i != 0){
                            m_VariableInputs[i-2]->setAxisMode(axis_mode);
                        }
                    } else {
                        m_VariableInputs[i-1]->setAxisMode(axis_mode);
                    }
                }
            }
            // enable removal buttons
            if (m_VariableInputs.size() < 3){
                m_VariableInputs.front()->enableRemoveButton(false);
                m_VariableInputs.back()->enableRemoveButton(false);
            }
            flag_Saved = false;
            return;
        }
    }
}

void BareMinimumPlotter::removeInequalityInput(int gui_number)
{
    if (ui->layout_Inequality->count() < 2) // at least 1 inequality input needed
        return;

    // 	Using for loop to find exact element within the vector to delete
    //	else, would have used the qobjectcast method

    for (int i = 0; i < static_cast<int>(m_InequalityInputs.size()); i++){
        if (m_InequalityInputs[i]->getNumber() == gui_number){
            ui->layout_Inequality->removeWidget(m_InequalityInputs[i]);
            delete m_InequalityInputs[i];
            if (i < static_cast<int>(m_InequalityInputs.size()))
                m_InequalityInputs.erase(m_InequalityInputs.begin()+i);
            // enable/disable position buttons
            determineInequalityOrder();
            determineButtonStates();
            flag_Saved = false;
            return;
        }
    }
    for (int i = 0; i < static_cast<int>(m_InequalityLoaders.size()); i++){
        if (m_InequalityLoaders[i]->getNumber() == gui_number){
            // if there was a problem parsing, show it
            if (m_InequalityLoaders[i]->getErrors() != "")
                printError();
            ui->layout_Inequality->removeWidget(m_InequalityLoaders[i]);
            delete m_InequalityLoaders[i];
            if (i < static_cast<int>(m_InequalityLoaders.size()))
                m_InequalityLoaders.erase(m_InequalityLoaders.begin()+i);
            // enable/disable position buttons
            determineInequalityOrder();
            determineButtonStates();
            flag_Saved = false;
            return;
        }
    }
}

void BareMinimumPlotter::moveInequalityInputUp (int gui_number)
{
    int pos = gui_number;
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < m_InequalityInputs.size(); i++){
            if (m_InequalityInputs[i]->getNumber() == gui_number){
                if (pos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(m_InequalityInputs[i]);
                ui->layout_Inequality->insertWidget(--pos, m_InequalityInputs[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_Saved = false;
                return;
            }
        }
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < m_InequalityLoaders.size(); i++){
            if (m_InequalityLoaders[i]->getNumber() == gui_number){
                if (pos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(m_InequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(--pos, m_InequalityLoaders[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_Saved = false;
                return;
            }
        }
}

void BareMinimumPlotter::moveInequalityInputDown (int gui_number)
{
    int pos = gui_number;
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < m_InequalityInputs.size(); i++){
            if (m_InequalityInputs[i]->getNumber() == gui_number){
                if (pos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(m_InequalityInputs[i]);
                ui->layout_Inequality->insertWidget(++pos, m_InequalityInputs[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_Saved = false;
                return;
            }
        }
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < m_InequalityLoaders.size(); i++){
            if (m_InequalityLoaders[i]->getNumber() == gui_number){
                if (pos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(m_InequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(++pos, m_InequalityLoaders[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_Saved = false;
                return;
            }
        }
}


///	Private Slots
/// ==============

void BareMinimumPlotter::menu_about()
{
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

// [BREAK] 14 July 2014 | finished primitive file open. Time to work on GUI.
void BareMinimumPlotter::menu_open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open plot", QString::fromStdString(m_default_directory), "JSON (*.json)");
    if (filename.isEmpty())
        return;

    clearGUI();

    BlueJSON parser;
    parser.readInFile(filename.toStdString());

    string token;
    //	case name
    parser.getNextKeyValue("name", token);
    parser.getStringToken(token);
    ui->lineEdit_PlotTitle->setText(QString::fromStdString(token));

    //	variables
    parser.getNextKeyValue("variables", token);
    open_variables(token);

    //	expressions
    int gui_number = 0;
    vector<string> keys;
    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;
    while (parser.getNextKeyValue(keys,token, closest_key)){
        if (closest_key == 1){	//	case found
            BlueJSON subparser = BlueJSON(token);
            string file;
            subparser.getNextKeyValue("file", file);
            subparser.getStringToken(file);
            addInequalityLoader(file);
            gui_number++;
        } else {				//	inequality found
            if ((gui_number + 1) > ui->layout_Inequality->count())
                addInequalityInput();
            gui_number++;
            m_InequalityInputs.back()->fromJSON(token);
        }
    }
    flag_Saved = false;
}

void BareMinimumPlotter::menu_saveAs()
{
    if (flag_Empty){
        m_error_message = "Error | Input | Plot not complete. Cannot save.";
        printError();
        return;
    }
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString filename = dialog.getSaveFileName(this, "Save configuration", QString::fromStdString(m_default_directory), "JSON (*.json)");
    save_JSON(filename);
    flag_Saved = true;
}

void BareMinimumPlotter::menu_new()
{
    if (flag_Saved){
        clearGUI();
        return;
    }
    QString warning = "<b>The current work has not been saved.</b><br>Are you sure you want to start a new case?";
    QMessageBox *warnbox;
    int response  = warnbox->warning(this, "New Case", warning, "&Cancel", "&New Case");
    if (response == 1){
        clearGUI();
    }
}

void BareMinimumPlotter::menu_quit()
{
    if (flag_Saved){
        this->close();
        return;
    }
    QString warning = "<b>The current work has not been saved.</b><br>Are you sure you want to exit?";
    QMessageBox *warnbox;
    int response  = warnbox->warning(this, "Exiting BareMinimumPlotter", warning, "&Cancel", "E&xit");
    if (response == 1){
        this->close();
    }
}

void BareMinimumPlotter::on_toolButton_Plot_clicked() { plot(); }

void BareMinimumPlotter::on_toolButton_AddVariable_clicked() { addVariableInput(); }

void BareMinimumPlotter::on_toolButton_AddInequality_clicked() { addInequalityInput(); }

void BareMinimumPlotter::on_toolButton_AddInequalityLoader_clicked() { addInequalityLoader(); }

void BareMinimumPlotter::on_splitter_Variable_splitterMoved(int /*pos*/, int /*index*/)
{
    elideLable(ui->label_VariablePlotMode, "Mode");
    elideLable(ui->label_VariablePointChooser, "Point Chooser");
    emit variableSplitterMoved(ui->splitter_Variable->sizes());
}

void BareMinimumPlotter::on_splitter_Inequality_splitterMoved(int /*pos*/, int /*index*/)
{
    emit inequalitySplitterMoved(ui->splitter_Inequality->sizes());
}

void BareMinimumPlotter::on_lineEdit_SettingsTolerance_editingFinished()
{
    QLineEdit *edit = ui->lineEdit_SettingsTolerance;
    stringstream buffer;
    buffer << edit->text().toStdString();
    if (!(buffer >> m_tolerance))
        m_tolerance = 0;
    edit->clearFocus();
}


void BareMinimumPlotter::on_lineEdit_PlotTitle_returnPressed() { ui->plotter->setFocus(); }

void BareMinimumPlotter::on_lineEdit_PlotTitle_textChanged(const QString&) { fitLineEditToContents(ui->lineEdit_PlotTitle); }

void BareMinimumPlotter::on_lineEdit_PlotTitle_editingFinished()
{
    if (ui->lineEdit_PlotTitle->text() == "Title") { m_Title = "untitled case"; }
    else { m_Title = ui->lineEdit_PlotTitle->text(); }
}
