#include "include/exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent)
{
    m_layout = new QFormLayout(this);

    m_labelTitle = new QLabel("Exporting Plot", this);
    m_labelHeight = new QLabel("Height", this);
    m_labelWidth = new QLabel("Width", this);
    m_labelDPI = new QLabel("Resolution (DPI)", this);
    m_labelMessage = new QLabel("", this);

    m_lineEditWidth = new QLineEdit(this);
    m_lineEditHeight = new QLineEdit(this);
    m_lineEditDPI = new QLineEdit(this);
    QIntValidator *int_val = new QIntValidator;
    m_lineEditWidth->setValidator(int_val);
    m_lineEditHeight->setValidator(int_val);
    m_lineEditDPI->setValidator(int_val);

    m_buttonOK = new QPushButton("OK", this);
    m_buttonCancel = new QPushButton("Cancel", this);
    QWidget::connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(close()));
    QWidget::connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(sendOptions()));

    m_layout->addWidget(m_labelTitle);
    m_layout->addRow(m_labelWidth, m_lineEditWidth);
    m_layout->addRow(m_labelHeight, m_lineEditHeight);
    m_layout->addRow(m_labelDPI, m_lineEditDPI);
    m_layout->addWidget(m_labelMessage);
    m_layout->addWidget(m_buttonOK);
    m_layout->addWidget(m_buttonCancel);

    this->setWindowTitle("Plotus :: Export Plot");
    this->setLayout(m_layout);

}

void ExportDialog::sendOptions()
{
    if (m_lineEditWidth->text().isEmpty() || m_lineEditHeight->text().isEmpty() || m_lineEditDPI->text().isEmpty()) {
        m_labelMessage->setText("Please fill in all fields.");
        m_labelMessage->setStyleSheet("color: red; font: bold;");
        return;
    }
    m_labelMessage->setText("");

    emit exportOptions(m_lineEditWidth->text().toInt(), m_lineEditHeight->text().toInt(), m_lineEditDPI->text().toInt());
    this->close();
}
