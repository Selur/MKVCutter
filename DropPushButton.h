#ifndef DROPPUSHBUTTON_H
#define DROPPUSHBUTTON_H

#include <QPushButton>
#include <QObject>
class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;

class DropPushButton : public QPushButton
{
  Q_OBJECT
  public:
    explicit DropPushButton(QWidget *parent = nullptr);

  protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

  private:
    bool m_drops;

    public slots:
    void acceptDrops(bool value);

    signals:
    void droppedInput(QString input);

};

#endif // DROPPUSHBUTTON_H
