#include "SubtitleCutter.h"
#include "Globals.h"
#include <QFile>

SubtitleCutter::SubtitleCutter(QObject *parent)
    : QObject(parent), m_cutSubtitles(), m_tempFolder(QString()), m_idxSubtitle()
{
  m_assCutter = new AssCutter(this, false);
  m_srtCutter = new SrtCutter(this, false);
  m_idxsubCutter = new IdxSubCutter(this);
  QObject::connect(m_idxsubCutter, SIGNAL(finished(QString)), this,
      SLOT(idxSubCutterFinished(QString)));
  QObject::connect(m_idxsubCutter, SIGNAL(sendInfos(QString)), this,
      SLOT(passThrougInfos(QString)));
  QObject::connect(m_idxsubCutter, SIGNAL(progress(int)), this, SLOT(passThrougProgress(int)));
}

QStringList SubtitleCutter::getCutSubtitles()
{
  return m_cutSubtitles;
}

QString SubtitleCutter::cutPGSSubtitle(const QString &input, const QString &output,
    const QStringList &cutList)
{
  Q_UNUSED(input);
  Q_UNUSED(output);
  Q_UNUSED(cutList);
  //QString out = output;
  // TODO: PGS/SUP schneiden. Bis dahin faellt die Spur weg -- ein leerer Name ist das
  // Signal dafuer, und start() gibt nur weiter, was auch wirklich auf der Platte liegt.
  emit sendInfos(
      tr("PGS/SUP subtitles can't be cut yet -- dropping this track."));
  return QString(); //out;
}

QString SubtitleCutter::cutSrtSubtitle(const QString &input, const QString &output,
    const QStringList &cutList)
{
  QString out = output;
  m_srtCutter->cut(input, out, cutList);
  return out;
}

QString SubtitleCutter::cutAssSubtitle(const QString &input, const QString &output,
    const QStringList &cutList)
{
  QString out = output;
  m_assCutter->cut(input, out, cutList);
  return out;
}

void SubtitleCutter::cutIdxSubtitle(const QString &input, const QStringList &cutList,
    const QString &output)
{
  m_idxsubCutter->cut(input, cutList, m_tempFolder, output);
  return;
}

void SubtitleCutter::cutSubtitles(QStringList elements, QStringList cutList, QString tempFolder)
{
  emit sendInfos("cutSubtitles");
  emit sendInfos(" elements:\n  " + elements.join("\n  "));
  emit sendInfos(" cutList:");
  QStringList elems;
  foreach (QString cut, cutList)
  {
    elems = cut.split("-");
    emit sendInfos(
        "  " + QString::number(Cutter::timeToSeconds(elems.at(0))) + "-"
            + QString::number(Cutter::timeToSeconds(elems.at(1))));
  }
  m_cutList = cutList;

  emit sendInfos(" tempFolder: " + tempFolder);
  m_tempFolder = tempFolder;
  if (elements.isEmpty()) {
    emit sendInfos("ELEMENTS IS EMPTY!");
    emit finished(0);
    return;
  }
  if (cutList.isEmpty()) {
    emit sendInfos("CUTLIST IS EMPTY!");
    m_cutSubtitles = elements;
    emit finished(0);
    return;
  }

  m_tempFolder = tempFolder;
  QString outputName, subtitle;
  for (int i = 0, c = elements.count(); i < c; ++i) {
    subtitle = elements.at(i);
    outputName = subtitle;
    outputName = outputName.insert(outputName.lastIndexOf("."), "_cut");
    outputName = outputName.trimmed();
    emit sendInfos(" output name: " + outputName);
    // MkvSubtitleExtractor schreibt PGS-Spuren als ".sup" -- die Pruefung auf "pgs"
    // allein traf deshalb nie zu, und cutPGSSubtitle() war unerreichbar (B17).
    if (outputName.endsWith("pgs", Qt::CaseInsensitive)
        || outputName.endsWith("sup", Qt::CaseInsensitive)) {
      emit sendInfos(" cutting pgs subtitle");
      outputName = this->cutPGSSubtitle(subtitle, outputName, cutList);
    } else if (outputName.endsWith("srt", Qt::CaseInsensitive)) {
      emit sendInfos(" cutting srt subtitle");
      outputName = this->cutSrtSubtitle(subtitle, outputName, cutList);
    } else if (outputName.endsWith("ass", Qt::CaseInsensitive)
        || outputName.endsWith("ssa", Qt::CaseInsensitive)) {
      emit sendInfos(" cutting ass subtitle");
      outputName = this->cutAssSubtitle(subtitle, outputName, cutList);
    } else if (outputName.endsWith("idx", Qt::CaseInsensitive)) {
      m_idxSubtitle << subtitle;
      continue;
    } else {
      emit sendInfos(tr("Ignoring %1 since I don't know it's format.").arg(outputName));
      outputName = QString();
    }
    // Weitergegeben wird nur, was auch wirklich geschrieben wurde. Frueher genuegte der
    // *Name*: der else-Zweig liess ihn stehen, und so landete eine nie erzeugte Datei in
    // der Liste fuer den finalen mkvmerge. Der brach dann mit Exit-Code 2 ab ("could not
    // be opened for reading") -- nachdem alles encodiert war, und ohne Ausgabedatei (B17).
    // Die Pruefung auf QFile::exists() deckt auch einen fehlgeschlagenen SRT- oder
    // ASS-Schnitt ab; cutSrtSubtitle() und cutAssSubtitle() liefern den Namen ungeprueft
    // zurueck.
    if (outputName.trimmed().isEmpty()) {
      emit sendInfos(tr("Dropping the subtitle track of %1.").arg(subtitle));
      continue;
    }
    if (!QFile::exists(outputName)) {
      emit sendInfos(
          tr("Dropping %1: the cut subtitle %2 wasn't written.").arg(subtitle).arg(outputName));
      continue;
    }
    emit sendInfos(tr("Finished cutting %1, output: %2").arg(subtitle).arg(outputName));
    m_cutSubtitles << outputName;
  }
  if (m_idxSubtitle.isEmpty()) {
    emit finished(0);
    return;
  }
  emit sendInfos(" cutting idx/sub subtitle");
  subtitle = m_idxSubtitle.takeFirst();
  outputName = subtitle;
  outputName = outputName.insert(outputName.lastIndexOf("."), "_cut");
  outputName = outputName.trimmed();
  this->cutIdxSubtitle(subtitle, cutList, outputName);
}

void SubtitleCutter::passThrougInfos(QString infos)
{
  emit sendInfos(infos);
}
void SubtitleCutter::passThrougProgress(int position)
{
  emit progress(position);
}

void SubtitleCutter::idxSubCutterFinished(const QString& outputfile)
{
  if (!outputfile.isEmpty()) {
    m_cutSubtitles << outputfile;
  }
  if (m_idxSubtitle.isEmpty()) {
    emit finished(0);
  } else {
    emit sendInfos(" cutting idx/sub subtitle");
    QString subtitle = m_idxSubtitle.takeFirst();
    QString outputName = subtitle;
    outputName = outputName.insert(outputName.lastIndexOf("."), "_cut");
    outputName = outputName.trimmed();
    this->cutIdxSubtitle(subtitle, m_cutList, outputName);
  }
}
