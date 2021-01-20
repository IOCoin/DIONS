#ifndef ACTIONWATCHER_H
#define ACTIONWATCHER_H
#include<QObject>
#include<QAction>
#include<QEvent>
#include<QToolButton>

#include "iocoingui.h"

class ActionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ActionWatcher(QObject * parent = Q_NULLPTR );
    virtual bool eventFilter(QObject * watched, QEvent * event) Q_DECL_OVERRIDE;

};

#endif
