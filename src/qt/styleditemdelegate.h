#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class StyledItemDelegate : public QStyledItemDelegate
{
public:
    StyledItemDelegate();

    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // STYLEDITEMDELEGATE_H
