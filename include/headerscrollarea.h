#ifndef HEADERSCROLLAREA_H
#define HEADERSCROLLAREA_H

#include <QScrollArea>
#include <QDebug>
#include <QScrollBar>
#include <QLayout>

class HeaderScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit HeaderScrollArea(QWidget *parent = 0, int header_height = 30);

    //	reimplemented
    void resizeEvent(QResizeEvent* e);
    void scrollContentsBy(int dx, int dy);

    // new
    QWidget* getHeaderWidget();
    void setHeaderHeight(int value);

signals:

private:
    int m_topMargin;
    QWidget *m_headerWidget;
};


#endif // HEADERSCROLLAREA_H
