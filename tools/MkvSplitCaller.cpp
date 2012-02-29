/*
 * MkvSplitCaller.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: Selur
 */

#include "MkvSplitCaller.h"
#include <QApplication>
#include <QDir>
#include "Globals.h"

MkvSplitCaller::MkvSplitCaller(QObject *parent) :
    QObject(parent), m_process(NULL), m_input(QString()), m_output(QString()),
        m_splitPart(QString()), m_outputFolder(QString())
{

}

MkvSplitCaller::~MkvSplitCaller()
{
}

void MkvSplitCaller::handleMkvmergeOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  emit
  sendInfos("MkvMerge out: " + out);
  QString err = m_process->readAllStandardOutput().data();
  emit sendInfos("MkvMerge err: " + err);
}

void MkvSplitCaller::mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit
    finished(-1);
    return;
  }
  emit finished(0);
}

void MkvSplitCaller::start(QString inputFile, QString outputFile, QString splitPart,
                           QString outputFolder)
{
  m_output = outputFile;
  m_input = inputFile;
  m_splitPart = splitPart;
  m_outputFolder = outputFolder;
  this->call(this->buildCall());
}

void MkvSplitCaller::call(QString call)
{
  this->sendInfos("MKVmerge call: "+call);
  return;
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvmergeFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                   SLOT(handleMkvmergeOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleMkvmergeOutput()));
  m_process->start(call);
}

QString MkvSplitCaller::buildCall()
{
  QString appFolder = QApplication::applicationDirPath();
  QString call;
#ifdef Q_OS_WIN32
  call = "mkvmerge.exe";
#else
  call = "mkvmerge";
#endif
  call = "\"" + QDir::toNativeSeparators(appFolder + QDir::separator() + call) + "\"";

  QString output = m_outputFolder + QDir::separator() + Globals::getWholeFileName(m_output);
  call += " -o \"" + QDir::toNativeSeparators(output) + "\"";
  call += " --split timecodes:" + m_splitPart;
  return call;
}
