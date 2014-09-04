#include "include/inequalityloader.h"
#include "ui_inequalityloader.h"

///	Static Functions
///	=================

static QString cropFileName(QString filename)
{
        int pos0 = 0, pos1 = 0;

        for (int i = 0; i < filename.size(); i++)
            if (filename[i] == '/')
                pos0 = i;

        for (int i = 0; i < filename.size(); i++)
            if (filename[i] == '.')
                pos1= i;

        return filename.mid(pos0 + 1, pos1 - pos0);
}

///	Public Functions
///	=================

//	Constructors
//	-------------

InequalityLoader::InequalityLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityLoader),
    m_guiNumber(-1),
    flag_skip(false),
    flag_ok(true),
    m_errorMessage(""),
    m_currentPlot(0)
{
    ui->setupUi(this);
    setAccessibleDescription("loader");
    //	splitters
    for(int i = 0; i < ui->splitter_InequalityInput->count(); i++){
        ui->splitter_InequalityInput->handle(i)->setEnabled(false);
    }
    m_detailLevel = 0;
}


//	Destructor
//	-----------

InequalityLoader::~InequalityLoader()
{
    delete ui;
}


// 	Setters
//	-------

void InequalityLoader::setNumber(int number = -1)
{
    if (number > -1)
        m_guiNumber = number;
    ui->label_Number->setNum(number + 1);
}

void InequalityLoader::setCaseName(QString value)
{
    if (!value.isEmpty())
        ui->label_CaseOut->setText(value);
}


//	TODO: insert error checking (flag_problem)

void InequalityLoader::loadCase(QString filename)
{
    if (filename.isEmpty()){
        emit killThis(m_guiNumber);
        return;
    }

    //	read in file
    m_file.setFileName(filename);

    flag_ok = m_file.open(QIODevice::ReadOnly | QIODevice::Text);
    checkOK("Could not open file: " + filename + ".");
    if(!flag_ok){
        emit killThis(m_guiNumber);
        return;
    }

    QString contents =  QString::fromUtf8(m_file.readAll());
    m_file.close();

    parseFile(contents);

    return;
}


void InequalityLoader::setX(QVector<double> vector) { m_xResults_combination = vector; }

void InequalityLoader::setY(QVector<double> vector) { m_yResults_combination = vector; }

void InequalityLoader::clearCombinationResults()
{
    m_xResults_combination.clear();
    m_yResults_combination.clear();
}

void InequalityLoader::createDetailItem(int level, QString type, QString desc, QString data1, QString data2)
{
    DetailItem newItem;
    newItem.level = level;
    newItem.description = desc;
    newItem.type = type;
    newItem.data1 = data1;
    newItem.data2 = data2;
    m_details << newItem;
}


//	Getters: UI
//	------------

QString InequalityLoader::getName() { return ui->comboBox_Plot->currentText(); }

int	InequalityLoader::getNumber() { return m_guiNumber; }

int InequalityLoader::getShapeIndex() { return ui->comboBox_Shape->currentIndex(); }

int InequalityLoader::getCombination() { return ui->comboBox_Combination->currentIndex(); }

int InequalityLoader::getPlot() { return ui->comboBox_Plot->currentIndex(); }

bool InequalityLoader::getSkip() { return flag_skip; }

QWidget* InequalityLoader::getFocusInWidget() { return ui->comboBox_Plot; }

QWidget* InequalityLoader::getFocusOutWidget()
{
    QWidget::setTabOrder(ui->comboBox_Plot, ui->pushButton_Details);
    return ui->pushButton_Details;
}

QColor InequalityLoader::getColor()
{
        switch(ui->comboBox_Color->currentIndex()){
        case Red:
            return QColor(Qt::red);
            break;
        case Green:
            return QColor(Qt::green);
            break;
        case Blue:
            return QColor(Qt::blue);
            break;
        case DarkRed:
            return QColor(Qt::darkRed);
            break;
        case DarkGreen:
            return QColor(Qt::darkGreen);
            break;
        case DarkBlue:
            return QColor(Qt::darkBlue);
            break;
        case Grey:
            return QColor(Qt::lightGray);
            break;
        case Black:
            return QColor(Qt::black);
            break;
        }
        return QColor(Qt::transparent);
}

QwtSymbol::Style InequalityLoader::getShape()
{
    switch(ui->comboBox_Shape->currentIndex()){
    case CrossX:
        return QwtSymbol::XCross;
        break;
    case CrossPlus:
        return QwtSymbol::Cross;
        break;
    case Circle:
        return QwtSymbol::Ellipse;
        break;
    case UpTriangle:
        return QwtSymbol::Triangle;
        break;
    case DownTriangle:
        return QwtSymbol::DTriangle;
        break;
    case Square:
        return QwtSymbol::Rect;
        break;
    case Diamond:
        return QwtSymbol::Diamond;
        break;
    }
    return QwtSymbol::NoSymbol;
}


