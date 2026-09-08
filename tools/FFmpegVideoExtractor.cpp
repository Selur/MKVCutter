/*
 * FFmpegVideoExtractor.cpp
 *
 *  Created on: 20.03.2015
 *      Author: Selur
 */

#include "FFmpegVideoExtractor.h"
#include <QStringList>
#include <QDir>
#include <QApplication>
#include "Globals.h"

FFmpegVideoExtractor::FFmpegVideoExtractor(QObject *parent)
    : QObject(parent), m_process(NULL)
{

}

void FFmpegVideoExtractor::startExtraction(QString filename, QString tempFolder)
{
  this->call(this->buildCall(filename, tempFolder));
}

void FFmpegVideoExtractor::call(QString call)
{
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
      SLOT(ffmpegFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(handleFFmpegOutput()));
  this->sendInfos("FFmpeg extractor call: " + call);
  m_process->startCommand(call);
}

QString FFmpegVideoExtractor::buildCall(QString filename, QString tempFolder)
{
  QStringList call;
  QString ffmpeg = QApplication::applicationDirPath();
  ffmpeg += QDir::separator();
  ffmpeg += "ffmpeg.exe";
  ffmpeg = "\"" + QDir::toNativeSeparators(ffmpeg) + "\"";
  call << ffmpeg;
  call << "-y";
  call << "-i \"" + filename + "\"";
  call << "-vcodec copy";
  call << "-an";
  call << "-sn";
  // '-vsync 0' ist in aktuellen ffmpeg-Builds entfernt ("Unrecognized option 'vsync'",
  // gemessen mit N-125875 / libavcodec 63). Ersatz seit ffmpeg 5.1 ist -fps_mode.
  call << "-fps_mode passthrough";
  call << "-bsf:v h264_mp4toannexb";
  filename = filename.remove(filename.lastIndexOf("."), filename.length());
  filename += ".264";
  filename = tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
  filename = QDir::toNativeSeparators(filename);
  call << "\"" + filename + "\"";
  return call.join(" ");
}

void FFmpegVideoExtractor::handleFFmpegOutput()
{
  emit sendInfos(m_process->readAllStandardError().data());
}

void FFmpegVideoExtractor::ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  emit sendInfos(
      "ffmpegFinished: " + QString::number(exitCode) + ", status " + QString::number(exitStatus));
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit finished(-1);
    return;
  }
  emit finished(0);
}
