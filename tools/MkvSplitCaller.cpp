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
    m_splitParts(), m_outputFolder(QString()), m_tempFiles(),m_audio(false)
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
  if (m_tempFiles.isEmpty() && QFile::exists(m_output)) {
      //this->sendInfos(tr("only one output file: %1").arg(m_output));
      m_tempFiles << m_output;
  }
  emit splitFiles(m_tempFiles);
  emit finished(0);
}

void MkvSplitCaller::start(QString inputFile, QString outputFile, QStringList splitParts,
                           QString outputFolder, bool audio)
{
  m_audio = audio;
  m_output = outputFile;
  m_input = inputFile;
  m_splitParts.clear();
  m_splitParts = splitParts;
  m_outputFolder = outputFolder;
  this->call(this->buildCall());
}

void MkvSplitCaller::call(QString call)
{
  QString typ = m_audio ? "Audio": "Video";
  //this->sendInfos(typ+" split call: "+call);
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

  m_output = m_outputFolder + QDir::separator() + Globals::getWholeFileName(m_output);
  if (m_audio) {
    m_output = m_output.insert(m_output.lastIndexOf("."),"_AudioCut");
  }
  call += " -o \"" + QDir::toNativeSeparators(m_output) + "\"";
  call += " --split parts:"+m_splitParts.join(",");
  if (m_audio) {
    call += " --no-video";
  } else {
    call += " --no-audio";
    call += " --no-subtitles --no-buttons --no-track-tags";
    call += " --no-chapters --no-attachments --no-global-tags";
  }
  call += " \""+m_input+"\"";
  return call;
}
