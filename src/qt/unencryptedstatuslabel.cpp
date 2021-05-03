#include "unencryptedstatuslabel.h"

UnencryptedStatusLabel::UnencryptedStatusLabel(QWidget* parent, Qt::WindowFlags f)
  : ClickableLabel()
{
	
}

UnencryptedStatusLabel::~UnencryptedStatusLabel() { }

void UnencryptedStatusLabel::mousePressEvent(QMouseEvent* e)
{
  emit encrypt(true);
}
