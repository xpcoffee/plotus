#ifndef INEQUALITYLOADER_H
#define INEQUALITYLOADER_H

//	Include
#include <QWidget>
#include <QtWidgets>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "inequality_input/inequalityinput.h"

//	Namespace
using namespace std;

namespace Ui {
class InequalityLoader;
}

//	Class
class InequalityLoader : public QWidget
{
    Q_OBJECT

public:
    explicit InequalityLoader(QWidget *parent = 0);
    ~InequalityLoader();
    //	setters
    void setNumber(int);
    void setCase(string);
    void loadCase(string);
    // getters
    int getNumber();
    int getColorIndex();
    int getShapeIndex();
    int getCombination();
    bool getSkip();
    QVector<double> getX();
    QVector<double> getY();
    // parsers
    string toJSON();
    // evaluation
    void beginPlot();
    void nextPlot();
    bool isEnd();
    // gui
    void enablePositionButtons(bool);
    void enableCombinations(bool);
    void resetCombinations();

signals:
    void moveUp(int nNumber);
    void moveDown (int nNumber);
    void killThis (int nNumber);

private slots:
    void on_pushButton_Details_clicked();
    void on_pushButton_Remove_clicked();
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_checkBoxSkip_toggled(bool checked);
    void on_comboBoxInteract_currentIndexChanged(int index);

private:
    Ui::InequalityLoader *ui;
    // data
    vector<QVector<double> > _XResults;
    vector<QVector<double> > _YResults;
    string sDetails;
    // meta
    int nInequalityInputNumber;
    bool flag_skip;
    string sFileName;
    // evaluation
    unsigned int nCurrentPlot;

    // functions

};

#endif // INEQUALITYLOADER_H
