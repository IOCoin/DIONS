#ifndef UNLOCKEDSTATUSLABEL_H
#define UNLOCKEDSTATUSLABEL_H

#include "clickablelabel.h"
#include<QLabel>
#include<QWidget>
#include<Qt>

class UnlockedStatusLabel : public ClickableLabel 
{
  Q_OBJECT
  public:
    explicit UnlockedStatusLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~UnlockedStatusLabel();

  signals:
    void lock(bool);

  protected:
    void mousePressEvent(QMouseEvent* e);
    
};

#endif
