#ifndef ROTATINGLABEL_H
#define ROTATINGLABEL_H

#include "svgiconengine.h"
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QVariantAnimation>

class RotatingLabel : public QLabel {

    Q_OBJECT
public:
    explicit RotatingLabel(QWidget* parent = Q_NULLPTR) :
        QLabel(parent),
        pixmap(100, 100),
        animation(new QVariantAnimation )
    {
std::string logoSVG_ = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" width=\"119.185px\" height=\"116.22px\" viewBox=\"14.148 6.833 119.185 116.22\" enable-background=\"new 14.148 6.833 119.185 116.22\" style=\"fill:white\">"
"  <g>"
"                <path d=\"M133.333,67.107c0,41.16-41.473,52.983-41.473,52.983V99.03c0,0,21.06-7.611,21.873-31.923,c0,0,1.293-24.458-21.873-36.281V10.079C91.86,10.079,133.333,20.129,133.333,67.107z\"/>"
"                <path d=\"M14.148,63.062c0-41.16,41.494-52.983,41.494-52.983v21.06c0,0-21.06,7.611-21.873,31.923,c0,0-1.293,24.458,21.873,36.281v20.747C55.642,120.09,14.148,110.04,14.148,63.062z\"/>"
"                <path d=\"M87.148,46.214v74.689c-12.406,4.837-27.065,0-27.065,0V46.214C60.104,46.214,72.907,40.71,87.148,46.214z\""
"                    />"
"                <path d=\"M87.148,9.308v20.372c-12.469-6.235-27.065,0-27.065,0V9.308C60.104,9.308,73.866,3.74,87.148,9.308z\"/>"
"  </g>"
"</svg>";
        QIcon* tmp = new QIcon(new SVGIconEngine(logoSVG_));
        QPixmap logoPixmap = tmp->pixmap(tmp->actualSize(QSize(200,200)));

        this->setPixmap(logoPixmap);

        resize(200, 200);
        //pixmap.fill(Qt::white);

        animation->setDuration(10000);
        animation->setStartValue(0.0f);
        animation->setEndValue(90.0f);
        connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant &value){
            qDebug()<<value;
            QTransform t;
            t.rotate(value.toReal());
            setPixmap(pixmap.transformed(t));
        });
        animation->start(); 
    }
private:
    QPixmap             pixmap;
    QVariantAnimation  *animation;
};

#endif
