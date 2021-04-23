#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
  : QLabel(parent)
{
	
}

ClickableLabel::~ClickableLabel() { }

void ClickableLabel::mousePressEvent(QMouseEvent* e)
{
  emit clicked(true);
}
