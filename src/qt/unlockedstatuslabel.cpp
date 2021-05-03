#include "unlockedstatuslabel.h"

UnlockedStatusLabel::UnlockedStatusLabel(QWidget* parent, Qt::WindowFlags f)
  : ClickableLabel()
{
	
}

UnlockedStatusLabel::~UnlockedStatusLabel() { }

void UnlockedStatusLabel::mousePressEvent(QMouseEvent* e)
{
  emit lock(true);
}
