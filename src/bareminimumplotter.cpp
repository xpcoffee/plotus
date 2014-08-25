/*!	Author(s):	Emerick Bosch
    Build:		0.3
    Date:		July 2014

    expression.cpp
    -------------
*/

/*	TASK LIST
    ---------
*/


///	Includes
///	========

#include "include/bareminimumplotter.h"
#include "ui_bareminimumplotter.h"

#ifndef DEBUG_POINT
#define DEBUG_POINT qDebug() << "Debug marker. | In file " << __FILE__ << " at line " << __LINE__;
#endif

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
    edit->setMinimumWidth(width + 10);
    edit->setMaximumWidth(width + 10);
}


///	Public Functions
/// =================

//	Constructor
//	-----------

BareMinimumPlotter::BareMinimumPlotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BareMinimumPlotter),
    m_title("untitled_case"),
    m_prevCombination (0),
    m_variableCount(0),
    m_inequalityCount(0),
    flag_saved(true),
    flag_empty (true)
{
    ui->setupUi(this);

    //	setup continued - things not defined by XML UI file
    setupUiCont();

    //	load settings
    loadSettings();

    // 	set up initial dynamic elements
    setupDynamicUi();

    flag_saved = true;
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
    variableSplitterMoved(ui->splitter_VariableHeader->sizes());
    inequalitySplitterMoved(ui->splitter_InequalityHeader->sizes());
}


//	Plotting and Evaluation
//	------------------------

void BareMinimumPlotter::plot()
{
    clearFormatting();
    configurePlot();

    if(!checkExpressions() || !checkVariables()){
        printError();
        setUIMode(Available);
        return;
    }

    thread = new QThread();
    worker = new PlotWorker();
    worker->moveToThread(thread);

    qRegisterMetaType<VarInputArray>("VarInputArray");
    qRegisterMetaType<IneqInputArray>("IneqInputArray");
    qRegisterMetaType<IneqLoaderArray>("IneqLoaderArray");
    qRegisterMetaType<PlottingVector>("PlottingVector");
    qRegisterMetaType<PlotStyle>("PlotStyle");
    qRegisterMetaType<Variable>("Variable");

    QWidget::connect(thread, SIGNAL	(started()), worker, SLOT	(createWorker()));

    QWidget::connect(worker, SIGNAL	(dataRequest()),
                     this, SLOT		(sendWorkerData()));
    QWidget::connect(this, SIGNAL	(feedPlotWorker(VarInputArray,IneqInputArray,IneqLoaderArray,Variable,Variable,double)),
                     worker, SLOT	(workerInit(VarInputArray,IneqInputArray,IneqLoaderArray,Variable,Variable,double)));
    QWidget::connect(worker, SIGNAL	(logMessage(QString)),
                     this, SLOT		(log(QString)));
    QWidget::connect(worker, SIGNAL	(progressUpdate(int, QString)),
                     this, SLOT		(setProgress(int, QString)));
    QWidget::connect(worker, SIGNAL	(newGraph(PlottingVector,PlotStyle,QColor, QString)),
                     this, SLOT		(addGraph(PlottingVector, PlotStyle, QColor, QString)));
    QWidget::connect(worker, SIGNAL	(newErrorGraph(PlottingVector)),
                     this, SLOT		(addErrorGraph(PlottingVector)));
    QWidget::connect(worker, SIGNAL(memberChanges(VarInputArray,IneqInputArray,IneqLoaderArray)),
                     this, SLOT(registerMemberChanges(VarInputArray,IneqInputArray,IneqLoaderArray)));
    QWidget::connect(worker, SIGNAL	(workFinished()),
                     this, SLOT		(plottingFinished()));

    QWidget::connect(worker, SIGNAL(workFinished()), thread, SLOT(quit()));
    QWidget::connect(worker, SIGNAL(workFinished()), worker, SLOT(deleteLater()));
    QWidget::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();

}

void BareMinimumPlotter::configurePlot()
{
    // reload loaders data
    // (in case previous plotting changed its data through combinations)
    for (unsigned int i = 0; i < m_inequalityLoaders.size(); i++){
//        m_inequalityLoaders[i]->loadCase(m_inequalityLoaders[i]->getFile());
        m_inequalityLoaders[i]->clearCombinationResults();
    }

    // axes, legend
    configureAxes();
    plotter->insertLegend(new QwtLegend);

    //	evaluate and plot each inequality
    m_graphCount = 0;
    m_prevCombination = 0;

    //	clear previous plots
    plotter->detachItems();
    plotter->replot();

    flag_empty = true;
}

