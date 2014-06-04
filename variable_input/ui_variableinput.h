/********************************************************************************
** Form generated from reading UI file 'variableinput.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VARIABLEINPUT_H
#define UI_VARIABLEINPUT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VariableInput
{
public:
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEditName;
    QLineEdit *lineEditMin;
    QLineEdit *lineEditMax;
    QLineEdit *lineEditElements;
    QLineEdit *lineEditUnits;
    QComboBox *comboBoxAxes;
    QSlider *horizontalSliderPoint;
    QLabel *labelPoint;

    void setupUi(QWidget *VariableInput)
    {
        if (VariableInput->objectName().isEmpty())
            VariableInput->setObjectName(QStringLiteral("VariableInput"));
        VariableInput->resize(705, 47);
        horizontalLayout = new QHBoxLayout(VariableInput);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lineEditName = new QLineEdit(VariableInput);
        lineEditName->setObjectName(QStringLiteral("lineEditName"));

        horizontalLayout->addWidget(lineEditName);

        lineEditMin = new QLineEdit(VariableInput);
        lineEditMin->setObjectName(QStringLiteral("lineEditMin"));

        horizontalLayout->addWidget(lineEditMin);

        lineEditMax = new QLineEdit(VariableInput);
        lineEditMax->setObjectName(QStringLiteral("lineEditMax"));

        horizontalLayout->addWidget(lineEditMax);

        lineEditElements = new QLineEdit(VariableInput);
        lineEditElements->setObjectName(QStringLiteral("lineEditElements"));

        horizontalLayout->addWidget(lineEditElements);

        lineEditUnits = new QLineEdit(VariableInput);
        lineEditUnits->setObjectName(QStringLiteral("lineEditUnits"));

        horizontalLayout->addWidget(lineEditUnits);

        comboBoxAxes = new QComboBox(VariableInput);
        comboBoxAxes->setObjectName(QStringLiteral("comboBoxAxes"));
        comboBoxAxes->setMinimumSize(QSize(70, 0));

        horizontalLayout->addWidget(comboBoxAxes);

        horizontalSliderPoint = new QSlider(VariableInput);
        horizontalSliderPoint->setObjectName(QStringLiteral("horizontalSliderPoint"));
        horizontalSliderPoint->setEnabled(false);
        horizontalSliderPoint->setMinimumSize(QSize(200, 0));
        horizontalSliderPoint->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(horizontalSliderPoint);

        labelPoint = new QLabel(VariableInput);
        labelPoint->setObjectName(QStringLiteral("labelPoint"));
        labelPoint->setMinimumSize(QSize(80, 0));
        labelPoint->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(labelPoint);


        retranslateUi(VariableInput);

        QMetaObject::connectSlotsByName(VariableInput);
    } // setupUi

    void retranslateUi(QWidget *VariableInput)
    {
        VariableInput->setWindowTitle(QApplication::translate("VariableInput", "VariableInput", 0));
        comboBoxAxes->clear();
        comboBoxAxes->insertItems(0, QStringList()
         << QApplication::translate("VariableInput", "x-axis", 0)
         << QApplication::translate("VariableInput", "y-axis", 0)
         << QApplication::translate("VariableInput", "point", 0)
        );
        labelPoint->setText(QApplication::translate("VariableInput", "-", 0));
    } // retranslateUi

};

namespace Ui {
    class VariableInput: public Ui_VariableInput {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VARIABLEINPUT_H
