#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include<QLabel>
#include<QWidget>
#include<Qt>

class ClickableLabel : public QLabel 
{
  Q_OBJECT
  public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel();

  signals:
    void clicked(bool);

  protected:
    void mousePressEvent(QMouseEvent* e);
    
};

#endif
