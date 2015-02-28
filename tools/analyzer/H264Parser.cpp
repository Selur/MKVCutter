/*
 * H264Parser.cpp
 *
 *  Created on: 21.06.2013
 *      Author: Selur
 */

#include "H264Parser.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QApplication>

H264Parser::H264Parser(QObject *parent) :
    QObject(parent), m_process(nullptr)
{
  this->setObjectName("H264Parser");
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this,
                   SLOT(h264ParseFinished(int,QProcess::ExitStatus)));
}

H264Parser::~H264Parser()
{

}

void H264Parser::removeStartOfLine(QString &line)
{
  line = line.remove(0, line.indexOf(":") + 1);
  line = line.trimmed();
}

void H264Parser::analyseOutput(QString output)
{
  QStringList lines = output.split("\n");
  foreach(QString line, lines)
  {
    line = line.trimmed();
    if (line.startsWith("num_ref_frames")) {
      this->removeStartOfLine(line);
      emit refframes(line.toInt());
      continue;
    }
    if (line.startsWith("weighted_pred_flag")) {
      this->removeStartOfLine(line);
      emit weightedP(line.toInt());
      continue;
    }
    if (line.startsWith("weighted_bipred_idc")) {
      this->removeStartOfLine(line);
      emit weightedB(line.toInt());
      continue;
    }
    if (line.startsWith("num_reorder_frames")) {
      this->removeStartOfLine(line);
      emit bframes(line.toInt());
      continue;
    }
    if (line.startsWith("pic_init_qp_minus26")) {
      this->removeStartOfLine(line);
      emit qpMin(line.toInt());
      continue;
    }
    if (line.startsWith("chroma_qp_index_offset")) {
      this->removeStartOfLine(line);
      emit chromaOffset(line.toInt());
      continue;
    }
  }
  emit finished();
}

void H264Parser::analyse(QString input)
{
  if (input.isEmpty() || !QFile::exists(input)) {
    emit sendInfo(tr("Input is empty or doesn't exist!"));
    emit finished();
    return;
  }

  QString h264Parse = QApplication::applicationDirPath();
  h264Parse += QDir::separator();
#ifdef Q_OS_WIN
  h264Parse += "h264_parse.exe";
#else
  h264Parse += "h264_parse";
#endif
  h264Parse = QDir::toNativeSeparators(h264Parse);
  QStringList call;
  call << "\"" + h264Parse + "\"";
  call << "\"" + input + "\"";
  emit sendInfo(tr("Analyzing %1 with h264_parse,..").arg(input));
  m_process->start(call.join(" "));
}

void H264Parser::h264ParseFinished(int exitState, QProcess::ExitStatus status)
{
  if (exitState < 0) {
    emit sendInfo(" "+tr("h264_parse finished(%1, %2).").arg(exitState).arg(status));
  }
  QString output = m_process->readAllStandardOutput();
  output += m_process->readAllStandardError();
  if (output.isEmpty()) {
    emit finished();
    return;
  }
  this->analyseOutput(output);
}
