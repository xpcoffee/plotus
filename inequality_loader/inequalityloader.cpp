#include "inequalityloader.h"
#include "ui_inequalityloader.h"

///	Static Functions
///	=================

// [BREAK] creating a JSON parser to find value of next specified key
//	need this to be able to pick out JSON data to save (must save the data of stuff being plotted only)
getNextValue(string key, string text, int start){
    string token;
    istringstream iss;
    iss << string.substr(start, text.size()-start);
    while (getline(iss, token, '"')){
        if (token == key){

        }
    }

}

///	Public Functions
///	=================

//	Constructor
//	-----------

InequalityLoader::InequalityLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityLoader),
    m_gui_number(-1),
    flag_skip(false),
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
    ui->label_Number->setNum(number+1);
}

void InequalityLoader::setCaseName(string value)
{
    if (!value.empty())
        ui->label_CaseOut->setText(QString::fromStdString(value));
}

static string extractExpression(string token, bool &flag_problem, string &m_error_message){
    int nCount = 0;
   string token_exp;
   string sDetails;
   stringstream iss;
   iss << token;
    while ( getline(iss, token_exp, '"')){
        if (token_exp == "left expression")	{
            if (getline (iss, token_exp, '"'))
                if (getline (iss, token_exp, '"'))
                    sDetails += token_exp; nCount++;
        } else if (token_exp == "symbol") {
            if (getline (iss, token_exp, '"'))
                if (getline (iss, token_exp, '"'))
                    sDetails += + " " + token_exp + " "; nCount++;
        } else if (token_exp == "right expression") {
            if (getline (iss, token_exp, '"'))
                if (getline (iss, token_exp, '"'))
                    sDetails += token_exp + "\n"; nCount++;
        } else if (token_exp == "combination") {
            sDetails += "Combination: "; nCount++;
            if (getline (iss, token_exp, '"')){
                if (getline (iss, token_exp, '"')){
                   stringstream jss;
                   int index;
                   jss << token_exp;
                   if(!(jss >> index))
                       index = -1;
                   switch (index){
                    case 0:
                       sDetails += "None\n";
                       break;
                   case 1:
                       sDetails += "Intersection\n";
                       break;
                   case 2:
                       sDetails += "Union\n";
                       break;
                   case 3:
                       sDetails += "Subtraction\n";
                       break;
                   default:
                       m_error_message += "Parsing Error | Combination\n";
                       flag_problem = true;
                       break;
                    }
                }
            }
        }
    }
    if (nCount != 4){
        flag_problem = true;
        m_error_message += "Parsing Error | Inequality\n";
    }
    return sDetails;
}

static string extractVariables(string token, bool &flag_problem, string &m_error_message){
    int nCount = 0;
    string token_var;
    stringstream iss;
    string sDetails;
    iss << token;
    while (getline(iss, token_var, '"')){
        if (token_var == "name" )
            if (getline (iss, token_var, '"'))
                if (getline (iss, token_var, '"'))
                { sDetails += "\n" + token_var + "\t"; nCount=1; }
        if (token_var == "min" )
            if (getline (iss, token_var, ':'))
                if (getline (iss, token_var, ','))
                { sDetails += "Min: " + token_var + "\t"; nCount++; }
        if (token_var == "max" )
            if (getline (iss, token_var, ':'))
                if (getline (iss, token_var, ','))
                { sDetails += "Max: " + token_var + "\t"; nCount++; }
        if (token_var == "elements" )
            if (getline (iss, token_var, ':'))
                if (getline (iss, token_var, ','))
                { sDetails += "Steps: " + token_var + "\t"; nCount++; }
        if (token_var == "units" )
            if (getline (iss, token_var, '"'))
                if (getline (iss, token_var, '"')) {
                    sDetails += "Units: " + token_var + "\t"; nCount++;
                    if (nCount != 5){
                        flag_problem = true;
                        m_error_message += "Parsing Error | Variable\n";
                    }
                }
        if (token_var == "axis" )
            if (getline (iss, token_var, '"'))
                if (getline (iss, token_var, '"')){
                    sDetails += "{" + token_var + "-axis}";
                }
        if (token_var == "slider point" )
            if (getline (iss, token_var, ':'))
                if (getline (iss, token_var, '}')){
                    sDetails += "{const: " + token_var + "}";
                }
    }
    return sDetails;
}

