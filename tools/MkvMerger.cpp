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
      QStringList lines  = out.split("\n");
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
  emit finished(0);
}

void MkvMerger::start(QStringList splitFiles, QStringList audioFiles, QString outputFile)
{
  m_output = outputFile;
  this->call(this->buildCall(splitFiles, audioFiles));
}

void MkvMerger::call(QString call)
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
  call += " -o \"" + m_output + "\"";
  foreach (QString file, splitFiles) {
   call += " \"" + file + "\" +";
  }
  if (call.endsWith("+")) {
      call = call.remove(call.size()-2, 2);
  }
  foreach (QString file, audioFiles) {
   call += " \"" + file + "\" +";
  }
  if (call.endsWith("+")) {
      call = call.remove(call.size()-2, 2);
  }

  return call.trimmed();
}
