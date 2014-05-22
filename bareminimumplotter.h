#ifndef BAREMINIMUMPLOTTER_H
#define BAREMINIMUMPLOTTER_H

#include <QMainWindow>
#include <vector>
#include <stdlib.h>
#include "deplot/include/variable.h"
#include "deplot/include/inequality.h"
#include "deplot/include/expression.h"

namespace Ui {
class BareMinimumPlotter;
}

class BareMinimumPlotter : public QMainWindow
{
    Q_OBJECT

public:
    explicit BareMinimumPlotter(QWidget *parent = 0);
    ~BareMinimumPlotter();

    void plot();
    bool isEmpty_InputFields();

private slots:
    void on_buttonPlot_clicked();

private:
    Ui::BareMinimumPlotter *ui;
    Variable mVariable1, mVariable2;
    Expression mExpression1, mExpression2;
    Inequality mInequality;

};

#endif // BAREMINIMUMPLOTTER_H
