/*
 * IdxSubCutter.cpp
 *
 *  Created on: 28.02.2015
 *      Author: Selur
 */

#include "IdxSubCutter.h"
#include <QApplication>
#include <QDir>

IdxSubCutter::IdxSubCutter(QObject *parent)
    : QObject(parent), m_process(nullptr), m_output(QString())
{
  this->setObjectName("IdxSubCutter");
}

IdxSubCutter::~IdxSubCutter()
{
}

void IdxSubCutter::cut(const QString &input, const QStringList& cutList, const QString& tempFolder,
    const QString& output)
{
  m_output = output;
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
      SLOT(idxCutterFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(idxCutterOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(idxCutterOutput()));

  QString appFolder = QApplication::applicationDirPath();
  QString call;
#ifdef Q_OS_WIN32
  call = "IdxSubCutter.exe";
#else
  call = "IdxSubCutter";
#endif
  call = "\"" + QDir::toNativeSeparators(appFolder + QDir::separator() + call) + "\"";
  call += " \"";
  call += QDir::toNativeSeparators(input);
  call += "\"";
  call += " \"";
  call += cutList.join(",");
  call += "\"";
  call += " \"";
  call += QDir::toNativeSeparators(tempFolder);
  call += "\"";
  emit sendInfos(tr("IdxSubCutter call: %1").arg(call));
  m_process->start(call);
}

void IdxSubCutter::idxCutterOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  out += m_process->readAllStandardError().data();
  emit sendInfos(out);
}

void IdxSubCutter::idxCutterFinished(int exitState, QProcess::ExitStatus status)
{
  if (exitState < 0) {
    emit progress(0);
    emit sendInfos(tr("IdxCutter crashed: %1 - %2").arg(exitState).arg(status));
    emit finished(QString());
    return;
  }
  emit progress(0);
  emit sendInfos("IdxSubCutter finished: " + m_output);
  emit finished(m_output);
}
