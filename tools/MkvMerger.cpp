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
  QString err = m_process->readAllStandardError().data();
  if (!err.isEmpty()) {
    emit sendInfos("MkvMerge err: " + err.trimmed());
  }
}

void MkvMerger::mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  emit sendInfos("MkvMerge finished");
  // mkvmerge: 0 = ok, 1 = Warnungen, 2 = Fehler. Geprueft wurde frueher nur auf
  // exitCode < 0, ein abgebrochener Merge galt also als Erfolg -- die Pipeline meldete
  // "fertig", obwohl gar keine Ausgabedatei entstanden war.
  if (exitStatus != QProcess::NormalExit || exitCode < 0 || exitCode >= 2) {
    emit sendInfos(
        tr("ERROR: mkvmerge failed. ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
    emit finished(-1);
    return;
  }
  if (exitCode == 1) {
    emit sendInfos(tr("mkvmerge reported warnings (exit code 1)"));
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
    const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays,
    const QStringList& audioSyncOffsets, const QString& timecodes, const QString& sourceFile,
    const QString& chapterFile, const bool keepIntermediate)
{
  m_output = outputFile;
  m_keepIntermediate = keepIntermediate;
  this->call(
      this->buildCall(splitFiles, audioFiles, subtitleFiles, fps, interlaced, paff, subtitles,
          audioDelays, audioSyncOffsets, timecodes, sourceFile, chapterFile));
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
  m_process->startCommand(call);
}

QString MkvMerger::buildCall(QStringList splitFiles, QStringList audioFiles,
    QStringList subtitleFiles, double fps, const bool interlaced, const bool paff,
    const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays,
                             const QStringList& audioSyncOffsets, const QString& timecodes,
                             const QString& sourceFile, const QString& chapterFile)
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
  options << m_output;
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
  if (splitfileCount == 1 && splitFiles.at(0).endsWith(".mkv")) {
    file = splitFiles.at(0).trimmed();
    // Diese Datei hat mkvmerge selbst aus 'parts-frames:A-B,+C-D' zusammengesetzt. An jeder
    // Naht bekommt das letzte Frame des vorigen Teils dabei eine zu kurze Dauer: gemessen an
    // einem reinen Keyframe-Schnitt (664 Frames, 23,976 fps) standen zwischen den Teilen 21
    // bzw. 14 ms statt 42, die Ausgabe war 27,645 s statt 27,694 s und MediaInfo meldete
    // 24,000 fps. Die geschnittene Zeitstempeldatei kennt die richtigen Dauern -- frueher
    // wurde sie in genau diesem Zweig nie angewendet.
    if (!timecodes.isEmpty()) {
      options << "--timecodes";
      options << "0:" + timecodes;
    }
    options << file;
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
        options << "0:"+timecodes;
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
        options << file;
      } else {
        options << "+";
        options << "(";
        options << file;
        options << ")";
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
  // Die Tonstuecke kommen einzeln aus dem Splitter und werden hier aneinandergehaengt.
  // Jedes Stueck bekommt seinen eigenen Versatz, weil mkvmerge verlustfrei nur auf
  // Frame-Grenzen des Tonformats schneiden kann und die Stuecke dadurch bis zu ein Frame
  // kuerzer oder laenger ausfallen als angefordert -- ohne Versatz summiert sich das auf
  // und der Ton laeuft dem Bild davon (B16). Der Versatz steht in audioSyncOffsets und
  // gehoert jeweils zum Stueck, vor dem er wirken soll.
  //
  // Eine ausdrueckliche '--append-to'-Zuordnung gibt es hier nicht mehr. Sie beschrieb nur
  // das, was mkvmerge ohnehin als Standard nimmt (Spur n an Spur n der Vorgaengerdatei) --
  // und sobald eine Quelle mehrere Tonspuren hat, war sie unvollstaendig: mkvmerge bricht
  // dann mit "Only partial append mappings were given" ab (gemessen an High10.mkv mit zwei
  // DTS-Spuren).
  for (int i = 0, audioCount = audioFiles.count(); i < audioCount; ++i) {
    options << "--no-video";
    options << "--no-global-tags";
    options << "--no-chapters";
    options << "--no-subtitles";
    options << "--no-track-tags";
    options << "--no-buttons";
    if (i == 0) {
      QHashIterator<QString, QString> delay(audioDelays);
      while (delay.hasNext()) {
        delay.next();
        options << "--sync";
        options << delay.key() + ":" + delay.value();
      }
      options << audioFiles.at(i);
      continue;
    }
    if (i < audioSyncOffsets.count() && audioSyncOffsets.at(i).toInt() != 0) {
      // -1 heisst "alle Spuren dieser Datei": eine Quelle kann mehrere Tonspuren haben,
      // und die muessen alle gleich weit verschoben werden.
      options << "--sync";
      options << "-1:" + audioSyncOffsets.at(i);
    }
    options << "+";
    options << "(";
    options << audioFiles.at(i);
    options << ")";
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
    options << file;
  }
  // ATTACHMENTS, GLOBALE TAGS UND KAPITEL
  // Der Videoschnitt streift beides ab, und die neu codierten Teile sind rohe .264-Streams
  // ohne jede Containerinformation -- die Ausgabe hatte deshalb nie Attachments oder Tags
  // (B18). Beides kommt jetzt direkt aus der Quelle: eine Zusatzeingabe, bei der alle
  // Spuren abgeschaltet sind. mkvmerge liest davon nur die Kopfdaten, das kostet auch bei
  // einer 1,6-GB-Quelle keine 0,2 Sekunden.
  //
  // Die Kapitel der Quelle bleiben dabei aussen vor ('--no-chapters'): ihre Zeiten gelten
  // fuer die ungeschnittene Fassung. Die umgerechneten kommen ueber '--chapters'.
  if (!sourceFile.isEmpty()) {
    options << "--no-video";
    options << "--no-audio";
    options << "--no-subtitles";
    options << "--no-chapters";
    options << "--no-track-tags";
    options << "--no-buttons";
    options << sourceFile;
  }
  if (!chapterFile.isEmpty()) {
    options << "--chapters";
    options << chapterFile;
  }
  if (Globals::saveTextTo(Globals::optionsToJson(options), optionFile) != 0) {
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