void BareMinimumPlotter::configureAxes()
{
    QString x_units, y_units;

    //	get variables and units
    for (int i = 0; i < static_cast<int>(m_variableInputs.size()); i++){
        VariableInput *varinput = m_variableInputs[i];
        if (varinput->getAxisMode() == PlotHorizontal){
            m_xVariable = varinput->getVariable();
            x_units = varinput-> getUnits();
        } else if (varinput->getAxisMode() == PlotVertical){
            m_yVariable = varinput->getVariable();
            y_units = varinput-> getUnits();
        }
    }

    //	format axes titles
    if (!x_units.isEmpty())
        x_units = " [" + x_units + "]";
    if (!y_units.isEmpty())
        y_units = " [" + y_units + "]";

    //	apply to graph
    plotter->setAxisTitle(QwtPlot::yLeft, QString::fromStdString( m_yVariable.name() ) + y_units);
    plotter->setAxisScale(QwtPlot::yLeft, m_yVariable.min(), m_yVariable.max());
    plotter->setAxisTitle(QwtPlot::xBottom, QString::fromStdString( m_xVariable.name() ) + x_units);
    plotter->setAxisScale(QwtPlot::xBottom, m_xVariable.min(), m_xVariable.max());
}

bool BareMinimumPlotter::addVariables(InequalityInput* input)
{
    bool flag_ok = true;

    for (int j = 0; j < static_cast<int>(m_variableInputs.size()); j++){
        VariableInput *variable = m_variableInputs[j];

        if (!variable->checkInput()) {	// check legal
            flag_ok = false;
            continue; // next check not needed
        }

        if(!input->addVariable(variable->getVariable())){
            variable-> highlightName();
            flag_ok = false;
        }
    }
    return flag_ok;
}


//	Validation
//	----------

void BareMinimumPlotter::printclr()
{
    m_errorMessage = "";
    ui->textEdit_Error->setText(m_errorMessage);
}

void BareMinimumPlotter::printError()
{
    // get all error messages
    // display messages
    for (int i = 0; i < static_cast<int>(m_inequalityInputs.size()); i++){
        m_errorMessage += m_inequalityInputs[i]->getErrors();
    }
    for (int i = 0; i < static_cast<int>(m_inequalityLoaders.size()); i++){
        m_errorMessage += QString::fromStdString(m_inequalityLoaders[i]->getErrors());
    }

    log(m_errorMessage);

    // progress bar
    setProgress(100, "Error.");
    flag_empty = true;
}


bool BareMinimumPlotter::checkVariables()
{
    bool flag_ok = true;

    for (unsigned int i = 0; i < m_inequalityInputs.size(); i++){
        //	check initialized
        if (!addVariables(m_inequalityInputs[i])){
            flag_ok = false;
        }

        //	check uninitialized
        flag_ok = flag_ok && m_inequalityInputs[i]->checkVariablesInit();
        m_inequalityInputs[i]->highlightInvalidExpressionTerms();
    }
    return flag_ok;
}

bool BareMinimumPlotter::checkExpressions()
{
    bool flag_ok = true;

    for (unsigned int i = 0; i < m_inequalityInputs.size(); i++){
        if(!m_inequalityInputs[i]->createInequality()){
            flag_ok = false;
        }
    }

    return flag_ok;
}

//	GUI
//	---

void BareMinimumPlotter::setupUiCont()
{
    setupInputValidation();

    setupQwtPlot();

    setupScrollAreas();

    setupButtons();

    loadCSS();
}

void BareMinimumPlotter::setupInputValidation()
{
    //	settings fields
    QDoubleValidator *dValidator = new QDoubleValidator;
    dValidator->setLocale(QLocale(QStringLiteral("de")));
    ui->lineEdit_SettingsTolerance->setValidator(dValidator);
}

void BareMinimumPlotter::setupQwtPlot()
{
    //	add Qwt plot
    QVBoxLayout *layout_Plot = new QVBoxLayout();
    ui->container_Graph->setLayout(layout_Plot);
    plotter = new QwtPlot(ui->centralWidget);
    plotter->canvas()->setStyleSheet("QwtPlotCanvas{"
                                     "border: 0px;"
                                     "background: white;"
                                     "}");
    layout_Plot->addWidget(plotter);
    plotter->setContextMenuPolicy(Qt::CustomContextMenu);
    QWidget::connect (plotter, SIGNAL(customContextMenuRequested(const QPoint &)),
                      this, SLOT(menu_qwt_context(const QPoint &)));

    //	make plot window fill the tab on startup
    resetQwtPlotWindow();
}