//	Getters: Data
//	--------------

QVector<double> InequalityLoader::getX()
{
    return m_xResults_combination.empty() ? m_xResults[m_currentPlot] : m_xResults_combination;
}

QVector<double> InequalityLoader::getY()
{
    return m_yResults_combination.empty() ? m_yResults[m_currentPlot] : m_yResults_combination;
}

QVector<double> InequalityLoader::getXProblem()
{
    //	not yet implemented
    QVector <double> emptything = QVector<double>();
    return emptything;
}

QVector<double> InequalityLoader::getYProblem()
{
    //	not yet implemented
    QVector <double> emptything = QVector<double>();
    return emptything;
}

QString InequalityLoader::getFile() { return m_file.fileName(); }

QString InequalityLoader::getErrors() { return m_errorMessage; }


//	Parsers
//	--------

void InequalityLoader::parseFile(QString json)
{
    BlueJSON parser = BlueJSON(json.toStdString());
    string token;
    bool flag_cumulOK = true;


    //	main case

    flag_ok = parser.getNextKeyValue("name", token);
    flag_ok &= parser.getStringToken(token);
    checkOK("Main Case | Name.");
    flag_cumulOK &=flag_ok;

    if (token.empty()) { m_name = cropFileName(m_file.fileName()); }
    else {
        m_name = QString::fromStdString(token);
    }

    ui->label_CaseOut->setText("<b><i>" + m_name + "</b></i>");

    createDetailItem(m_detailLevel, "Case", m_name, "", "");
    m_detailLevel++;

    flag_ok = parser.getNextKeyValue("variables", token);
    checkOK("Main Case | Variables not found.");
    flag_cumulOK &=flag_ok;

    m_variables = QString::fromStdString(token);
    formatVariables(m_variables);


    //	main case plots

    while (parser.getNextKeyValue("plot", token)){
        BlueJSON plotparser = BlueJSON(token);

        formatPlot();
        m_detailLevel++;

        // expressions
        flag_ok = plotparser.getNextKeyValue("expressions", token);
        checkOK("Plot | Expressions not found.");
        flag_cumulOK &=flag_ok;


        token = BlueJSON::jsonObject(token);
        QString expressions = QString::fromStdString(token);

        m_expressions << expressions;
        formatExpressions(expressions);

        // data
        flag_ok = plotparser.getNextKeyValue("data", token);
        checkOK("Plot | Data not found.");
        flag_cumulOK &=flag_ok;

        parsePlotData(QString::fromStdString(token));

        m_detailLevel--;
    }

    flag_ok = flag_cumulOK;



    //	if input was invalid, kill the widget
    if (!flag_ok){
        emit killThis(m_guiNumber);
        return;
    }

    setComboBoxPlot();

}

void InequalityLoader::parsePlotData(QString json)
{
    string token;
    double point;
    QVector<double> x_vector, y_vector;
    BlueJSON parser = BlueJSON(json.toStdString());

    while ( parser.getNextKeyValue("x", token) ){
        flag_ok &= parser.getDoubleToken(point);
        x_vector.push_back(point);

        flag_ok &= parser.getNextKeyValue("y", token);
        flag_ok &= parser.getDoubleToken(point);
        y_vector.push_back(point);
    }

    checkOK("Data | problem parsing co-ordinate [" + QString::number(x_vector.count()) + "]");

    if (x_vector.size() != y_vector.size()){ flag_ok = false; }

    checkOK("Data | data set does not have the same amount of x-values as y-values\n");

    m_xResults << x_vector;
    m_yResults << y_vector;
}

void InequalityLoader::parseProblem(QString problem)
{
    m_errorMessage += "Error | Parsing | " + problem + "\n";
}

void InequalityLoader::formatPlot()
{
    createDetailItem(m_detailLevel, "Plot", "", "", "");
}

