#include "tableview.h"
#include <QMouseEvent>
#include <iostream>
#include "styleditemdelegate.h"

TableView::TableView(QWidget *parent)
    : QTableView (parent)
    , mHoverRow(-1)
    , mHoverColumn(-1)
{
    setMouseTracking(true);
    setSelectionBehavior(SelectRows);

    StyledItemDelegate* sid = new StyledItemDelegate();
    setItemDelegateForColumn(0,sid);
    setItemDelegateForColumn(1,sid);
    setItemDelegateForColumn(2,sid);
    setItemDelegateForColumn(3,sid);
    setItemDelegateForColumn(4,sid);
}

void TableView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    mHoverRow = index.row();
    mHoverColumn = index.column();
}
