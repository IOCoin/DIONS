#ifndef LOCKEDSTATUSLABEL_H
#define LOCKEDSTATUSLABEL_H

#include "clickablelabel.h"
#include<QLabel>
#include<QWidget>
#include<Qt>

class LockedStatusLabel : public ClickableLabel 
{
  Q_OBJECT
  public:
    explicit LockedStatusLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~LockedStatusLabel();

  signals:
    void unlock(bool);
    void unenc(bool);

  protected:
    void mousePressEvent(QMouseEvent* e);
    
};

#endif
