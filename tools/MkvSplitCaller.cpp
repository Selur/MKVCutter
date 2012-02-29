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
    m_splitPart(QString()), m_outputFolder(QString()), m_tempFiles()
{

}

MkvSplitCaller::~MkvSplitCaller()
{
}

void MkvSplitCaller::handleMkvmergeOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  if (!out.isEmpty()) {
      //emit sendInfos("MkvMerge out: " + out.trimmed());
      QStringList lines  = out.split("\n");
      foreach(QString line, lines) {
          if (line.startsWith("Progress:")) {
              line = line.remove(0, 10);
              line = line.remove("%").trimmed();
              emit progress(line.toInt());
              continue;
          }
          if (!line.startsWith("The file '")) {
              continue;
          }
          line = line.remove(0, line.indexOf("'")+1);
          line = line.remove(line.indexOf("'"), line.size());
          m_tempFiles << line;
      }
  }
  QString err = m_process->readAllStandardOutput().data();
  if (!err.isEmpty()) {
      emit sendInfos("MkvMerge err: " + err.trimmed());
  }
}

void MkvSplitCaller::mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit
    finished(-1);
    return;
  }
  emit splitFiles(m_tempFiles);
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
  //TODO: remove one audio&co can be handled
  call += " --no-audio --no-subtitles --no-buttons --no-track-tags";
  call += " --no-chapters --no-attachments --no-global-tags";
  call += " \""+m_input+"\"";
  return call;
}
