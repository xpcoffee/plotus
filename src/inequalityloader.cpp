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

    string case_name, variables, plot, expressions, data, desc;

    parser.getNextKeyValue("name", case_name); 		// case name
    parser.getStringToken(case_name);
    if (case_name.empty())
        m_name = cropFileName(filename);
    else
        m_name = case_name;
    ui->label_CaseOut->setText(QString::fromStdString("<b><i>" + m_name + "</b></i>"));
    ui->label_CaseOut->setText(QString::fromStdString("<b><i>" + case_name + "</b></i>"));

    parser.getNextKeyValue("variables", variables); // variables

    while (parser.getNextKeyValue("plot", plot)){ 	// get plots
        BlueJSON plotparser = BlueJSON(plot);

        // description

        // expressions
        plotparser.getNextKeyValue("expressions", expressions);
        m_detailsJSON.push_back("\"variables\":[" + variables + "],\n"			// 	JSON for saving
                                "\"expressions\":{" + expressions + "}\n");
        m_detailsHTML.push_back(formatVariables(variables) +					//	Formatted for display
                            "<hr>" +
                            "<table align=\"center\" cellspacing=\"10\">" +
                            "<tr><th>Description</th>" +
                            "<th>Expression</th>" +
                            "<th>Combination</th></tr>" +
                            formatExpressions(expressions) +
                            "</table>");

        // data
        plotparser.getNextKeyValue("data", data);
        parsePlotData(data);
    }

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


//	Getters: UI
//	------------

QString InequalityLoader::getName() 		{ return ui->comboBox_Plot->currentText(); }

int	InequalityLoader::getNumber() 		{ return m_guiNumber; }

int InequalityLoader::getShapeIndex() 	{ return ui->comboBox_Shape->currentIndex(); }

int InequalityLoader::getCombination() 	{ return ui->comboBox_Combination->currentIndex(); }

int InequalityLoader::getPlot() { return ui->comboBox_Plot->currentIndex(); }

bool InequalityLoader::getSkip() 		{ return flag_skip; }

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
    while (parser.getNextKeyValue("x", token)){
            if(parser.getDoubleToken(point))
                x_vector.push_back(point);
            if (parser.getNextKeyValue("y", token)){
                if(parser.getDoubleToken(point))
                    y_vector.push_back(point);
            }
    }
    if (x_vector.size() != y_vector.size()){
//        flag_problem = true;
        m_errorMessage += "Parsing Error | Data | data set does not have the same amount of x-values as y-values\n";
    }
    m_xResults.push_back(x_vector);
    m_yResults.push_back(y_vector);
}

void InequalityLoader::parseProblem(string problem)
{
    m_errorMessage += "Error | Parsing| " + problem + "\n";
}

string InequalityLoader::formatName(string token)
{
    stringstream buffer;
    BlueJSON parser = BlueJSON(token);
    buffer << "<tr>";
    buffer << "<td align=\"center\">";
    buffer << "<b>" << parser.getStringToken(token) << "</b>";
    buffer << "</td>";
    buffer << "</tr>";
    return buffer.str();
}

string InequalityLoader::formatVariables(string json)
{
    stringstream buffer;
    string token;
    int int_token;
    BlueJSON parser = BlueJSON(json);
    buffer << "<table cellspacing=\"10\">";
    buffer << "<style></style>";
    buffer << "<tr> <th>Variable</th> <th>Min</th> <th>Max</th> <th>Steps</th> <th>Units</th> <th>Axis</th></tr>";
    while (parser.getNextKeyValue("name", token)){
            buffer << "<tr>";
            buffer << "<td align=\"center\">";
            if(parser.getStringToken(token))
                buffer<< token;
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("min", token)){
                if(parser.getIntToken(int_token))
                    buffer<< int_token;
            }
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("max", token)){
                if(parser.getIntToken(int_token))
                    buffer << int_token;
            }
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("elements", token)){
                if(parser.getIntToken(int_token))
                    buffer << int_token;
            }
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("units", token)){
                if(parser.getStringToken(token))
                    buffer << token ;
            }
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("axis", token)){
                if(parser.getStringToken(token))
                    buffer << token;
            }
            buffer << "</td>";
            buffer << "</tr>";
    }
    buffer << "</table>";
    return buffer.str();
}

string InequalityLoader::formatInequality(string json)
{
    BlueJSON parser = BlueJSON(json + ","); // comma allows final value to be parsed
    stringstream buffer;
    string token;

//  description
    if (!parser.getNextKeyValue("description", token)){
        parseProblem("Inequality | description");
        return "";
    }
    buffer << "<td align=\"center\">";
    buffer << token;
    buffer << "</td>";

//	left expression
    if (!parser.getNextKeyValue("left expression", token)){
        parseProblem("Inequality | left expression");
        return "";
    }
    buffer << "<td align=\"center\">";
    parser.getStringToken(token);
    buffer << token << " ";

//	symbol
    if (!parser.getNextKeyValue("symbol", token)){
        parseProblem("Inequality | symbol");
        return "";
    }
    parser.getStringToken(token);
    buffer << token << " ";

//	right expression
    if (!parser.getNextKeyValue("right expression", token)){
        parseProblem("Inequality | right expression");
        return "";
    }
    parser.getStringToken(token);
    buffer << token << " ";
    buffer << "</td>";

//	combination
    buffer << "<td align=\"center\">";
    if (!parser.getNextKeyValue("combination", token)){
        parseProblem("Inequality | combination");
        return "";
    }
    parser.getStringToken(token);
    buffer << token;
    buffer << "</td>";

//	return
    return buffer.str();
}

string InequalityLoader::formatCase(string json)
{
    BlueJSON parser = BlueJSON(json);
    string case_name, variables, expressions;
    parser.getNextKeyValue("name", case_name);
    parser.getStringToken(case_name);
    parser.getNextKeyValue("variables", variables);
    parser.getNextKeyValue("expressions", expressions);
    return formatName(case_name) + formatVariables(variables) + formatExpressions(expressions);
}

string InequalityLoader::formatExpressions(string json)
{
    stringstream buffer;
    string token;
    BlueJSON parser = BlueJSON(json);
    vector<string> keys;
    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;
    while (parser.getNextKeyValue(keys,token, closest_key)){
            if (closest_key == 1){	//	case found
                buffer << "<tr> <td>case</td> <td></td> </tr>";
                buffer << formatCase(token);
                buffer << "<tr> <td>end case</td> <td></td> </tr>";
            } else {				//	inequality found
                buffer << "<tr>";
                buffer << formatInequality(token);
                buffer << "</tr>";
            }
    }
    return buffer.str();
}

string InequalityLoader::expressionToJSON()
{
    return 	"\"case\":{\n"
            "\"name\":\"" + m_name + "\",\n"
            "\"file\":\"" + m_filename + "\",\n" +
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
                   "\"x\":"<< x_vector[j] << ","
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
            if (m_descriptions.count() < i){
                buffer << m_descriptions[i].toStdString();
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
    for (int i = 0; i < static_cast<int>(m_detailsHTML.size()); i++){
        stringstream buffer;
        buffer << "<b>" << m_name << "</b><hr>" << m_detailsHTML[i];
        message.append(QString::fromStdString(buffer.str()));
    }
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
