#include "MkvMerger.h"
#include <QApplication>
#include <QDir>
#include "Globals.h"

MkvMerger::MkvMerger(QObject *parent)
    : QObject(parent), m_process(nullptr), m_output(QString())
{
}

void MkvMerger::handleMkvmergeOutput()
{
  if (m_process == nullptr) {
    return;
  }
  QString out = m_process->readAllStandardOutput().data();
  if (!out.isEmpty()) {
    //emit sendInfos("MkvMerge out: " + out.trimmed());
    QStringList lines = out.split("\n");
    foreach(QString line, lines)
    {
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
  emit sendInfos("MkvMerge finished");
  if (exitCode < 0) {
    emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit finished(-1);
    return;
  }
  if (QFile::remove(m_optionsFile)) {
    this->sendInfos(tr("deleted %1").arg(m_optionsFile));
  } else {
    this->sendInfos(tr("coudln't delete %1").arg(m_optionsFile));
  }
  m_optionsFile = QString();
  emit finished(0);
}

void MkvMerger::start(QStringList splitFiles, QStringList audioFiles, QStringList subtitleFiles,
    QString outputFile, const double fps, const bool interlaced, const bool paff)
{
  m_output = outputFile;
  this->call(this->buildCall(splitFiles, audioFiles, subtitleFiles, fps, interlaced, paff));
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

QString MkvMerger::buildCall(QStringList splitFiles, QStringList audioFiles,
    QStringList subtitleFiles, double fps, const bool interlaced, const bool paff)
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
  options << "--clusters-in-meta-seek";
  options << "--engage";
  options << "no_simpleblocks";
  options << "--disable-lacing";
  options << "--engage";
  options << "no_cue_duration";
  options << "--engage";
  options << "no_cue_relative_position";

// VIDEO FILES
  int splitfileCount = splitFiles.count();
  QString optionFile;
  if (splitfileCount == 1 && splitFiles.at(0).endsWith(".mkv")) {
    options << doubleBackSlash(splitFiles.at(0));
  } else {
    QString fpsValue = Globals::decimalToFractionConvert(fps);
    QString fpsExtension = "p";
    if (interlaced) {
      fpsExtension = "i";
      if (!paff) {
        fpsValue = Globals::decimalToFractionConvert(fps * 2);
      }
    }
    QString file;
    if (splitfileCount > 0) {
      options << "--no-audio";
      options << "--default-duration";
      options << "0:" + fpsValue + fpsExtension;
      options << "--fix-bitstream-timing-information";
      options << "0:1";
    }
    if (splitfileCount > 1) {
      options << "(";
    }
    for (int i = 0; i < splitfileCount; ++i) {
      file = splitFiles.at(i);
      options << doubleBackSlash(file);
      if (i == 0) {
        optionFile = file;
      }
    }
    if (splitfileCount > 1) {
      options << ")";
    }
  }
// OPTION FILE
  optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  optionFile += "_mkvOptions.txt";

// AUDIO FILES
  foreach (QString file, audioFiles)
  {
    options << "--no-video";
    options << doubleBackSlash(file);
  }

// SUBTITLE FILES
  foreach(QString file, subtitleFiles)
  {
    options << "--compression";
    options << "-1:none";
    options << doubleBackSlash(file);
  }

  if (Globals::saveTextTo(options.join("\n"), optionFile) != 0) {
    emit sendInfos(tr("ERROR: Couldn't save %1!").arg(optionFile));
  } else {
    emit sendInfos("  " + tr("Saved mkvoptions file: %1").arg(optionFile));
    emit sendInfos("   ----------------------------");
    emit sendInfos(options.join("\n"));
    emit sendInfos("   ----------------------------");
  }
  while (optionFile.contains("\\\\")) {
    optionFile = optionFile.replace("\\\\", "\\");
  }
  m_optionsFile = Globals::removeQuotes(optionFile);
  call += " @\"" + optionFile + "\"";
  return call;
}
