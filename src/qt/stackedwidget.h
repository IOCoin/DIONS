#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>

class StackedWidget : public QStackedWidget
{
  public:
  StackedWidget(QWidget* o) : QStackedWidget(o) { }

  protected:
  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;
};

#endif