void BareMinimumPlotter::setupScrollAreas()
{
    //	account for scrollbar sizes in ui
    int new_size = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QSpacerItem *scroll_bar_spacer = ui->spacer_VariableHeaderScroll;
    QSize old_size = scroll_bar_spacer->sizeHint();
    scroll_bar_spacer->changeSize(new_size, old_size.height());
    ui->spacer_InequalityHeaderScroll->changeSize(new_size, old_size.height());

    QWidget::connect(ui->scrollArea_InequalityInputs->horizontalScrollBar(), SIGNAL(sliderMoved(int)),
                     this, SLOT(scrollInequalityHeader(int)));
    QWidget::connect(ui->scrollArea_VariableInputs->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                     this, SLOT(scrollVariableHeader(int)));

    //	header scroll areas
//    QWidget *header = ui->layout_InequalityHeaders->takeAt(1)->widget();
//    QWidget *header_widget = ui->scrollArea_InequalityInputs->getHeaderWidget();
//    QVBoxLayout *header_layout = new QVBoxLayout();
//    header_layout->addWidget(header);
//    header_widget->setLayout(header_layout);
}

void BareMinimumPlotter::setupButtons()
{
    ui->layout_VariableButtons->setAlignment(Qt::AlignVCenter);
    ui->layout_InequalityButtons->setAlignment(Qt::AlignVCenter);

    //	tool buttons
    ui->toolButton_AddInequality->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddInequality->setIcon(QPixmap("../bare_minimum_plotter/rsc/add-cross-white.png"));
    ui->toolButton_AddInequality->setIconSize(QSize(22,22));

    ui->toolButton_AddInequalityLoader->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddInequalityLoader->setIcon(QPixmap("../bare_minimum_plotter/rsc/load-white.png"));
    ui->toolButton_AddInequalityLoader->setIconSize(QSize(30,22));

    ui->toolButton_AddVariable->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_AddVariable->setIcon(QPixmap("../bare_minimum_plotter/rsc/add-cross-white.png"));
    ui->toolButton_AddVariable->setIconSize(QSize(22,22));

    ui->layout_VariableButtons->setAlignment(Qt::AlignTop);
    ui->layout_InequalityButtons->setAlignment(Qt::AlignTop);
}

void BareMinimumPlotter::setupDynamicUi()
{
    // 	add original x & y variable inputs
    ui->layout_Variable->setAlignment(Qt::AlignTop);
    addVariableInput();
    addVariableInput();
    m_variableInputs[0]->setAxisMode(PlotHorizontal);
    m_variableInputs[1]->setAxisMode(PlotVertical);
    m_variableInputs[0]->enableRemoveButton(false);
    m_variableInputs[1]->enableRemoveButton(false);
    m_variableInputs[0]->setSplitterSizes(ui->splitter_VariableHeader->sizes());
    m_variableInputs[1]->setSplitterSizes(ui->splitter_VariableHeader->sizes());

    // 	add original inequality input
    ui->layout_Inequality->setAlignment(Qt::AlignTop);
    addInequalityInput();
    m_inequalityInputs.front()->enablePositionButtons(false);
}

void BareMinimumPlotter::loadCSS()
{
    ifstream infile("../bare_minimum_plotter/rsc/centralwidget.css");
    string token, stylesheet;
    if (!infile.is_open())
        return;
    while (getline(infile, token)){
        stylesheet += token + "\n";
    }
    infile.close();
    ui->centralWidget->setStyleSheet(QString::fromStdString(stylesheet));
}

void BareMinimumPlotter::loadSettings()
{
    //	precision of inequality comparison
    stringstream buffer;
    buffer << ui->lineEdit_SettingsTolerance->text().toStdString();
    if (!(buffer >> m_compPrec))
        m_compPrec = 0;

    //	directory
    m_defaultDir = QDir::currentPath();
}

void BareMinimumPlotter::clearGUI()
{
    addInequalityInput();
    while (ui->layout_Inequality->count() > 1){
        removeInequalityInput(0);
    }
    while (m_variableInputs.size() > 2){
        removeVariableInput(0);
    }
    clearFormatting();
    m_variableInputs.back()->clearFields();
    m_variableInputs.front()->clearFields();
    resetQwtPlotWindow();
}

