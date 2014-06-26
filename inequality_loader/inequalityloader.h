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

// 	Enumerators
enum COMBINE
{
    COMBINE_NONE 			= 0,
    COMBINE_INTERSECTION 	= 1,
    COMBINE_UNION 			= 2,
    COMBINE_SUBTRACTION 	= 3,

};

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
    string getType();
    QVector<double> getCurrentX();
    QVector<double> getCurrentY();
    // evaluation
    void beginPlot();
    bool nextPlot();

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
    string sType;
    // evaluation
    unsigned int nCurrentPlot;

    // functions
    bool isEnd();

};

#endif // INEQUALITYLOADER_H
