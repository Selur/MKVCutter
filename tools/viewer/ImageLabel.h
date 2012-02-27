/*
 * ImageLabel.h
 *
 *  Created on: Feb 19, 2012
 *      Author: Selur
 */

#ifndef IMAGELABEL_H_
#define IMAGELABEL_H_
#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>

class ImageLabel : public QLabel
{
  public:
    ImageLabel(QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~ImageLabel();
    void paintEvent(QPaintEvent *aEvent);
    void setPixmap(QPixmap aPicture);

  private:
    QPixmap m_source, m_current;
    void displayImage();



};

#endif /* IMAGELABEL_H_ */
