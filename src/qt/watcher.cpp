
#include "watcher.h"
#include "SvgIconEngine.h"
#include<QToolButton>
#include<QMouseEvent>
#include<QMetaEnum>
#include<QDebug>
#include<iostream>

using namespace::std;
Watcher::Watcher(QObject * parent) :
    QObject(parent)
{}

bool Watcher::eventFilter(QObject * watched, QEvent * event)
{
    return false;
}  
