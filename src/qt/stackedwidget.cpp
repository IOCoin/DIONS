#include "stackedwidget.h"

QSize StackedWidget::sizeHint() const 
{
  return currentWidget()->sizeHint();
}
QSize StackedWidget::minimumSizeHint() const 
{
  return currentWidget()->minimumSizeHint();
}
