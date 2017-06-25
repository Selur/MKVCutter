#include "MkvMerger.h"
#include <QApplication>
#include <QDir>
#include <QHashIterator>
#include "Globals.h"

MkvMerger::MkvMerger(QObject *parent)
    : QObject(parent), m_process(nullptr), m_output(QString()), m_keepIntermediate(false)
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
  if (!m_keepIntermediate) {
    if (QFile::remove(m_optionsFile)) {
      this->sendInfos(tr("deleted %1").arg(m_optionsFile));
    } else {
      this->sendInfos(tr("coudln't delete %1").arg(m_optionsFile));
    }
  }
  m_optionsFile = QString();
  emit finished(0);
}

void MkvMerger::start(QStringList splitFiles, QStringList audioFiles, QStringList subtitleFiles,
    QString outputFile, const double fps, const bool interlaced, const bool paff,
    const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays, const QString& timecodes, const bool keepIntermediate)
{
  m_output = outputFile;
  m_keepIntermediate = keepIntermediate;
  this->call(
      this->buildCall(splitFiles, audioFiles, subtitleFiles, fps, interlaced, paff, subtitles, audioDelays, timecodes));
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
    QStringList subtitleFiles, double fps, const bool interlaced, const bool paff,
    const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays,
                             const QString& timecodes)
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
  splitFiles.sort();
  QString optionFile, file;
  QStringList append;
  if (splitfileCount == 1 && splitFiles.at(0).endsWith(".mkv")) {
    file = splitFiles.at(0).trimmed();
    options << doubleBackSlash(file);
    optionFile = file;
  } else {
    QString fpsValue = Globals::decimalToFractionConvert(fps);
    QString fpsExtension = "p";
    if (interlaced) {
      fpsExtension = "i";
      if (!paff) {
        fpsValue = Globals::decimalToFractionConvert(fps * 2);
      }
    }
    for (int i = 0; i < splitfileCount; ++i) {
      options << "--no-global-tags";
      options << "--no-chapters";
      options << "--no-subtitles";
      options << "--no-track-tags";
      options << "--no-buttons";
      options << "--no-audio";
      options << "--no-attachments";
      // TIME CODES
      if (i == 0 && !timecodes.isEmpty()) {
        options << "--timecodes";
        options << "0:"+doubleBackSlash(timecodes);
      }

      options << "--forced-track";
      options << "0:no";
      options << "--default-duration";
      options << "0:" + fpsValue + fpsExtension;
      options << "--fix-bitstream-timing-information";
      options << "0:1";
      file = splitFiles.at(i);
      if (i == 0) {
        optionFile = file;
        options << doubleBackSlash(file);
      } else {
        options << "+";
        options << "(";
        options << doubleBackSlash(file);
        options << ")";
        append << QString::number(i) + ":0:" + QString::number(i - 1) + ":0";
      }
    }
  }
// OPTION FILE
  int index = optionFile.indexOf("_withoutSubs");
  if (index != -1) {
    optionFile = optionFile.remove(index, optionFile.size());
  } else {
    optionFile = optionFile.remove(optionFile.lastIndexOf("."), optionFile.size());
  }
  optionFile += "_mkvOptions.txt";

// AUDIO FILES
  foreach (QString file, audioFiles)
  {
    options << "--no-video";
    options << "--no-global-tags";
    options << "--no-chapters";
    options << "--no-subtitles";
    options << "--no-track-tags";
    options << "--no-buttons";
    QHashIterator<QString, QString> i(audioDelays);
    while (i.hasNext()) {
      i.next();
      options << "--sync";
      options << i.key()+ ":" +i.value();
    }

    options << doubleBackSlash(file);
  }
  QString lang;
// SUBTITLE FILES
  foreach(QString file, subtitleFiles)
  {
    options << "--no-video";
    options << "--no-audio";
    options << "--no-global-tags";
    options << "--no-chapters";
    options << "--no-track-tags";
    options << "--subtitle-tracks";
    options << "0";
    lang = file;
    index = lang.lastIndexOf("_track_");
    if (index != -1) {
      lang = lang.remove(0, index + 7);
      lang = lang.remove(lang.lastIndexOf("."), lang.size()).trimmed();
      index = lang.toInt();
      lang = subtitles.at(index).language;
      if (lang != QString()) {
        options << "--language";
        options << "0:" + lang;
      }
    }
    options << "--compression";
    options << "-1:none";
    options << doubleBackSlash(file);
  }
  if (!append.isEmpty()) {
    options << "--append-to";
    options << append.join(",");
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
