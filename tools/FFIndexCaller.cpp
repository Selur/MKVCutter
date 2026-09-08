/*
 * FFIndexCaller.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#include "FFIndexCaller.h"
#include "Globals.h"
#include <QDir>
#include <QApplication>

FFIndexCaller::FFIndexCaller(QObject *parent) :
    QObject(parent), m_input(QString()), m_cache(QString()), m_process(NULL)
{

}

FFIndexCaller::~FFIndexCaller()
{

}

void FFIndexCaller::index(QString inputFile, QString cacheFile)
{
  emit sendInfos(tr("FFIndexCaller(%1, %2)").arg(inputFile).arg(cacheFile));
  m_input = Globals::shortFileName(inputFile);
  m_cache = cacheFile;
  if (QFile::exists(m_cache)) {
    emit sendInfos(
        " -> " + tr("Skipping indexing %1 since %2 already exists.").arg(inputFile).arg(cacheFile));
    emit finished(0);
    return;
  }
  QStringList call;
#ifdef Q_OS_WIN32
  QString inputPath = QApplication::applicationDirPath();
  QString path = QDir::toNativeSeparators(inputPath+QDir::separator()+"ffmsindex.exe");
  if (!QFile::exists(path)
      && QFile::exists(QDir::toNativeSeparators(inputPath+QDir::separator()+"ffmsindex64.exe"))) {
    path = QDir::toNativeSeparators(inputPath+QDir::separator()+"ffmsindex64.exe");
  }
  call << "\"" + path + "\"";
#else
  call << "ffmsindex";
#endif
// call << "\"" + QDir::toNativeSeparators(Globals::shortFileName(inputFile)) + "\"";
  call << "\"" + QDir::toNativeSeparators(inputFile) + "\"";
  call << "\"" + QDir::toNativeSeparators(cacheFile) + "\"";
  QString tmp = call.join(" ");
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(indexerFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(handleIndexerOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleIndexerOutput()));
  emit
  enableGui(false);
  emit
  sendInfos(tr("FFIndex call: %1").arg(tmp));
  m_process->startCommand(tmp);
}

void FFIndexCaller::handleIndexerOutput()
{
  QString out = m_process->readAllStandardError().data();
  out = out.trimmed();
  if (!out.isEmpty()) {
    emit sendInfos(out);
  }
  out = m_process->readAllStandardOutput().data();
  out = out.remove(0, out.lastIndexOf("Indexing, please wait..."));
  int index = out.indexOf("%");
  if (index == -1) {
    return;
  }
  out = out.remove(index + 1, out.size()).trimmed();
  QString tmp = out;
  tmp = tmp.remove(0, tmp.lastIndexOf(".") + 1);
  tmp = tmp.remove(tmp.indexOf("%"), tmp.size()).trimmed();
  emit progress(tmp.toInt());
}

void FFIndexCaller::indexerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  emit sendInfos(tr("indexerFinished,.."));
  emit
  enableGui(true);
  if (exitCode < 0) {
    emit sendInfos(tr("indexerFinished: %1, %2").arg(exitCode).arg(exitStatus));
    emit
    finished(-1);
    return;
  }
  if (!QFile::exists(m_cache)) {
    emit sendInfos(tr("%1 wasn't created!").arg(m_cache));
    emit
    finished(-2);
    return;
  }
  emit finished(0);
}
