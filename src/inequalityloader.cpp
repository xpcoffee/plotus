#include "include/inequalityloader.h"
#include "ui_inequalityloader.h"

///	Static Functions
///	=================

static string cropFileName(string filename)
{
        string filename_short;
        stringstream ss_name;
        ss_name << filename;

        while (getline(ss_name, filename_short, '/')){}
        return filename_short.substr(0, filename_short.size() - 5); // .json is 5 characters long
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
    flag_problem(false),
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

void InequalityLoader::setCaseName(string value)
{
    if (!value.empty())
        ui->label_CaseOut->setText(QString::fromStdString(value));
}


//	TODO: insert error checking (flag_problem)

void InequalityLoader::loadCase(string filename)
{
    if (filename.empty()){
        emit killThis(m_guiNumber);
        return;
    }

    //	read in file
    m_filename = filename;
    BlueJSON parser;
    parser.readInFile(filename);

    //	parse
    m_detailsHTML.clear();

    string case_name, plot, expressions, data;

    //! main case
    parser.getNextKeyValue("name", case_name); 		// case name

    if (case_name.empty()) { m_name = cropFileName(filename); }
    else {
        parser.getStringToken(case_name);
        m_name = case_name;
    }

    createDetailItem(m_detailLevel, "Case", QString::fromStdString(m_name), "", "");
    m_detailLevel++;

    ui->label_CaseOut->setText(QString::fromStdString("<b><i>" + m_name + "</b></i>"));

    //! main case variables
    parser.getNextKeyValue("variables", m_variables);
    formatVariables(m_variables);

    //! main case plots
    while (parser.getNextKeyValue("plot", plot)){
        BlueJSON plotparser = BlueJSON(plot);

        createDetailItem(m_detailLevel, "Plot", "", "", "");
        m_detailLevel++;

        // expressions
        plotparser.getNextKeyValue("expressions", expressions);

        m_detailsJSON.push_back("\"expressions\":{" + expressions + "}\n");
        formatExpressions(expressions);

        // data
        plotparser.getNextKeyValue("data", data);
        parsePlotData(data);

        m_detailLevel--;
    }

    m_detailsHTML.push_back( "End Case: <b>" + m_name + "</b>" );

    setComboBoxPlot();
    return;

    //	if input was invalid, kill the widget
    if (flag_problem){
        emit killThis(m_guiNumber);
        return;
    }

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

string InequalityLoader::getFile() { return m_filename; }

string InequalityLoader::getErrors() { return m_errorMessage; }


//	Parsers
//	--------

void InequalityLoader::parsePlotData(string json)
{
    string token;
    double point;
    QVector<double> x_vector, y_vector;
    BlueJSON parser = BlueJSON(json);

    while ( parser.getNextKeyValue("x", token) ){
        parser.getDoubleToken(point);
        x_vector.push_back(point);

        parser.getNextKeyValue("y", token);
        parser.getDoubleToken(point);
        y_vector.push_back(point);
    }

    if (x_vector.size() != y_vector.size()){
//        flag_problem = true;
        parseProblem("Data | data set does not have the same amount of x-values as y-values\n");
    }

    m_xResults.push_back(x_vector);
    m_yResults.push_back(y_vector);
}

void InequalityLoader::parseProblem(string problem)
{
    m_errorMessage += "Error | Parsing | " + problem + "\n";
}

void InequalityLoader::formatVariables(string json)
{
    string name, units, axis, token;
    int steps;
    double min, max;
    BlueJSON parser = BlueJSON(json);

    while (parser.getNextKeyValue("name", name)){
        parser.getStringToken(name);
        parser.getNextKeyValue("min", token);
        parser.getDoubleToken(min);
        parser.getNextKeyValue("max", token);
        parser.getDoubleToken(max);
        parser.getNextKeyValue("elements", token);
        parser.getIntToken(steps);
        parser.getNextKeyValue("units", units);
        parser.getStringToken(units);
        parser.getNextKeyValue("axis", axis);
        parser.getStringToken(axis);

        stringstream buffer;
        buffer << "linear [" << min << ";" << max << "]"
                  " in " << steps << " steps";

        createDetailItem(m_detailLevel, "Variable", QString::fromStdString(name),
                         QString::fromStdString(buffer.str()) , QString::fromStdString(axis));
        buffer.flush();
    }
}

void InequalityLoader::formatInequality(string json)
{
    BlueJSON parser = BlueJSON(json + ","); // comma allows final value to be parsed
    string desc, l_exp, sym, r_exp, comb;

    parser.getNextKeyValue("description", desc);
    parser.getNextKeyValue("left expression", l_exp);
    parser.getNextKeyValue("symbol", sym);
    parser.getNextKeyValue("right expression", r_exp);
    parser.getNextKeyValue("combination", comb);

    parser.getStringToken(desc);
    parser.getStringToken(l_exp);
    parser.getStringToken(sym);
    parser.getStringToken(r_exp);
    parser.getStringToken(comb);

    m_dropDownList << QString::fromStdString(desc);

    stringstream buffer;
    buffer << l_exp << " " << sym << " " << r_exp;
    createDetailItem(m_detailLevel, "Inequality", QString::fromStdString(desc),
                     QString::fromStdString(buffer.str()), QString::fromStdString(comb));
}

void InequalityLoader::formatCase(string json)
{
    BlueJSON parser = BlueJSON(json);
    string case_name, variables, expressions;

    parser.getNextKeyValue("name", case_name);
    parser.getNextKeyValue("variables", variables);
    parser.getNextKeyValue("expressions", expressions);

    parser.getStringToken(case_name);

    createDetailItem(m_detailLevel, "Case", QString::fromStdString(case_name), "", "");
    m_detailLevel++;
    formatVariables(variables);
    formatExpressions(expressions);
    m_detailLevel--;
}

void InequalityLoader::formatExpressions(string json)
{
    string token;
    BlueJSON parser = BlueJSON(json);
    vector<string> keys;

    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;

    while (parser.getNextKeyValue(keys, token, closest_key)){
        //	case found
        if (closest_key == 1){ formatCase(token); }
        //	inequality found
        else { formatInequality(token); }
    }
}

string InequalityLoader::expressionToJSON()
{
    return 	"\"case\":{\n"
            "\"name\":\"" + m_name + "\",\n"
            "\"file\":\"" + m_filename + "\",\n"
            "\"variables\":[" + m_variables + "],\n" +
            m_detailsJSON[m_currentPlot] +
            "}\n";
}

string InequalityLoader::dataToJSON()
{
    stringstream buffer;
    QVector<double> x_vector = m_xResults[m_currentPlot];
    QVector<double> y_vector = m_yResults[m_currentPlot];

    buffer << "\"data\":[";
    for (int j = 0; j < x_vector.size(); j++){
        buffer << "{"
                   "\"x\":" << x_vector[j] << ","
                   "\"y\":" << y_vector[j] <<
                   "}";
        if (j != x_vector.size()-1){
            buffer << ",";
            buffer << "\n";
        }
    }
    // close plot data
    buffer << "]\n";
    return buffer.str();
}

template <typename T>
inline string InequalityLoader::htmlTable(T value, string options)
{
    stringstream buffer;
    buffer << "<table " << options << ">" << value << "</table>";
    return buffer.str();
}

template <typename T>
inline string InequalityLoader::htmlTableCell(T value, string options)
{
    stringstream buffer;
    buffer << "<td " << options << ">" << value << "</td>";
    return buffer.str();
}

template <typename T>
inline string InequalityLoader::htmlTableRow(T value, string options)
{
    stringstream buffer;
    buffer << "<tr " << options << ">" << value << "</tr>";
    return buffer.str();
}

template <typename T>
inline string InequalityLoader::htmlTableHeader(T value, string options)
{
    stringstream buffer;
    buffer << "<th " << options << ">" << value << "</th>";
    return buffer.str();
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
        for (unsigned int i = 0; i < m_xResults.size(); i++){
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


///	Public Slots
///	=============

void InequalityLoader::splitterResize(QList<int> sizes){ ui->splitter_InequalityInput->setSizes(sizes); }


///	Private Functions
/// ==================

//	Private Slots
//	--------------

void InequalityLoader::on_pushButton_Details_clicked()
{
    QMessageBox *dialog = new QMessageBox(0);
    QString message = "";

    stringstream buffer;
        buffer << htmlTableRow( htmlTableHeader("Level") +
                                htmlTableHeader("Type", "align=\"left\"") +
                                htmlTableHeader("Description") +
                                htmlTableHeader("Data1") +
                                htmlTableHeader("Data2") );

    for (int i = 0; i < m_details.count(); i++){
        QString indent = "";
        for (int j = 0; j < m_details[i].level; j++)
            indent.append("~");

        buffer << htmlTableRow( htmlTableCell(m_details[i].level) +
                                htmlTableCell(indent.toStdString() + m_details[i].type.toStdString()) +
                                htmlTableCell(m_details[i].description.toStdString()) +
                                htmlTableCell(m_details[i].data1.toStdString()) +
                                htmlTableCell(m_details[i].data2.toStdString()) );
    }
    message.append(QString::fromStdString( htmlTable(buffer.str(), "cellspacing=\"10\" align=\"left\"") ));

    dialog->setText(message);
    dialog->show();
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