void BareMinimumPlotter::clearFormatting()
{
    m_errorMessage = "";
    for (int i = 0; i < static_cast<int>(m_inequalityInputs.size()); i++){
       m_inequalityInputs[i]->clearFormatting();
    }
    for (vector<VariableInput*>::iterator it = m_variableInputs.begin(); it != m_variableInputs.end(); it++){
        (*it)->clearFormatting();
    }
    setProgress(0, "");
}

void BareMinimumPlotter::resetQwtPlotWindow()
{
    //	clear graphs
    plotter->detachItems();
    plotter->setAxisTitle(QwtPlot::xBottom, "");
    plotter->setAxisTitle(QwtPlot::yLeft, "");
    plotter->replot();

    // reset size to fill window
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
    m_variableInputs.push_back(new VariableInput());
    VariableInput *new_variable = m_variableInputs.back();
    //	add to gui
    new_variable->setNumber(m_variableCount++);
    ui->layout_Variable->addWidget(new_variable);
    //	connect slots to signals
    QObject::connect(new_variable, SIGNAL(axisModeChanged(int)), 	// axis mode synch
                     this, SLOT(checkAxisMode(int)));
    QObject::connect(new_variable, SIGNAL(killThis(int)),			// delete
                     this, SLOT(removeVariableInput(int)));
    QObject::connect(this, SIGNAL(variableSplitterMoved(QList<int>)), 		// splitter resize
                     new_variable, SLOT(splitterResize(QList<int>)));
    //	enable remove buttons
    if (m_variableInputs.size() > 2){
        new_variable->setAxisMode(PlotConstant);
        m_variableInputs[0]->enableRemoveButton(true);
        m_variableInputs[1]->enableRemoveButton(true);
    }
    //	set tab order
    determineTabOrder();
    //	resize according to splitter
    variableSplitterMoved(ui->splitter_VariableHeader->sizes());
    flag_saved = false;
}

void BareMinimumPlotter::addInequalityInput()
{
    m_inequalityInputs.push_back(new InequalityInput());
    InequalityInput *new_inequality = m_inequalityInputs.back();
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
    inequalitySplitterMoved(ui->splitter_InequalityHeader->sizes());
    flag_saved = false;
}

void BareMinimumPlotter::addInequalityLoader(QString filename)
{
    m_inequalityLoaders.push_back(new InequalityLoader);
    InequalityLoader *new_inequality = m_inequalityLoaders.back();
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
        filename = QFileDialog::getOpenFileName(this, "Open plot", ".", "JSON (*.json)");
    }
    new_inequality->loadCase(filename.toStdString());
    // enable/disable combination menu
    determineButtonStates();
    //	set tab order
    determineTabOrder();
    //	resize according to splitter
    inequalitySplitterMoved(ui->splitter_InequalityHeader->sizes());

    flag_saved = false;
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
    m_inequalityInputs = tmpVecInput;
    m_inequalityLoaders = tmpVecLoader;
}

