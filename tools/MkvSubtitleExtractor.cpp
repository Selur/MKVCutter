#include "MkvSubtitleExtractor.h"
#include "Globals.h"

#include <QDir>
#include <QApplication>

MkvSubtitleExtractor::MkvSubtitleExtractor(QObject *parent) :
    QObject(parent), m_process(nullptr), m_outputFiles()
{
}

QStringList MkvSubtitleExtractor::getOutputFiles()
{
    return m_outputFiles;
}

void MkvSubtitleExtractor::startExtraction(QString inputFile, QList<SubtitleTrack> tracks, QString tempFolder)
{
  QString call = this->buildCall(inputFile, tracks, tempFolder);
  this->call(call);
}

void MkvSubtitleExtractor::call(QString call)
{
  this->sendInfos("Mkv subtitle extractor call: " + call);
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvextractFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                   SLOT(handleMkvExtractOutput()));
  m_process->startCommand(call);
}

QString extensionForFormat(QString typ)
{
    QString extension = ".txt";
  bool ssa = typ.startsWith("S_SSA") || typ.startsWith("S_TEXT/SSA");
  bool ass = typ.startsWith("S_ASS") || typ.startsWith("S_TEXT/ASS");
  bool text = typ.startsWith("S_TEXT");
  bool tele = typ.startsWith("tele", Qt::CaseInsensitive);
  bool sub = typ.startsWith("S_VOBSUB") || typ.startsWith("subp", Qt::CaseInsensitive)
    || typ.startsWith("subrip", Qt::CaseInsensitive);
  bool tx3g = typ.startsWith("tx3g", Qt::CaseInsensitive);
  bool ttxt = typ.startsWith("ttxt", Qt::CaseInsensitive);
  bool pgs = typ.startsWith("S_HDMV/PGS");
  if (ass) {
     extension = ".ass";
  } else if (ssa) {
    extension = ".ssa";
  } else if (text || tele) {
    extension = ".srt";
  } else if (sub) {
    extension = ".idx";
  } else if (ttxt || tx3g) {
    extension = ".srt";
  }else if (pgs) {
    extension = ".sup";
  }
  return extension;
}

QString MkvSubtitleExtractor::buildCall(QString inputFile,QList<SubtitleTrack> tracks, QString tempFolder)
{
  QString call = QApplication::applicationDirPath();
  call += QDir::separator();
  call += "mkvextract.exe";
  call = "\"" + QDir::toNativeSeparators(call) + "\"";
  call += " tracks";
  call += " \"" + inputFile + "\"";
  foreach(SubtitleTrack track, tracks)
  {
      call += " "+QString::number(track.trackID)+":";
      QString filename = inputFile;
      filename = filename.remove(filename.lastIndexOf("."), filename.length());
      filename += "_track_" + QString::number(track.trackID);
      filename += extensionForFormat(track.type);
      filename = tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
      filename = QDir::toNativeSeparators(filename);
      m_outputFiles << filename;
      call += "\"" + filename + "\"";
  }
  return call;
}

void MkvSubtitleExtractor::handleMkvExtractOutput()
{
  QString text = m_process->readAllStandardOutput().data();
  if (!text.startsWith("Progress: ")) {
    return;
  }
  text = text.remove(0, text.indexOf(":") + 1);
  text = text.remove(text.indexOf("%"), text.size());
  text = text.remove("%").trimmed();
  emit progress(int(text.toDouble()));
}

void MkvSubtitleExtractor::mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit finished(-1);
    return;
  }
  emit finished(0);
}