void InequalityLoader::loadCase(string filename)
{
    if (filename.empty())
        return;
    m_filename = filename;

    string token;
    ifstream ss(filename.c_str());

    if(ss.is_open()){
        bool flag_problem = false;
        sDetails = "";
        string JSON_string = "";
        m_x_results.clear();
        m_y_results.clear();
        QVector<double> qvX;
        QVector<double> qvY;

        string filename_short;
        stringstream ss_name;
        ss_name << filename;

                    // crop file name [not really necessary]
                    while (getline(ss_name, filename_short, '/')){}
                    ss_name << filename_short;
                    ui->label_CaseOut->setText(QString::fromStdString("<b><i>" + filename_short +"<i\\><b\\>"));

        while ( getline(ss, token, '"')){
            // inequalities
            if (token == "inequality") {			// beginning of inequality
                sDetails += "Inequality:\n";
                if (getline(ss, token, '{'))
                    if (getline(ss, token, '}')){
                        sDetails += extractExpression(token, flag_problem, m_error_message);
                        InequalityInput tmp = new InequalityInput();
                        tmp.fromJSON(token);
                        JSON_string += tmp.expressionToJSON();
                    }
            }

            // variables
            if (token == "variables") {			// beginning of inequality
                sDetails += "Variables:";
                if (getline (ss, token, '['))
                    if (getline (ss, token, ']')){
                        sDetails += extractVariables(token, flag_problem, m_error_message);
                    }
            }

            // plot data
            if (token == "data"){				// beginning of plot data
                if (getline(ss, token,'['))
                    if (getline(ss, token,']')){
                       string token_plot;
                       stringstream iss;
                       iss << token;
                       while (getline(iss, token_plot, '"')){
                            if (token_plot == "x")
                                if (getline (iss, token_plot, ':'))
                                    if (getline (iss, token_plot, ',')){
                                        qvX.push_back(atof(token_plot.c_str()));
                                    }

                            if (token_plot == "y")
                                if (getline (iss, token_plot, ':'))
                                    if (getline (iss, token_plot,'}')){
                                        qvY.push_back(atof(token_plot.c_str()));
                                    }
                        }
                        if (!qvX.empty()){
                            m_x_results.push_back(qvX);
                            m_y_results.push_back(qvY);
                        }
                        // push plot details
                        m_details.push_back(sDetails);
                        sDetails = "";
                    }
            }	// plot data
        }	// end of file

        if (m_x_results.empty() || m_y_results.empty() || (m_y_results.size() != m_x_results.size())){
            flag_problem = true;
            m_error_message += "Parsing Error | Plot data\n";
        }

        //	if input was invalid, kill the widget
        if (flag_problem)
            emit killThis(m_gui_number);

        QStringList combo_data;
        for (unsigned int i = 0; i < m_x_results.size(); i++){
            stringstream buffer;
            buffer << "Plot " << i;
            combo_data << QString::fromStdString(buffer.str());
        }

        ui->comboBox_Plot->clear();
        ui->comboBox_Plot->insertItems(0, combo_data);
    }	// open file

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

QVector<double> InequalityLoader::getX()
{
    return m_x_results[m_current_plot];
}

QVector<double> InequalityLoader::getY()
{
   return m_y_results[m_current_plot];
}

string InequalityLoader::getFile() { return m_filename; }

string InequalityLoader::getErrors() { return m_error_message; }


//	Parsers
//	--------

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
        buffer << "Plot " << i << "\n-----------\n" << m_details[i];
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
