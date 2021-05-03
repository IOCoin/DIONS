#ifndef UNENCRYPTEDSTATUSLABEL_H
#define UNENCRYPTEDSTATUSLABEL_H

#include "clickablelabel.h"
#include<QLabel>
#include<QWidget>
#include<Qt>

class UnencryptedStatusLabel : public ClickableLabel 
{
  Q_OBJECT
  public:
    explicit UnencryptedStatusLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~UnencryptedStatusLabel();

  signals:
    void encrypt(bool);

  protected:
    void mousePressEvent(QMouseEvent* e);
    
};

#endif
