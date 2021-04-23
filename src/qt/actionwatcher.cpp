
#include "actionwatcher.h"
#include "svgiconengine.h"
#include<QToolButton>
#include<QMouseEvent>
#include<QMetaEnum>
#include<QDebug>
#include<iostream>



using namespace::std;
/*
QDebug operator<<(QDebug str, const QEvent * ev) {
   static int eventEnumIndex = QEvent::staticMetaObject
         .indexOfEnumerator(Type);
   str << QEvent;
   if (ev) {
      QString name = QEvent::staticMetaObject
            .enumerator(eventEnumIndex).valueToKey(ev->type());
   } else {
      str << (void*)ev;
   }
   return str.maybeSpace();
}
*/
ActionWatcher::ActionWatcher(QObject * parent) :
    QObject(parent)
{}

bool ActionWatcher::eventFilter(QObject * watched, QEvent * event)
{
	//cout << "event handler" << endl;
    //QToolButton * toolbutton = qobject_cast<QToolButton*>(watched);
    QAction * action = qobject_cast<QAction*>(watched);


    //cout << "action" << endl;
    //if(event->type() == QEvent::ActionChanged)
//	    cout << "action changed" << endl;

    //cout << action->isChecked() << endl;

    string iconSvgDataHover = 
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
    string iconSvgDataLeave = 
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
    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"  <g>"
"    <polygon points=\"2,9.6 2,24 9,24 9,17 15,17 15,24 22,24 22,9.6 12,0.7 \"></polygon>"
"  </g>"
"</svg>"; 
    if(action->isChecked())
    {
	    for(auto widget : action->associatedWidgets())
	    {
      QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
    QToolButton * button = qobject_cast<QToolButton*>(widget);
      button->setIcon(icon);
	    }
      return true;
    }
    /*
    if (event->type() == QEvent::Enter) {
	  cout << "hover event" << endl;
    //cout << iconSvgData << endl;
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataHover));
       // The push button is hovered by mouse
        //button->setIcon(QIcon(:/images/start_hov.png));
        toolbutton->setIcon(icon);
        return true;
    }


    if (event->type() == QEvent::Leave){
        // The push button is not hovered by mouse
        //button->setIcon(QIcon(:/images/start.png));
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataLeave));
        toolbutton->setIcon(icon);
        return true;
    }
    if (event->type() == QEvent::MouseButtonPress){
	    cout << "button press" << endl;
	    QMouseEvent* mev = static_cast<QMouseEvent*>(event);
	    if(mev->button() == Qt::LeftButton)
	    {
	    cout << "left" << endl;
        // The push button is not hovered by mouse
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        toolbutton->setIcon(icon);
	return true;
	    }
    }
 */
    return false;
}  
