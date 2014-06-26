#include "inequalityloader.h"
#include "ui_inequalityloader.h"

///	Public Functions
//	Constructor
InequalityLoader::InequalityLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InequalityLoader)
{
    ui->setupUi(this);
    // initialize variables
    flag_skip = false;
    sType = "loader";
    nInequalityInputNumber = -1;
    beginPlot();
    //	experimental
    QString filename = QFileDialog::getOpenFileName(this, "Open plot", ".", "JSON (*.json)");
    loadCase(filename.toStdString());
}

//	Destructor
InequalityLoader::~InequalityLoader()
{
    delete ui;
}


// 	Setters
void InequalityLoader::setNumber(int nNumber = -1) {
    if (!(nNumber > -1))
        nInequalityInputNumber = nNumber;
}

void InequalityLoader::setCase(string sCaseName){
    if (!sCaseName.empty())
        ui->label_CaseOut->setText(QString::fromStdString(sCaseName));
}

void InequalityLoader::loadCase(string filename){
    if (filename.empty())
        return;

    string token;
    ifstream ss(filename.c_str());

    if(ss.is_open()){
        bool flag_problem = false;
        sDetails = "";
        int nCount;
        QVector<double> qvX;
        QVector<double> qvY;

        string filename_short;
        stringstream ss_name;
        ss_name << filename;
        while (getline(ss_name, filename_short, '/')){ cout << filename_short << endl; }
        ss_name << filename_short;
        cout << "a: " << filename_short << endl;
        ui->label_CaseOut->setText(QString::fromStdString("<b><i>" + filename_short +"<i\\><b\\>"));

        while ( getline(ss, token, '"')){
            // inequalities
            if (token == "expression") {			// beginning of inequality
                sDetails += "Inequality:\n";
                nCount = 0;
                if (getline(ss, token, '{'))
                    if (getline(ss, token, '}')){
                       string token_exp;
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
                                       default:
                                           flag_problem = true;
                                           break;
                                        }
                                    }
                                }
                            }
                        }
                        if (nCount != 4)
                            flag_problem = true;
                    }
            }

            // variables
            if (token == "variables") {			// beginning of inequality
                sDetails += "Variable:";
                nCount = 0;
                if (getline (ss, token, '['))
                    if (getline (ss, token, ']')){
                        string token_var;
                        stringstream iss;
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
                                        if (nCount != 5)
                                            flag_problem = true;
                                    }
                            if (token_var == "slider point" )
                                if (getline (iss, token_var, ':'))
                                    if (getline (iss, token_var, ',')){
                                        sDetails += "Step chosen as constant: " + token_var;
                                    }
                        }
                    }
            }

            // plot data
            if (token == "plot"){				// beginning of plot data
                if (getline(ss, token,'['))
                    if (getline(ss, token,']')){
                       string token_plot;
                        stringstream iss;
                       iss << token;
                       while (getline(iss, token_plot, '"')){
                            if (token_plot == "x")
                                if (getline (iss, token_plot, ':'))
                                    if (getline (iss, token_plot, ','))
                                        qvX.push_back(atof(token.c_str()));

                            if (token_plot == "y")
                                if (getline (iss, token_plot, ':'))
                                    if (getline (iss, token_plot,'}'))
                                        qvY.push_back(atof(token.c_str()));

                        }
                        if (!qvX.empty()){
                            _XResults.push_back(qvX);
                            _YResults.push_back(qvY);
                        }
                    }
            }	// plot data
        }	// end of file

        if (_XResults.empty() || _YResults.empty() || (_YResults.size() != _XResults.size()))
            flag_problem = true;

        //	if input was invalid, kill the widget
        if (flag_problem)
            emit killThis(nInequalityInputNumber);
        return; // unsure if this is needed
    }	// open file

}


//	Getters
int InequalityLoader::getNumber(){ return nInequalityInputNumber; }

int InequalityLoader::getColorIndex(){ return ui->comboBoxColor->currentIndex(); }

int InequalityLoader::getShapeIndex(){ return ui->comboBoxShape->currentIndex(); }

int InequalityLoader::getCombination(){ return ui->comboBoxInteract->currentIndex(); }

bool InequalityLoader::getSkip(){ return flag_skip; }

string InequalityLoader::getType(){ return sType; }

QVector<double> InequalityLoader::getCurrentX(){
    return _XResults[nCurrentPlot];
}

QVector<double> InequalityLoader::getCurrentY(){
   return _YResults[nCurrentPlot];
}

void InequalityLoader::beginPlot(){ nCurrentPlot = 0; }

bool InequalityLoader::nextPlot(){
    if (isEnd())
        return false;
    nCurrentPlot++;
    return true;
}


///	Private Functions
bool InequalityLoader::isEnd(){
    if (_XResults.empty())
        return true;
    return nCurrentPlot == (_XResults.size()-1);
}

//	Private Slots
void InequalityLoader::on_pushButton_Details_clicked()
{
    QMessageBox *dialog = new QMessageBox(0);
    dialog->setText(QString::fromStdString(sDetails));
    dialog->show();
}

void InequalityLoader::on_pushButton_Remove_clicked() { emit killThis(nInequalityInputNumber); }

void InequalityLoader::on_pushButtonUp_clicked() { emit moveUp(nInequalityInputNumber); }

void InequalityLoader::on_pushButtonDown_clicked() { emit moveDown(nInequalityInputNumber); }

void InequalityLoader::on_comboBoxInteract_currentIndexChanged(int index)
{
   switch (index) {
    case COMBINE_NONE:
       ui->comboBoxColor->setEnabled(true);
       ui->comboBoxShape->setEnabled(true);
       break;
   default:
       ui->comboBoxColor->setEnabled(false);
       ui->comboBoxShape->setEnabled(false);
       break;
   }
}

void InequalityLoader::on_checkBoxSkip_toggled(bool checked)
{
   if(checked) {
       flag_skip = true;
       ui->comboBoxColor->setEnabled(false);
       ui->comboBoxShape->setEnabled(false);
       ui->comboBoxInteract->setEnabled(false);
   }
   else {
       flag_skip = false;
       ui->comboBoxColor->setEnabled(true);
       ui->comboBoxShape->setEnabled(true);
       if(ui->pushButtonDown->isEnabled())
           ui->comboBoxInteract->setEnabled(true);
   }
}
