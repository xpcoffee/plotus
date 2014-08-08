#include "include/headerscrollarea.h"

HeaderScrollArea::HeaderScrollArea(QWidget *parent, int header_height ) :
        QScrollArea(parent)
    {
        setHeaderHeight(header_height);
        header_widget = new QWidget(this);
    }

void HeaderScrollArea::resizeEvent(QResizeEvent* e)
{
    QRect rect = this->viewport()->geometry();
    header_widget->setGeometry(rect.x(), rect.y() - top_margin, rect.width() - 1, top_margin );
    QScrollArea::resizeEvent(e);
}

void HeaderScrollArea::setHeaderHeight(int value)
{
    setViewportMargins(0, value, 0, 0);
    top_margin = value;
}



QWidget* HeaderScrollArea::getHeaderWidget() { return header_widget; }
