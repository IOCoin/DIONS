#include "lockedstatuslabel.h"

LockedStatusLabel::LockedStatusLabel(QWidget* parent, Qt::WindowFlags f)
  : ClickableLabel()
{
	
}

LockedStatusLabel::~LockedStatusLabel() { }

void LockedStatusLabel::mousePressEvent(QMouseEvent* e)
{
  emit unlock(true);
}
