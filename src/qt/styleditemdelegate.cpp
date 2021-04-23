#include "styleditemdelegate.h"
#include "tableview.h"
#include <QDebug>
StyledItemDelegate::StyledItemDelegate()
    : QStyledItemDelegate ()
{

}


void StyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    opt.state &= ~QStyle::State_HasFocus;

    TableView *view = qobject_cast<TableView *>(opt.styleObject);
    QModelIndex hoverIndex = view->hoverIndex();

    if (hoverIndex.row() == index.row())
      opt.state |= QStyle::State_MouseOver;
    QStyledItemDelegate::paint(painter, opt, index);
}
