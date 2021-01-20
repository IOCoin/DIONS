
#include "buttoneventhandler.h"
#include "SvgIconEngine.h"
#include<QToolButton>
#include<QMouseEvent>
#include<QMetaEnum>
#include<QDebug>
#include<iostream>



using namespace::std;
    QString OVERVIEW_BUTTON_TEXT    = "Overview";
    QString SEND_BUTTON_TEXT        = "Send coins";
    QString RECEIVE_BUTTON_TEXT     = "Receive coins";
    QString HISTORY_BUTTON_TEXT     = "Transactions";
    QString ADDRESSBOOK_BUTTON_TEXT = "Address Book";
    QString DIONS_BUTTON_TEXT       = "Dions";

    string OVERVIEW_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"  <g>"
"    <polygon points=\"2,9.6 2,24 9,24 9,17 15,17 15,24 22,24 22,9.6 12,0.7 \"></polygon>"
"  </g>"
"</svg>";
    string OVERVIEW_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"    <polygon points=\"2,9.6 2,24 9,24 9,17 15,17 15,24 22,24 22,9.6 12,0.7 \"></polygon>"
"  </g>"
"</svg>";

    string SEND_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"  <g>"
"                <path d=\"M1.4,11l2.2,1.6l6.4-3c0.5-0.2,1,0.4,0.5,0.8l-4.5,4v6.7c0,0.9,1.2,1.4,1.8,0.6l3.1-3.7l6.5,4.9 c0.6,0.4,1.4,0.1,1.6-0.6l4-20c0.2-0.8-0.6-1.4-1.4-1.1l-20,8C0.9,9.6,0.8,10.6,1.4,11z\"></path>"
"  </g>"
"</svg>";
    string SEND_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path d=\"M1.4,11l2.2,1.6l6.4-3c0.5-0.2,1,0.4,0.5,0.8l-4.5,4v6.7c0,0.9,1.2,1.4,1.8,0.6l3.1-3.7l6.5,4.9 c0.6,0.4,1.4,0.1,1.6-0.6l4-20c0.2-0.8-0.6-1.4-1.4-1.1l-20,8C0.9,9.6,0.8,10.6,1.4,11z\"></path>"
"  </g>"
"</svg>";
    string RECEIVE_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"            <g transform=\"translate(0, 0)\"> "
"                <line data-cap=\"butt\" stroke-miterlimit=\"10\" x1=\"12\" y1=\"3\" x2=\"12\" y2=\"17\" stroke-linejoin=\"miter\" stroke-linecap=\"butt\"></line> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"5,10 19,10 19,8 5,8 \" stroke-linejoin=\"miter\"></polyline> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"7,12 12,17 17,12 \" stroke-linejoin=\"miter\"></polyline> "
"                <line data-color=\"color-2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"21\" x2=\"5\" y2=\"21\" stroke-linejoin=\"miter\"></line> "
"            </g> "
"</svg>";
    string RECEIVE_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"            <g transform=\"translate(0, 0)\"> "
"                <line data-cap=\"butt\" stroke-miterlimit=\"10\" x1=\"12\" y1=\"3\" x2=\"12\" y2=\"17\" stroke-linejoin=\"miter\" stroke-linecap=\"butt\"></line> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"5,10 19,10 19,8 5,8 \" stroke-linejoin=\"miter\"></polyline> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"7,12 12,17 17,12 \" stroke-linejoin=\"miter\"></polyline> "
"                <line data-color=\"color-2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"21\" x2=\"5\" y2=\"21\" stroke-linejoin=\"miter\"></line> "
"            </g> "
"</svg>";
    string HISTORY_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"  <g>"
"                <path data-color=\"color-2\" d=\"M21,16H8v-4l-8,6l8,6v-4h13c0.6,0,1-0.4,1-1v-2C22,16.4,21.6,16,21,16z\"></path> "
"                <path d=\"M16,12l8-6l-8-6v4H3C2.4,4,2,4.4,2,5v2c0,0.6,0.4,1,1,1h13V12z\"></path> "
"  </g>"
"</svg>";
    string HISTORY_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path data-color=\"color-2\" d=\"M21,16H8v-4l-8,6l8,6v-4h13c0.6,0,1-0.4,1-1v-2C22,16.4,21.6,16,21,16z\"></path> "
"                <path d=\"M16,12l8-6l-8-6v4H3C2.4,4,2,4.4,2,5v2c0,0.6,0.4,1,1,1h13V12z\"></path> "
"  </g>"
"</svg>";
    string ADDRESSBOOK_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"  <g>"
"                <path d=\"M15.6,8L12,0.2L8.4,8H0.2l6,6.2l-2.3,9.3l8.1-4.6l8.1,4.6l-2.3-9.3l6-6.2H15.6z\"></path>"
"  </g>"
"</svg>";
    string ADDRESSBOOK_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path d=\"M15.6,8L12,0.2L8.4,8H0.2l6,6.2l-2.3,9.3l8.1-4.6l8.1,4.6l-2.3-9.3l6-6.2H15.6z\"></path>"
"  </g>"
"</svg>";
    string DIONS_ENTER = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#bebebe \">"
"   <g> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"7.5,0 10,0 9.5,24 7,24\" stroke-linejoin=\"miter\"></polyline>  "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"10.5,0 13,0 12.5,24 10,24\" stroke-linejoin=\"miter\"></polyline>  "
"   </g> "
"   <text x=\"0\" y=\"21\" font-size=\"27\">D</text> "
"</svg>";
    string DIONS_LEAVE = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"   <g> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"7.5,0 10,0 9.5,24 7,24\" stroke-linejoin=\"miter\"></polyline>  "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"10.5,0 13,0 12.5,24 10,24\" stroke-linejoin=\"miter\"></polyline>  "
"   </g> "
"   <text x=\"0\" y=\"21\" font-size=\"27\">D</text> "
"</svg>";

ButtonHoverWatcher::ButtonHoverWatcher(QObject * parent, QToolButton* b) :
    QObject(parent), toolbutton_(b)
{}

bool ButtonHoverWatcher::eventFilter(QObject * watched, QEvent * event)
{
    QAction * action = qobject_cast<QAction*>(watched);
    
    if (!toolbutton_->isChecked() && event->type() == QEvent::Enter) {
	    if(toolbutton_->text() == OVERVIEW_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(OVERVIEW_ENTER));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == SEND_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(SEND_ENTER));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == RECEIVE_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(RECEIVE_ENTER));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == HISTORY_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(HISTORY_ENTER));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == ADDRESSBOOK_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(ADDRESSBOOK_ENTER));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == DIONS_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(DIONS_ENTER));
              toolbutton_->setIcon(icon);
	    }
          return true;
	}

    if (!toolbutton_->isChecked() && event->type() == QEvent::Leave) {
	    if(toolbutton_->text() == OVERVIEW_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(OVERVIEW_LEAVE));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == SEND_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(SEND_LEAVE));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == RECEIVE_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(RECEIVE_LEAVE));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == HISTORY_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(HISTORY_LEAVE));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == ADDRESSBOOK_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(ADDRESSBOOK_LEAVE));
              toolbutton_->setIcon(icon);
	    }
	    else
	    if(toolbutton_->text() == DIONS_BUTTON_TEXT)
	    {
              QIcon icon = QIcon(new SVGIconEngine(DIONS_LEAVE));
              toolbutton_->setIcon(icon);
	    }
        return true;
	}
    
    return false;
}  
