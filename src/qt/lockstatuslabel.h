#ifndef LOCKSTATUSLABEL_H
#define LOCKSTATUSLABEL_H

#include "clickablelabel.h"
#include<QLabel>
#include<QWidget>
#include<Qt>

class LockStatusLabel : public ClickableLabel 
{
  Q_OBJECT
  public:
    explicit LockStatusLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~LockStatusLabel();

  signals:
    void unlock(bool);

  protected:
    void mousePressEvent(QMouseEvent* e);
    
};

#endif
