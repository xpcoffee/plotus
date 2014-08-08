#ifndef HEADERSCROLLAREA_H
#define HEADERSCROLLAREA_H

#include <QScrollArea>

class HeaderScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit HeaderScrollArea(QWidget *parent = 0, int header_height = 30);

    void resizeEvent(QResizeEvent* e);

    QWidget* getHeaderWidget();

    void setHeaderHeight(int value);

private:
    int top_margin;
    QWidget *header_widget;
};


#endif // HEADERSCROLLAREA_H
