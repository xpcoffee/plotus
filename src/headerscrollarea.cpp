#include "include/headerscrollarea.h"

HeaderScrollArea::HeaderScrollArea(QWidget *parent, int header_height ) :
        QScrollArea(parent)
    {
        setHeaderHeight(header_height);

        QVBoxLayout *layout_main = new QVBoxLayout(this);
        layout_main->setAlignment(Qt::AlignTop);
        layout_main->setContentsMargins(0,0,0,0);
        layout_main->setSpacing(5);

        QHBoxLayout *layout_head = new QHBoxLayout();
        layout_head->setObjectName("header_layout");
        layout_head->setContentsMargins(0,0,0,0);
        layout_head->setSpacing(5);

        m_headerWidget = new QWidget(this);
        m_headerWidget->setObjectName("header_widget");
        m_headerWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

        layout_head->addWidget(m_headerWidget);
        layout_main->addLayout(layout_head);
        layout_main->addWidget(viewport());
    }

void HeaderScrollArea::resizeEvent(QResizeEvent* e)
{
    QRect rect = viewport()->geometry();
    QRect pos = viewport()->geometry();

    QLayout *inner_layout = static_cast<QLayout*>(this->children().at(0)->children().at(0)->children().at(0)->children().at(0));

    if (inner_layout->count() > 0){
        // inner layout > object
        rect = inner_layout->itemAt(0)->widget()->geometry();
    }

    m_headerWidget->setGeometry(rect.x() - 6 , pos.y() - m_topMargin, rect.width() - 6, m_topMargin);
    int scroll = -1 * (horizontalScrollBar()->value() / 2);
    m_headerWidget->scroll(scroll, 0);

    QScrollArea::resizeEvent(e);
}

void HeaderScrollArea::scrollContentsBy(int dx, int dy)
{

    m_headerWidget->scroll(dx,0);
    QScrollArea::scrollContentsBy(dx, dy);
}


void HeaderScrollArea::setHeaderHeight(int value)
{
    setViewportMargins(0, value, 0, 0);
    m_topMargin = value;
}



QWidget* HeaderScrollArea::getHeaderWidget() { return m_headerWidget; }
