#include "inequalityloader.h"
#include "ui_inequalityloader.h"

///	Static Functions
///	=================

static string cropFileName(string filename)
{
        string filename_short;
        stringstream ss_name;
        ss_name << filename;

        while (getline(ss_name, filename_short, '/')){}
        return filename_short;

}

///	Public Functions
///	=================

//	Constructors
//	-------------

InequalityLoader::InequalityLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityLoader),
    m_gui_number(-1),
    flag_skip(false),
    flag_problem(false),
    m_error_message(""),
    m_current_plot(0)
{
    ui->setupUi(this);
    setAccessibleDescription("loader");
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
        m_gui_number = number;
    ui->label_Number->setNum(number + 1);
}

void InequalityLoader::setCaseName(string value)
{
    if (!value.empty())
        ui->label_CaseOut->setText(QString::fromStdString(value));
}



// [BREAK] 11 July 2014 | implementing JSON parser

void InequalityLoader::loadCase(string filename)
{
    if (filename.empty())
        return;

    m_filename = filename;
    m_details.clear();

    string token;
    // Exprimental
    string file = "";
    ifstream iss(filename.c_str());
    if (!iss.is_open())
        return;
    while (getline(iss, token)){
        file += token + "\n";
    }
    iss.close();

    string variables, plot, expressions, data;
    BlueJSON parser = BlueJSON(file);
    // get case variables
    parser.getNextKeyValue("variables", variables);
    // get plots
    while (parser.getNextKeyValue("plot", plot)){
        // get data and expressions
        BlueJSON plotparser = BlueJSON(plot);
        if(plotparser.getNextKeyValue("expressions", expressions)){
            m_expressions.push_back(variables + expressions);
            m_details.push_back(formatVariables(variables) +
                                "<hr>" +
                                "<table align=\"center\" cellspacing=\"10\">" +
                                "<tr><th>Type</th><th>Expressions</th><th>Combination</th><tr>" +
                                formatExpressions(expressions) +
                                "</table>");
        }
        plotparser.getNextKeyValue("data", data);
        parsePlotData(data);
    }

    setComboBoxPlot();
    return;

    //	if input was invalid, kill the widget
    if (flag_problem){
        emit killThis(m_gui_number);
        return;
    }

}

void InequalityLoader::setX(QVector<double> vector) { m_x_results[m_current_plot] = vector; }

void InequalityLoader::setY(QVector<double> vector) { m_y_results[m_current_plot] = vector; }


//	Getters
//	--------

int	InequalityLoader::getNumber() 		{ return m_gui_number; }

int InequalityLoader::getColorIndex() 	{ return ui->comboBox_Color->currentIndex(); }

int InequalityLoader::getShapeIndex() 	{ return ui->comboBox_Shape->currentIndex(); }

int InequalityLoader::getCombination() 	{ return ui->comboBox_Interact->currentIndex(); }

bool InequalityLoader::getSkip() 		{ return flag_skip; }

QVector<double> InequalityLoader::getX() { return m_x_results[m_current_plot]; }

QVector<double> InequalityLoader::getY() { return m_y_results[m_current_plot]; }

string InequalityLoader::getFile() { return m_filename; }

string InequalityLoader::getErrors() { return m_error_message; }



//	Parsers
//	--------

void InequalityLoader::parsePlotData(string json){
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
        m_error_message += "Parsing Error | Data | data set does not have the same amount of x-values as y-values\n";
    }
    m_x_results.push_back(x_vector);
    m_y_results.push_back(y_vector);
}

