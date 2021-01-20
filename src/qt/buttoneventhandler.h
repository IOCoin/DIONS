#ifndef BUTTONEVENTHANDLER_H
#define BUTTONEVENTHANDLER_H
#include<QObject>
#include<QAction>
#include<QEvent>
#include<QToolButton>

#include "iocoingui.h"
class ButtonHoverWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ButtonHoverWatcher(QObject * parent = Q_NULLPTR, QToolButton* b=Q_NULLPTR);
    virtual bool eventFilter(QObject * watched, QEvent * event) Q_DECL_OVERRIDE;
private:
    QToolButton* toolbutton_;
};

#endif
