#include "DropPushButton.h"
#include <QUrl>
#include <QDir>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>

DropPushButton::DropPushButton(QWidget *parent) :
    QPushButton(parent), m_drops(false)
{
}
void DropPushButton::dropEvent(QDropEvent *event)
{
  if (!m_drops) {
    return;
  }
  QList<QUrl> urls = event->mimeData()->urls();
  QString location;
  if (urls.count() > 0) {
    location += QDir::toNativeSeparators(urls.at(0).toLocalFile());
  }

  if (location.isEmpty()) {
    event->ignore();
    return;
  }
  this->setText(location);
  emit droppedInput(location);
  event->accept();
}

void DropPushButton::dragEnterEvent(QDragEnterEvent *event)
{
  if (!m_drops) {
    return;
  }
  event->acceptProposedAction();
}

void DropPushButton::acceptDrops(bool value)
{
  m_drops = value;
  this->setAcceptDrops(value);
}
