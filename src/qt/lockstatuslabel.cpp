#include "lockstatuslabel.h"

LockStatusLabel::LockStatusLabel(QWidget* parent, Qt::WindowFlags f)
  : ClickableLabel()
{
	
}

LockStatusLabel::~LockStatusLabel() { }

void LockStatusLabel::mousePressEvent(QMouseEvent* e)
{
  emit unlock(true);
}