void BareMinimumPlotter::determineButtonStates()
{
    int nSize = ui->layout_Inequality->count()-1;
    QWidget *front_item = ui->layout_Inequality->itemAt(0)->widget();
    // disable movement buttons if only 1 widget in layout
    if (ui->layout_Inequality->count() < 2){
        if(front_item->accessibleDescription() == "input")
            m_inequalityInputs.front()->enablePositionButtons(false);
        if(front_item->accessibleDescription() == "loader")
            m_inequalityLoaders.front()->enablePositionButtons(false);
    }
    // enable position buttons if more than 1 widget
    if (ui->layout_Inequality->count() > 1){
        if(front_item->accessibleDescription() == "input")
            m_inequalityInputs.front()->enablePositionButtons(true);
        if(front_item->accessibleDescription() == "loader")
            m_inequalityLoaders.front()->enablePositionButtons(true);
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

void BareMinimumPlotter::setUIMode(UIMode mode)
{
    if (mode == Busy){
        ui->pushButton_Cancel->setEnabled(true);
        ui->container_InequalityMain->setEnabled(false);
        ui->container_VariableMain->setEnabled(false);
        ui->toolButton_Plot->setEnabled(false);
        ui->toolButton_AddInequality->setEnabled(false);
        ui->toolButton_AddInequalityLoader->setEnabled(false);
        ui->toolButton_AddVariable->setEnabled(false);
    } else if (mode == Available){
        ui->pushButton_Cancel->setEnabled(false);
        ui->container_InequalityMain->setEnabled(true);
        ui->container_VariableMain->setEnabled(true);
        ui->toolButton_Plot->setEnabled(true);
        ui->toolButton_AddInequality->setEnabled(true);
        ui->toolButton_AddInequalityLoader->setEnabled(true);
        ui->toolButton_AddVariable->setEnabled(true);
    }
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

void BareMinimumPlotter::saveCase_JSON(QString filename)
{
    stringstream case_buffer, case_element_buffer, case_subelement_buffer;
    if (filename.isEmpty())
        return;

    //	- variables
    //		-- all plots in case have the same variable parameters
    for(unsigned int j = 0; j < m_variableInputs.size(); j++){
        case_element_buffer << m_variableInputs[j]->toJSON().toStdString();
        if (j != m_variableInputs.size() - 1)
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
        outFile << "{\"name\":\"" << m_title.toStdString() << "\",\n" << case_buffer.str() << "}";
        outFile.close();
    }
}


void BareMinimumPlotter::openCase(QString filename)
{
    BlueJSON parser;
    parser.readInFile(filename.toStdString());
    string token;

    //	case name
    parser.getNextKeyValue("name", token);
    parser.getStringToken(token);
    ui->lineEdit_PlotTitle->setText(QString::fromStdString(token));

    //	variables
    parser.getNextKeyValue("variables", token);
    openVariables(token);

    //	expressions
    int gui_number = 0;

    vector<string> keys;
    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;

    while (parser.getNextKeyValue(keys,token, closest_key)){
        if (closest_key == 1){
            BlueJSON subparser = BlueJSON(token);
            string file;

            subparser.getNextKeyValue("file", file);
            subparser.getStringToken(file);
            addInequalityLoader(QString::fromStdString(file));

            gui_number++;
        } else {				//	inequality found
            if ((gui_number + 1) > ui->layout_Inequality->count())
                addInequalityInput();

            gui_number++;

            m_inequalityInputs.back()->fromJSON(token);
        }
    }

    flag_saved = false;

}

void BareMinimumPlotter::openVariables(string json)
{
    string token;
    stringstream ss;
    unsigned int nVar = 0;
    ss << json;
    while ( getline (ss, token, '{')){
        if ( getline (ss, token, '}')){
            if ((nVar+1) > m_variableInputs.size()){
                addVariableInput();
            }
            m_variableInputs[nVar]->fromJSON(token);
        }
        nVar++;
    }
}

///	Public Slots
/// =============

void BareMinimumPlotter::checkAxisMode(int gui_number)
{
    int xcount = 0, ycount = 0, xpos, ypos;
    for (int i = 0; i < static_cast<int>(m_variableInputs.size()); i++){ 	// find x-axis and y-axis labels
       if(m_variableInputs[i]->getAxisMode() == PlotHorizontal) {
            xcount++;
            if (m_variableInputs[i]->getNumber() != gui_number)
                xpos = i;
       }
       if(m_variableInputs[i]->getAxisMode() == PlotVertical) {
            ycount++;
            if (m_variableInputs[i]->getNumber() != gui_number)
                ypos = i;
       }
    }

    if (xcount == 2 && ycount == 1){ m_variableInputs[xpos]->setAxisMode(PlotConstant); } 	// sort such that only one x and one y
    else if (xcount == 2 && ycount == 0) { m_variableInputs[xpos]->setAxisMode(PlotVertical); }
    else if (xcount == 0 && ycount == 2) { m_variableInputs[ypos]->setAxisMode(PlotHorizontal); }
    else if (xcount == 1 && ycount == 2) { m_variableInputs[ypos]->setAxisMode(PlotConstant); }
    else if (xcount == 1 && ycount == 0) { m_variableInputs[gui_number]->setAxisMode(PlotVertical); }
    else if (xcount == 0 && ycount == 1) { m_variableInputs[gui_number]->setAxisMode(PlotHorizontal); }
}

void BareMinimumPlotter::removeVariableInput(int gui_number)
{
    if (m_variableInputs.size() < 3) // at least 2 variable inputs needed
        return;
    for (unsigned int i = 0; i < m_variableInputs.size(); i++){
        if (m_variableInputs[i]->getNumber() == gui_number){
            int axis_mode = m_variableInputs[i]->getAxisMode();
            // delete
            ui->layout_Variable->removeWidget(m_variableInputs[i]);
            delete m_variableInputs[i];
            if (i < m_variableInputs.size())
                m_variableInputs.erase(m_variableInputs.begin()+i);
            // ensure correct axis modes
            if (axis_mode != PlotConstant){
                if (i < m_variableInputs.size()){
                    if(m_variableInputs[i]->getAxisMode() != PlotConstant){
                        if (i+1 < m_variableInputs.size()){
                            m_variableInputs[i+1]->setAxisMode(axis_mode);
                        } else if (i != 0){
                            m_variableInputs[i-1]->setAxisMode(axis_mode);
                        }
                    } else {
                        m_variableInputs[i]->setAxisMode(axis_mode);
                    }
                } else {
                    if(m_variableInputs[i-1]->getAxisMode() != PlotConstant){
                        if (i != 0){
                            m_variableInputs[i-2]->setAxisMode(axis_mode);
                        }
                    } else {
                        m_variableInputs[i-1]->setAxisMode(axis_mode);
                    }
                }
            }
            // enable removal buttons
            if (m_variableInputs.size() < 3){
                m_variableInputs.front()->enableRemoveButton(false);
                m_variableInputs.back()->enableRemoveButton(false);
            }
            flag_saved = false;
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

    for (int i = 0; i < static_cast<int>(m_inequalityInputs.size()); i++){
        if (m_inequalityInputs[i]->getNumber() == gui_number){
            ui->layout_Inequality->removeWidget(m_inequalityInputs[i]);
            delete m_inequalityInputs[i];
            if (i < static_cast<int>(m_inequalityInputs.size()))
                m_inequalityInputs.erase(m_inequalityInputs.begin()+i);
            // enable/disable position buttons
            determineInequalityOrder();
            determineButtonStates();
            flag_saved = false;
            return;
        }
    }
    for (int i = 0; i < static_cast<int>(m_inequalityLoaders.size()); i++){
        if (m_inequalityLoaders[i]->getNumber() == gui_number){
            // if there was a problem parsing, show it
            if (m_inequalityLoaders[i]->getErrors() != "")
                printError();
            ui->layout_Inequality->removeWidget(m_inequalityLoaders[i]);
            delete m_inequalityLoaders[i];
            if (i < static_cast<int>(m_inequalityLoaders.size()))
                m_inequalityLoaders.erase(m_inequalityLoaders.begin()+i);
            // enable/disable position buttons
            determineInequalityOrder();
            determineButtonStates();
            flag_saved = false;
            return;
        }
    }
}

void BareMinimumPlotter::moveInequalityInputUp (int gui_number)
{
    int pos = gui_number;
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < m_inequalityInputs.size(); i++){
            if (m_inequalityInputs[i]->getNumber() == gui_number){
                if (pos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(m_inequalityInputs[i]);
                ui->layout_Inequality->insertWidget(--pos, m_inequalityInputs[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_saved = false;
                return;
            }
        }
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < m_inequalityLoaders.size(); i++){
            if (m_inequalityLoaders[i]->getNumber() == gui_number){
                if (pos == 0) // if at top
                    return;
                ui->layout_Inequality->removeWidget(m_inequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(--pos, m_inequalityLoaders[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_saved = false;
                return;
            }
        }
}

void BareMinimumPlotter::moveInequalityInputDown (int gui_number)
{
    int pos = gui_number;
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "input")
        for (unsigned int i = 0; i < m_inequalityInputs.size(); i++){
            if (m_inequalityInputs[i]->getNumber() == gui_number){
                if (pos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(m_inequalityInputs[i]);
                ui->layout_Inequality->insertWidget(++pos, m_inequalityInputs[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_saved = false;
                return;
            }
        }
    if (ui->layout_Inequality->itemAt(pos)->widget()->accessibleDescription() == "loader")
        for (unsigned int i = 0; i < m_inequalityLoaders.size(); i++){
            if (m_inequalityLoaders[i]->getNumber() == gui_number){
                if (pos == ui->layout_Inequality->count()) 		// if at bottom
                    return;
                ui->layout_Inequality->removeWidget(m_inequalityLoaders[i]);
                ui->layout_Inequality->insertWidget(++pos, m_inequalityLoaders[i]);
                determineInequalityOrder();
                determineButtonStates();
                determineTabOrder();
                flag_saved = false;
                return;
            }
        }
}

void BareMinimumPlotter::scrollInequalityHeader(int value)
{
    int scroll_amt = ui->scrollAreaWidget_InequalityInputs->geometry().x() + value/2;
    ui->splitter_InequalityHeader->scroll(scroll_amt, 0);
    ui->splitter_InequalityHeader->scroll(0, 0);
}

void BareMinimumPlotter::scrollVariableHeader(int value) { ui->splitter_InequalityHeader->scroll(-value/2, 0); }

void BareMinimumPlotter::sendWorkerData()
{
    emit feedPlotWorker(m_variableInputs,
                        m_inequalityInputs,
                        m_inequalityLoaders,
                        m_xVariable,
                        m_yVariable,
                        m_compPrec);
}

void BareMinimumPlotter::setProgress(int value, QString message)
{
    int progress = floor( ( value/100.0 + m_graphCount )/ui->layout_Inequality->count() * 100);
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat(message);

    if(message == "Error."){
        ui->progressBar->setStyleSheet("QProgressBar::chunk {background: red;}");
        ui->progressBar->setValue(100);
    } else if(message == "Cancelling..." ){
        ui->progressBar->setStyleSheet("QProgressBar::chunk {background: rgb(255, 170, 0);}");
        ui->progressBar->setValue(100);
    } else if(message == "Cancelled."){
        ui->progressBar->setStyleSheet("QProgressBar::chunk {background: rgb(208, 139, 0);}");
        ui->progressBar->setValue(100);
    } else {
        ui->progressBar->setStyleSheet("QProgressBar::chunk {background: rgb(37, 158, 0);}");
    }
}

void BareMinimumPlotter::addGraph(QVector<QPointF> qwt_samples, PlotStyle shape, QColor marker_color, QString tag)
{
        QwtPlotCurve *plot = new QwtPlotCurve(tag);

        if (shape == QwtSymbol::UserStyle){
            //	Dots style
            plot->setStyle(QwtPlotCurve::Dots);
            plot->setPen(marker_color);
            plot->setRenderThreadCount(0); // ideal thread count
        }
        else {
            int marker_line_width = 1;
            QSize marker_size = QSize(5,5);
            QwtSymbol *marker = new QwtSymbol(shape, QBrush(),
                                              QPen(marker_color, marker_line_width),
                                              marker_size);
            plot->setSymbol(marker);
            plot->setStyle(QwtPlotCurve::NoCurve);
        }

        //	set data
        plot->setSamples(qwt_samples);
        //	add graph
        plot->attach(plotter);
        plotter->replot();

        m_graphCount++;
        flag_empty = false;
}

void BareMinimumPlotter::addErrorGraph(QVector<QPointF> qwt_problem_samples)
{
        QwtPlotCurve *plot = new QwtPlotCurve();
        QwtSymbol *marker = new QwtSymbol(QwtSymbol::Star1,
                   QBrush(), QPen(Qt::red, 2),
                   QSize(4,4));
        //	add style
        plot->setSymbol(marker);
        plot->setStyle(QwtPlotCurve::NoCurve);
        plot->setTitle("Problem Points");
        //	add data
        plot->setSamples(qwt_problem_samples);
        //	add graph
        plot->attach(plotter);
        plotter->replot();
}

void BareMinimumPlotter::registerMemberChanges(VarInputArray var_inputs,
                                       IneqInputArray ineq_inputs,
                                       IneqLoaderArray ineq_loaders)
{
    m_variableInputs = var_inputs;
    m_inequalityInputs = ineq_inputs;
    m_inequalityLoaders = ineq_loaders;
}

void BareMinimumPlotter::log(QString message) { ui->textEdit_Error->setText(message); }

void BareMinimumPlotter::plottingFinished()
{
    setUIMode(Available);
    if (ui->progressBar->text() != "Cancelled.")
        ui->tabWidget->setCurrentIndex(1); // switch to plot pane
}

///	Private Slots
/// ==============

//	TODO: add github link instead of email. new dialog subclass?
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

    QMessageBox::about(0, title, info);
}

void BareMinimumPlotter::menu_open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open plot", m_defaultDir, "JSON (*.json)");
    if (filename.isEmpty())
        return;

    clearGUI();

    openCase(filename);
}

void BareMinimumPlotter::menu_saveAs()
{
    if (flag_empty){
        m_errorMessage = "Empty/incomplete graph. Nothing to save.";
        printError();
        return;
    }
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString filename = dialog.getSaveFileName(this, "Save configuration", m_defaultDir, "JSON (*.json)");
    if (filename.indexOf(".json") > 0)
        saveCase_JSON(filename);
    flag_saved = true;
}

void BareMinimumPlotter::menu_export()
{
    if (flag_empty){
        m_errorMessage = "Error | Input | Plot not complete. Cannot save.";
        printError();
        return;
    }
    ExportDialog *dialog = new ExportDialog(this);
    dialog->show();
    QWidget::connect(dialog, SIGNAL(exportOptions(int,int,int)), this, SLOT(exportQwt(int,int,int)));

}

void BareMinimumPlotter::menu_new()
{
    if (flag_saved){
        clearGUI();
        return;
    }
    QString warning = "<b>The current work has not been saved.</b><br>Are you sure you want to start a new case?";
    QMessageBox *warnbox;
    int response  = warnbox->warning(0, "New Case", warning, "&Cancel", "&New Case");
    if (response == 1){
        clearGUI();
    }
}

void BareMinimumPlotter::menu_quit()
{
    if (flag_saved){
        this->close();
        return;
    }

    QString warning = "<b>The current work has not been saved.</b><br>Are you sure you want to exit?";
    QMessageBox *warnbox;

    int response  = warnbox->warning(0, "Exiting BareMinimumPlotter", warning, "&Cancel", "E&xit");
    if (response == 1){
        this->close();
    }
}

void BareMinimumPlotter::menu_qwt_context(const QPoint &)
{
    QAction *copy_qwt = new QAction(this);
    copy_qwt->setText("Copy");

    QMenu *context_menu = new QMenu();
    context_menu->addAction(copy_qwt);
    connect(copy_qwt, SIGNAL(triggered()), this, SLOT(copyQwtToClipboard()));
    context_menu->popup(QCursor::pos());

}

void BareMinimumPlotter::on_toolButton_Plot_clicked()
{
    setUIMode(Busy);

    plot();

    flag_saved = false;
}

void BareMinimumPlotter::exportQwt(int width, int height, int dpi)
{
    QwtPlotRenderer *renderer = new QwtPlotRenderer();
    renderer->exportTo(plotter, m_title, QSizeF(width, height), dpi);
}

void BareMinimumPlotter::copyQwtToClipboard()
{
    //	BUG: getting "QImage::pixel: coordinate (...,...) out of range" when application quits
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(plotter->grab(plotter->geometry()));
}

void BareMinimumPlotter::on_toolButton_AddVariable_clicked() { addVariableInput(); }

void BareMinimumPlotter::on_toolButton_AddInequality_clicked() { addInequalityInput(); }

void BareMinimumPlotter::on_toolButton_AddInequalityLoader_clicked() { addInequalityLoader(); }

void BareMinimumPlotter::on_splitter_VariableHeader_splitterMoved(int /*pos*/, int /*index*/)
{
    elideLable(ui->label_VariableMode, "Mode");
    elideLable(ui->label_VariableConst, "Const. Chooser");
    emit variableSplitterMoved(ui->splitter_VariableHeader->sizes());
}

void BareMinimumPlotter::on_splitter_InequalityHeader_splitterMoved(int /*pos*/, int /*index*/)
{
    emit inequalitySplitterMoved(ui->splitter_InequalityHeader->sizes());
}

void BareMinimumPlotter::on_lineEdit_SettingsTolerance_editingFinished()
{
    QLineEdit *edit = ui->lineEdit_SettingsTolerance;
    stringstream buffer;
    buffer << edit->text().toStdString();
    if (!(buffer >> m_compPrec))
        m_compPrec = 0;
    edit->clearFocus();
}

void BareMinimumPlotter::on_lineEdit_PlotTitle_returnPressed() { ui->container_Graph->setFocus(); }

void BareMinimumPlotter::on_lineEdit_PlotTitle_textChanged(const QString&) { fitLineEditToContents(ui->lineEdit_PlotTitle); }

void BareMinimumPlotter::on_lineEdit_PlotTitle_editingFinished()
{
    if (ui->lineEdit_PlotTitle->text() == "New Plot") { m_title = "untitled case"; }
    else {
        m_title = ui->lineEdit_PlotTitle->text();
        flag_saved = false;
    }
}

void BareMinimumPlotter::on_pushButton_Cancel_clicked()
{
    worker->flag_cancel = true;
    setProgress(100, "Cancelling...");
}
