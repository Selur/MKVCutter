#include "MkvMerger.h"
#include <QApplication>
#include <QDir>
#include "Globals.h"

MkvMerger::MkvMerger(QObject *parent) :
    QObject(parent), m_output(QString())
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
  /*if (QFile::remove(optionFile)) {
    this->sendInfos(tr("deleted %1").arg(optionFile));
  } else {
    this->sendInfos(tr("only one output file: %1").arg(m_output));
  }*/
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
QString MkvMerger::doubleBackSlash(QString text)
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
  QStringList files;
  if (splitFiles.count() > 0) {
   options << "--compression";
   options << "-1:none";
  }
  foreach (QString file, splitFiles) {
      files << doubleBackSlash(file);
  }
  options << files.join("\n+");
  QString optionFile;
  if (!files.isEmpty()) {
      optionFile = files.at(0);
  }
  files.clear();
  if (audioFiles.count() > 0) {
   options << "--compression";
   options << "-1:none";
  }
  foreach (QString file, audioFiles) {
    files << doubleBackSlash(file);
  }
  options << files.join("\n+");
  if (optionFile.isEmpty() && !files.isEmpty()) {
      optionFile = files.at(0);
  }
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile += "_mkvOptions.txt";
  if (Globals::saveTextTo(options.join("\n"), optionFile) != 0) {
    emit sendInfos(tr("ERROR: Couldn't save %1!").arg(optionFile));
  } else {
    emit sendInfos(tr("Saved %1.").arg(optionFile));
  }
  while (optionFile.contains("\\\\")) {
    optionFile = optionFile.replace("\\\\","\\");
  }
  call += " @\"" + optionFile + "\"";
  return call;
}
