/*
 * MarkSlider.h
 *
 *  Created on: Feb 20, 2012
 *      Author: Selur
 */

#ifndef MARKSLIDER_H_
#define MARKSLIDER_H_

#include <QSlider>
class QPaintEvent;

class MarkSlider : public QSlider
{
  public:
    MarkSlider(QWidget *parent = 0);
    virtual ~MarkSlider();
    void resetMarks();
    int getEnd() const;
    int getStart() const;
    void setEnd(int end);
    void setStart(int start);

  private:
    int m_start;
    int m_end;

  protected:
    void paintEvent(QPaintEvent *event);
};

#endif /* MARKSLIDER_H_ */