void InequalityLoader::formatVariables(QString json)
{
    string name, units, axis, token;
    int steps;
    double min, max;
    BlueJSON parser = BlueJSON(json.toStdString());

    while (parser.getNextKeyValue("name", name)){
        flag_ok &= parser.getStringToken(name);
        flag_ok &= parser.getNextKeyValue("min", token);
        flag_ok &= parser.getDoubleToken(min);
        flag_ok &= parser.getNextKeyValue("max", token);
        flag_ok &= parser.getDoubleToken(max);
        flag_ok &= parser.getNextKeyValue("elements", token);
        flag_ok &= parser.getIntToken(steps);
        flag_ok &= parser.getNextKeyValue("units", units);
        flag_ok &= parser.getStringToken(units);
        flag_ok &= parser.getNextKeyValue("axis", axis);
        flag_ok &= parser.getStringToken(axis);

        checkOK("Variable | Problem parsing [" + QString::fromStdString(name) + "]");

        stringstream buffer;
        buffer << "linear [" << min << ";" << max << "]"
                  " in " << steps << " steps";

        createDetailItem(m_detailLevel, "Variable", QString::fromStdString(name),
                         QString::fromStdString(buffer.str()) , QString::fromStdString(axis));
        buffer.flush();
    }
}

void InequalityLoader::formatInequality(QString json)
{
    BlueJSON parser = BlueJSON(json.toStdString() + ","); // comma allows final value to be parsed
    string desc, l_exp, sym, r_exp, comb;

    flag_ok &= parser.getNextKeyValue("description", desc);
    flag_ok &= parser.getNextKeyValue("left expression", l_exp);
    flag_ok &= parser.getNextKeyValue("symbol", sym);
    flag_ok &= parser.getNextKeyValue("right expression", r_exp);
    flag_ok &= parser.getNextKeyValue("combination", comb);

    flag_ok &= parser.getStringToken(desc);
    flag_ok &= parser.getStringToken(l_exp);
    flag_ok &= parser.getStringToken(sym);
    flag_ok &= parser.getStringToken(r_exp);
    flag_ok &= parser.getStringToken(comb);

    checkOK("Inequality | Problem parsing inequality.");

    m_dropDownList << QString::fromStdString(desc);

    stringstream buffer;
    buffer << l_exp << " " << sym << " " << r_exp;
    createDetailItem(m_detailLevel, "Inequality", QString::fromStdString(desc),
                     QString::fromStdString(buffer.str()), QString::fromStdString(comb));
}

void InequalityLoader::formatCase(QString json)
{
    BlueJSON parser = BlueJSON(json.toStdString());
    string case_name, variables, expressions;

    flag_ok &= parser.getNextKeyValue("name", case_name);
    flag_ok &= parser.getNextKeyValue("variables", variables);
    flag_ok &= parser.getNextKeyValue("expressions", expressions);

    flag_ok &= parser.getStringToken(case_name);

    checkOK("Case | Problem parsing case.");

    createDetailItem(m_detailLevel, "Case", QString::fromStdString(case_name), "", "");
    m_detailLevel++;
    formatVariables(QString::fromStdString(variables));
    formatExpressions(QString::fromStdString(expressions));
    m_detailLevel--;
}

void InequalityLoader::formatExpressions(QString json)
{
    string token;
    BlueJSON parser = BlueJSON(json.toStdString());
    vector<string> keys;

    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;

    while (parser.getNextKeyValue(keys, token, closest_key)){
        //	case found
        if (closest_key == 1){ formatCase(QString::fromStdString(token)); }
        //	inequality found
        else { formatInequality(QString::fromStdString(token)); }
    }
}

QString InequalityLoader::expressionToJSON()
{
    vector<string> properties;
    properties.push_back(BlueJSON::jsonKeyValue("name", m_name.toStdString()));
    properties.push_back(BlueJSON::jsonKeyValue("file", m_file.fileName().toStdString()));
    properties.push_back(BlueJSON::jsonKeyValue("variables", BlueJSON::jsonArray(m_variables.toStdString(), Flat)));
    properties.push_back(BlueJSON::jsonKeyValue("name", m_name.toStdString()));
    properties.push_back(BlueJSON::jsonKeyValue("expressions", m_expressions[m_currentPlot].toStdString()));

    string json = BlueJSON::jsonObject(properties);
    return QString::fromStdString(BlueJSON::jsonKeyValue("case", json));
}

QString InequalityLoader::dataToJSON()
{
    QVector<double> x= getX();
    QVector<double> y= getY();
    vector<string> data_array;

    // create point objects
    for (int i = 0; i < x.size(); i++){
        vector<string> point;
        point.push_back( BlueJSON::jsonKeyValue("x", x[i]) );
        point.push_back( BlueJSON::jsonKeyValue("y", y[i]) );
        data_array.push_back( BlueJSON::jsonObject(point, Flat));
    }

    // create & return data array
    string json = BlueJSON::jsonArray(data_array);
    json = BlueJSON::jsonKeyValue("data", json);
    return QString::fromStdString(json);
}

inline void InequalityLoader::checkOK(QString message)
{
   if(!flag_ok)
       parseProblem(message);
}


//	Evaluation
//	-----------

