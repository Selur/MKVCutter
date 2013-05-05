/*
 * MkvInfoSourceAnalyser.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#include "MkvInfoSourceAnalyser.h"
#include <QHash>

MkvInfoSourceAnalyser::MkvInfoSourceAnalyser(QObject *parent) :
    QObject(parent), m_process(0), m_input(QString()), m_outData(QString()), m_keyFrameInfos(),
        m_linesread(0),m_crashed(false)
{
  this->setObjectName("MkvInfoSourceAnalyser");
}

MkvInfoSourceAnalyser::~MkvInfoSourceAnalyser()
{

}
QString buildCall(QString input)
{
  QStringList elems;
#ifdef Q_OS_WIN32
  elems << "mkvinfo.exe";
#else
  elems << "mkvinfo";
#endif
  elems << "--ui-language";
#if defined Q_OS_LINUX || defined Q_OS_DARWIN
  elems << "en_US";
#endif
#ifdef Q_OS_WIN32
  elems << "en";
#endif
  elems << " -s";
  elems << "\"" + input + "\"";
  return elems.join(" ");
}

void MkvInfoSourceAnalyser::analyse(QString input)
{
  this->reset();
  m_input = input;
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvinfoFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(handleMkvInfoOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleMkvInfoError()));
  QString call = buildCall(input);
  emit
  enableGui(false);
  emit
  sendInfos(tr("MkvInfoAnalyszer call: %1").arg(call));
  m_process->start(call);
}

void MkvInfoSourceAnalyser::handleMkvInfoOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  m_outData += out;
  m_linesread += out.split("\n").count();
  emit progress(m_linesread);
}

void MkvInfoSourceAnalyser::handleMkvInfoError()
{
  QString error = m_process->readAllStandardError().data();
  error = error.trimmed();
  if (error.isEmpty()) {
    return;
  }
  emit sendInfos(error);
}
void MkvInfoSourceAnalyser::mkvinfoFinished(int exitState, QProcess::ExitStatus status)
{
  if (exitState < 0) {
    emit sendInfos(tr("mkvinfo crashed for %1: %2 - %3").arg(m_input).arg(exitState).arg(status));
    this->reset();
    emit
    enableGui(true);
    emit
    finished();
    return;
  }
  this->analyseOutput();
}

void MkvInfoSourceAnalyser::checkMediaInfo(QString mediaInfo)
{
  emit
  sendInfos(mediaInfo);
  //TODO: Check MediaInfo data for compatibility
  // video needs to be AVC
  // audio needs to be delayCut compatible
  // subtitle need to be ?
  // -> warn if incompatible streams are detected
}

void MkvInfoSourceAnalyser::analyseOutput()
{
  QStringList infos;
  QStringList lines = m_outData.split("\n");
  int videoTrack = -1;
  int index1, index2;
  QString mediaInfo;
  QString line, track, videoData;
  bool gotMediaInfo = false;
  for (int i = 0, c = lines.count(); i < c; ++i) {
    line = lines.at(i);
    //emit sendInfos(tr("looking at: %1").arg(line));
    if (!gotMediaInfo && line.startsWith("Track")) {
      mediaInfo += "\r\n" + line.trimmed();
      if (videoTrack == -1) {
        index1 = line.indexOf(":");
        index2 = line.indexOf(": video");
        if (index1 != index2) {
          continue;
        }
        videoData = line;
        line = line.remove(index1, line.size());
        line = line.remove(0, 6).trimmed();
        videoTrack = line.toInt();
        emit videoTrackID(videoTrack);
        emit sendInfos(tr("video track numer: %1").arg(videoTrack));
        int index = videoData.indexOf(" fps");
        if (index == -1) {
          index = videoData.indexOf("frames/fields per second for a video track");
        }
        if (index != -1) {
          line = videoData;
          line = line.remove(index, line.size());
          line = line.remove(0, line.lastIndexOf("(") + 1);
          emit fps(line.trimmed().toDouble());
        }
      }
      continue;
    }
    gotMediaInfo = true;
    line = line.remove(line.indexOf(" , size"), line.size());
    track = line;
    track = track.remove(0, track.indexOf(",") + 8).trimmed();
    track = track.remove(track.indexOf(","), track.size());
    if (track.toInt() == videoTrack) { //add all the video track frame infos
      infos << line.trimmed();
    }
  }
  this->checkMediaInfo(mediaInfo);
  emit
  frameCount(infos.length());
  QStringList keyFrames;
  QString iframe = "I frame, track " + QString::number(videoTrack) + ", ";
  int currentFrame = -1;
  foreach(QString frame, infos) {
    currentFrame++;
    if (!frame.startsWith(iframe)) {
      continue;
    }
    // I frame, track 1, timecode 20000 (00:00:20.000), size 737102, adler 0x14359d8d
    frame = frame.remove(iframe).trimmed();
    // timecode 20000 (00:00:20.000), size 737102, adler 0x14359d8d
    frame = frame.remove(0, 9);
    // 20000 (00:00:20.000), size 737102, adler 0x14359d8d
    frame = frame.remove(frame.indexOf(", "), frame.size()).trimmed();
    // 20000 (00:00:20.000)
    frame = QString::number(currentFrame) + ", " + frame;
    //currentframe, 20000 (00:00:20.000)
    keyFrames << frame;
  }
  sendInfos(
      tr("MkvvInfo detected %1 frames %2 of them are key frames").arg(currentFrame + 1).arg(
          keyFrames.count()));

  keyFrameInfos(keyFrames);
  emit
  enableGui(true);
  emit finished();
}

void MkvInfoSourceAnalyser::reset()
{
  if (m_process != 0) {
    m_process->disconnect();
    m_process->kill();
    m_process = 0;
  }
  m_input = QString();
  m_keyFrameInfos.clear();
  m_linesread = 0;
  m_crashed = false;
}
