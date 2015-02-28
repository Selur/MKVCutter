#include "SubtitleCutter.h"
#include "Globals.h"

SubtitleCutter::SubtitleCutter(QObject *parent)
    : QObject(parent), m_cutSubtitles(), m_tempFolder(QString()), m_inputSubtitles(),
        m_idxIsRunning(false)
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
  QString out = output;
  emit sendInfos(" ignored subtitles since pgs cutting is not implemented");
  //TODO: implement cutPGSSubtitle
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
  m_inputSubtitles = elements;
  for (int i = 0, c = m_inputSubtitles.count(); i < c; ++i) {
    subtitle = m_inputSubtitles.takeFirst();
    outputName = subtitle;
    outputName = outputName.insert(outputName.lastIndexOf("."), "_cut");
    outputName = outputName.trimmed();
    emit sendInfos(" output name: " + outputName);
    if (outputName.endsWith("pgs", Qt::CaseInsensitive)) {
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
      while (m_idxIsRunning) {
        // waiting
      }
      emit sendInfos(" cutting idx/sub subtitle");
      m_idxIsRunning = true;
      this->cutIdxSubtitle(subtitle, cutList, outputName);
      continue;
    } else {
      emit sendInfos(tr("Ignoring %1 since I don't know it's format.").arg(outputName));
    }
    if (!outputName.trimmed().isEmpty()) {
      emit sendInfos(tr("Finished cutting %1, output: %2").arg(subtitle).arg(outputName));
      m_cutSubtitles << outputName;
    }
  }
  if (m_inputSubtitles.isEmpty() && !m_idxIsRunning) {
    emit finished(0);
  }
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
  m_idxIsRunning = false;
  if (outputfile.isEmpty()) {
    return;
  }
  m_cutSubtitles << outputfile;
  if (m_inputSubtitles.isEmpty()) {
    emit finished(0);
  }
}