void InequalityLoader::setPlot(int index)
{
    if (index < 0 ||
        index > (ui->comboBox_Plot->count() - 1) ){
        m_currentPlot = ui->comboBox_Plot->currentIndex();
    }
    else { m_currentPlot = index; }
}


//	GUI
//	----

void InequalityLoader::enablePositionButtons (bool flag_enable)
{
    ui->pushButton_Down->setEnabled(flag_enable);
    ui->pushButton_Up->setEnabled(flag_enable);
    ui->comboBox_Combination->setEnabled(flag_enable);
    ui->pushButton_Delete->setEnabled(flag_enable);
}

void InequalityLoader::enableCombinations(bool flag_enable) { ui->comboBox_Combination->setEnabled(flag_enable); }

void InequalityLoader::resetCombinations() { ui->comboBox_Combination->setCurrentIndex(CombinationNone); }

void InequalityLoader::setComboBoxPlot()
{
    if (ui->comboBox_Plot->count() == 0){
        QStringList combo_data;
        for (int i = 0; i < m_xResults.count(); i++){
            stringstream buffer;
            if (i < m_dropDownList.count()){
                buffer << m_dropDownList[i].toStdString();
            } else {
                buffer << "Plot " << i;
            }
            combo_data << QString::fromStdString(buffer.str());
        }
        ui->comboBox_Plot->clear();
        ui->comboBox_Plot->insertItems(0, combo_data);
    }
}

void InequalityLoader::createDetailDialog()
{
    QDialog *dialog = new QDialog(this->parentWidget());
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QTreeWidget *tree = new QTreeWidget(dialog);
    QTreeWidgetItem *header = tree->headerItem();
    QTreeWidgetItem *child = new QTreeWidgetItem(tree);
    QList<QTreeWidgetItem*> *parents = new QList<QTreeWidgetItem*>();

    int prev_lvl = 0;
    int cur_lvl;

    dialog->setWindowTitle("BareMinimumPlotter :: Details :: Inequality " + QString::number(getNumber() + 1));

    header->setText(0, "Type");
    header->setText(1, "Description");
    header->setText(2, "Data 1");
    header->setText(3, "Data 2");

    child->setText(0, m_details[0].type);
    child->setText(1, m_details[0].description);
    child->setText(2, m_details[0].data1);
    child->setText(3, m_details[0].data2);
    parents->push_back(child);
    prev_lvl++;

    for (int i = 1; i < m_details.count(); i++){
        cur_lvl = m_details[i].level;

        while(prev_lvl > cur_lvl){
            parents->takeLast();
            prev_lvl--;
        }
        if ( prev_lvl < cur_lvl){
            parents->append(child);
        }

        child = new QTreeWidgetItem( parents->last() );

        child->setText(0, m_details[i].type);
        child->setText(1, m_details[i].description);
        child->setText(2, m_details[i].data1);
        child->setText(3, m_details[i].data2);

        prev_lvl = cur_lvl;
    }


    layout->addWidget(tree);
    dialog->setLayout(layout);
    dialog->show();

    tree->expandAll();
}


///	Public Slots
///	=============

void InequalityLoader::splitterResize(QList<int> sizes){ ui->splitter_InequalityInput->setSizes(sizes); }


///	Private Functions
/// ==================

//	Private Slots
//	--------------

void InequalityLoader::on_pushButton_Details_clicked()
{
    createDetailDialog();
}

void InequalityLoader::on_pushButton_Delete_clicked() { emit killThis(m_guiNumber); }

void InequalityLoader::on_pushButton_Up_clicked() { emit moveUp(m_guiNumber); }

void InequalityLoader::on_pushButton_Down_clicked() { emit moveDown(m_guiNumber); }

void InequalityLoader::on_comboBox_Combination_currentIndexChanged(int index)
{
   switch (index) {
    case CombinationNone:
       ui->comboBox_Color->setEnabled(true);
       ui->comboBox_Shape->setEnabled(true);
       break;
   default:
       ui->comboBox_Color->setEnabled(false);
       ui->comboBox_Shape->setEnabled(false);
       break;
   }
}

void InequalityLoader::on_checkBox_Skip_toggled(bool checked)
{
   if(checked) {
       flag_skip = true;
       ui->comboBox_Color->setEnabled(false);
       ui->comboBox_Shape->setEnabled(false);
       ui->comboBox_Combination->setEnabled(false);
   }
   else {
       flag_skip = false;
       ui->comboBox_Color->setEnabled(true);
       ui->comboBox_Shape->setEnabled(true);
       if(ui->pushButton_Down->isEnabled())
           ui->comboBox_Combination->setEnabled(true);
   }
}

void InequalityLoader::on_comboBox_Plot_currentIndexChanged(int) { setPlot(); }
