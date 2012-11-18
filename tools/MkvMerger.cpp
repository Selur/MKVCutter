#include "MkvMerger.h"
#include <QApplication>
#include <QDir>

MkvMerger::MkvMerger(QObject *parent) :
    QObject(parent)
{
}

void MkvMerger::handleMkvmergeOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  if (!out.isEmpty()) {
    //emit sendInfos("MkvMerge out: " + out.trimmed());
    QStringList lines = out.split("\n");
    foreach(QString line, lines) {
      if (!line.startsWith("Progress:")) {
        continue;
      }
      line = line.remove(0, 10);
      line = line.remove("%").trimmed();
      emit progress(line.toInt());
      continue;
    }
  }
  QString err = m_process->readAllStandardOutput().data();
  if (!err.isEmpty()) {
    emit sendInfos("MkvMerge err: " + err.trimmed());
  }
}

void MkvMerger::mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit
    finished(-1);
    return;
  }
  QString optionFile = m_output;
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile += "_mkvOptions.txt";
  if (QFile::remove(optionFile)) {
    this->sendInfos(tr("deleted %1").arg(optionFile));
  } else {
    this->sendInfos(tr("only one output file: %1").arg(m_output));
  }
  emit finished(0);
}

void MkvMerger::start(QStringList splitFiles, QStringList audioFiles, QString outputFile)
{
  m_output = outputFile;
  this->call(this->buildCall(splitFiles, audioFiles));
}

void MkvMerger::call(QString call)
{
  this->sendInfos("MKVmerge call: " + call);
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvmergeFinished(int, QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                   SLOT(handleMkvmergeOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleMkvmergeOutput()));
  m_process->start(call);
}
QString doubleBackSlash(QString text)
{
  return text.replace("\\", "\\\\");
}

QString MkvMerger::buildCall(QStringList splitFiles, QStringList audioFiles)
{
  QString appFolder = QApplication::applicationDirPath();
  QString call;
#ifdef Q_OS_WIN32
  call = "mkvmerge.exe";
#else
  call = "mkvmerge";
#endif
  call = "\"" + QDir::toNativeSeparators(appFolder + QDir::separator() + call) + "\"";
  QStringList options;
  options << "-o";
  options << doubleBackSlash(m_output);
  QString videoFiles;
  foreach (QString file, splitFiles) {
    videoFiles += file + "+";
  }
  if (videoFiles.endsWith("+")) {
    videoFiles = videoFiles.remove(videoFiles.size() - 2, 2);
  }
  options << videoFiles;
  QString audio;
  foreach (QString file, audioFiles) {
    audio += file + "+";
  }
  if (audio.endsWith("+")) {
    audio = audio.remove(call.size() - 2, 2);
  }
  options << audio;
  QString optionFile = m_output;
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile += "_mkvOptions.txt";
  if (Globals::saveTextTo(options.join("\n"), optionFile) != 0) {
    emit sendInfos(tr("ERROR: Couldn't save %1!").arg(optionFile));
  } else {
    emit sendInfos(tr("Saved %1.").arg(optionFile));
  }

  call += " @\"" + optionFile + "\"";
  return call;
}
