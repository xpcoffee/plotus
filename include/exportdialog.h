#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QIntValidator>

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExportDialog(QWidget *parent = 0);

signals:
    void exportOptions(int width, int heigh, int dpi);

private slots:
    void sendOptions();

private:
    QFormLayout *m_layout;
    QLabel *m_labelTitle;
    QLabel *m_labelHeight;
    QLabel *m_labelWidth;
    QLabel *m_labelDPI;
    QLabel *m_labelMessage;
    QLineEdit *m_lineEditHeight;
    QLineEdit *m_lineEditWidth;
    QLineEdit *m_lineEditDPI;
    QPushButton *m_buttonOK;
    QPushButton *m_buttonCancel;

};

#endif // EXPORTDIALOG_H
