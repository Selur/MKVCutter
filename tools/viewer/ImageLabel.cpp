/*
 * ImageLabel.cpp
 *
 *  Created on: Feb 19, 2012
 *      Author: Selur
 */

#include "ImageLabel.h"
#include <QPainter>

ImageLabel::ImageLabel(QWidget * parent, Qt::WindowFlags f) :
    QLabel(parent, f)
{
}

ImageLabel::~ImageLabel()
{

}

void ImageLabel::paintEvent(QPaintEvent *aEvent)
{
  QLabel::paintEvent(aEvent);
  this->displayImage();
}

void ImageLabel::setPixmap(QPixmap aPicture)
{
  m_source = m_current = aPicture;
  repaint();
}

void ImageLabel::displayImage()
{
  if (m_source.isNull()) { //no image was set, don't draw anything
    return;
  }
  float cw = width(), ch = height();
  float pw = m_current.width(), ph = m_current.height();

  if ((pw > cw && ph > ch && pw / cw > ph / ch) || //both width and high are bigger, ratio at high is bigger or
      (pw > cw && ph <= ch) || //only the width is bigger or
      (pw < cw && ph < ch && cw / pw < ch / ph) //both width and height is smaller, ratio at width is smaller
      ) {
    m_current = m_source.scaledToWidth(cw);
  } else if ((pw > cw && ph > ch && pw / cw <= ph / ch) || //both width and high are bigger, ratio at width is bigger or
      (ph > ch && pw <= cw) || //only the height is bigger or
      (pw < cw && ph < ch && cw / pw > ch / ph) //both width and height is smaller, ratio at height is smaller
      ) {
    m_current = m_source.scaledToHeight(ch);
  }
  int x = (cw - m_current.width()) / 2, y = (ch - m_current.height()) / 2;
  QPainter paint(this);
  paint.drawPixmap(x, y, m_current);
}
