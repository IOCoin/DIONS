#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView
{
    Q_OBJECT
public:
    TableView(QWidget *parent = nullptr);

    QModelIndex hoverIndex() const { return model()->index(mHoverRow, mHoverColumn); }

    // QWidget interface
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //virtual bool viewportEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    int mHoverRow, mHoverColumn;
};

#endif // TABLEVIEW_H