// [BREAK] 12 July 2014 | formatting nested cases in HTML
string InequalityLoader::formatExpressions(string json){
    stringstream buffer;
    string token;
    BlueJSON parser = BlueJSON(json);
    vector<string> keys;
    keys.push_back("inequality");
    keys.push_back("case");
    int closest_key;
    while (parser.getNextKeyValue(keys,token, closest_key)){
            if (closest_key = 1){	 // case found
                buffer << "<tr><td>case</td><td></td><td></td></tr>";
                buffer << formatExpressions(token);
                buffer << "<tr><td>end case</td><td></td><td></td></tr>";
            }
            parser.getNextKeyValue("left expression", token);
            buffer << "<tr>";
            buffer << "<td align=\"center\">";
            if(parser.getStringToken(token))
                buffer << token << " ";
            if (parser.getNextKeyValue("symbol", token)){
                if(parser.getStringToken(token))
                    buffer << token << " ";
            }
            if (parser.getNextKeyValue("right expression", token)){
                if(parser.getStringToken(token))
                    buffer << token << " ";
            }
            buffer << "</td>";
            buffer << "<td align=\"center\">";
            if (parser.getNextKeyValue("combination", token)){
                if(parser.getStringToken(token))
                    buffer << token;
            }
            buffer << "</td>";
            buffer << "</tr>";
    }
    return buffer.str();
}

string InequalityLoader::formatVariables(string json){
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

string InequalityLoader::expressionToJSON()
{
    return m_expressions[m_current_plot];
}

string InequalityLoader::dataToJSON(){
    stringstream buffer;
    QVector<double> x_vector = m_x_results[m_current_plot];
    QVector<double> y_vector = m_y_results[m_current_plot];

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

void InequalityLoader::setPlot() {
    m_current_plot = ui->comboBox_Plot->currentIndex();
}

//	GUI
//	----

void InequalityLoader::enablePositionButtons (bool flag_enable)
{
    ui->pushButton_Down->setEnabled(flag_enable);
    ui->pushButton_Up->setEnabled(flag_enable);
    ui->comboBox_Interact->setEnabled(flag_enable);
    ui->pushButton_Remove->setEnabled(flag_enable);
}

void InequalityLoader::enableCombinations(bool flag_enable) { ui->comboBox_Interact->setEnabled(flag_enable); }

void InequalityLoader::resetCombinations() { ui->comboBox_Interact->setCurrentIndex(COMBINE_NONE); }

void InequalityLoader::setComboBoxPlot(){
    if (ui->comboBox_Plot->count() == 0){
        QStringList combo_data;
        for (unsigned int i = 0; i < m_x_results.size(); i++){
            stringstream buffer;
            buffer << "Plot " << i;
            combo_data << QString::fromStdString(buffer.str());
        }
        ui->comboBox_Plot->clear();
        ui->comboBox_Plot->insertItems(0, combo_data);
    }
}



///	Private Functions
/// ==================

//	Private Slots
//	--------------

void InequalityLoader::on_pushButton_Details_clicked()
{
    QMessageBox *dialog = new QMessageBox(0);
    QString message = "";
    for (int i = 0; i < static_cast<int>(m_details.size()); i++){
        stringstream buffer;
        buffer << "<b>Plot " << i << "</b><hr>" << m_details[i];
        message.append(QString::fromStdString(buffer.str()));
    }
    dialog->setText(message);
    dialog->show();
}

void InequalityLoader::on_pushButton_Remove_clicked() { emit killThis(m_gui_number); }

void InequalityLoader::on_pushButton_Up_clicked() { emit moveUp(m_gui_number); }

void InequalityLoader::on_pushButton_Down_clicked() { emit moveDown(m_gui_number); }

void InequalityLoader::on_comboBox_Interact_currentIndexChanged(int index)
{
   switch (index) {
    case COMBINE_NONE:
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
       ui->comboBox_Interact->setEnabled(false);
   }
   else {
       flag_skip = false;
       ui->comboBox_Color->setEnabled(true);
       ui->comboBox_Shape->setEnabled(true);
       if(ui->pushButton_Down->isEnabled())
           ui->comboBox_Interact->setEnabled(true);
   }
}

void InequalityLoader::on_comboBox_Plot_currentIndexChanged(int) { setPlot(); }
