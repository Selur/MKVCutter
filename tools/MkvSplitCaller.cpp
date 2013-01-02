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
    QObject(parent), m_process(NULL), m_input(QString()), m_output(QString()), m_splitParts(),
    m_outputFolder(QString()), m_tempFiles(), m_audio(false), m_keepIntermediate(false)
{

}

MkvSplitCaller::~MkvSplitCaller()
{
}

void MkvSplitCaller::setKeepIntermediate(bool keep)
{
    m_keepIntermediate = keep;
}

void MkvSplitCaller::handleMkvmergeOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  if (!out.isEmpty()) {
    QStringList lines = out.split("\n", QString::SkipEmptyParts);
    int index;
    foreach(QString line, lines) {
      line = line.trimmed();
      //emit sendInfos("MkvMerge output: " + line);
      if (line.startsWith("Progress:")) {
        line = line.remove(0, 10);
        line = line.remove("%").trimmed();
        emit progress(line.toInt());
        index = line.indexOf("The file '");
        if (index == -1) {
          continue;
        }
        line = line.remove(0, index);
        line = line.trimmed();
      }
      index = line.indexOf("The file '");
      if (index == -1) {
        continue;
      }
      line = line.remove(0, index+10);
      line = line.remove(line.indexOf("'"), line.size());
      emit sendInfos(" -> " + tr("new temp file: %1").arg(line));
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
  QString optionFile = m_output;
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile +="_mkvOptions.txt";
  if (!m_keepIntermediate && QFile::remove(optionFile)) {
      this->sendInfos(tr("deleted %1").arg(optionFile));
  } else {
      this->sendInfos(tr("only one output file: %1").arg(m_output));
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
  QString typ = m_audio ? "Audio" : "Video";
  this->sendInfos(typ + " split call: " + call);
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvmergeFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                   SLOT(handleMkvmergeOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleMkvmergeOutput()));
  m_process->start(call);
}
QString MkvSplitCaller::doubleBackSlash(QString text)
{
    return text.replace("\\", "\\\\");
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
    m_output = m_output.insert(m_output.lastIndexOf("."), "_AudioCut");
  }
  QStringList options;
  options << "-o";
  options <<  doubleBackSlash(QDir::toNativeSeparators(m_output));
  if (m_audio) {
    options << "--split";
    options << "parts:"+m_splitParts.join(",+");
    options <<  "--no-video";
  } else {
    options << "--split";
    options << "parts:"+m_splitParts.join(",");
    options << "--no-audio";
    options << "--no-subtitles";
    options << "--no-buttons";
    options << "--no-track-tags";
    options << "--no-chapters";
    options << "--no-attachments";
    options << "--no-global-tags";
  }
  options << doubleBackSlash(QDir::toNativeSeparators(m_input));
  QString optionFile = m_output;
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile +="_mkvOptions.txt";
  emit sendInfos(tr("Saving options:"));
  emit sendInfos("---------------------------");
  foreach (QString option, options) {
    emit sendInfos(option);
  }
  emit sendInfos("---------------------------");
  emit sendInfos(tr("to:  %1").arg(optionFile));
  if (Globals::saveTextTo(options.join("\n"), optionFile)!= 0) {
      emit sendInfos(tr("ERROR: Couldn't save %1!").arg(optionFile));
  } else {
      emit sendInfos(tr("Saved %1.").arg(optionFile));
  }

  call += " @\""+Globals::shortFileName(optionFile)+"\"";
  return call;
}
