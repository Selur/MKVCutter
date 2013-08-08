#include "SubtitleCutter.h"
#include "Globals.h"

SubtitleCutter::SubtitleCutter(QObject *parent) :
    QObject(parent), m_cutSubtitles(), m_tempFolder(QString())
{
    m_assCutter = new AssCutter(this, true);
    m_srtCutter = new SrtCutter(this, true);
}

QStringList SubtitleCutter::getCutSubtitles()
{
    return m_cutSubtitles;
}

QString SubtitleCutter::cutPGSSubtitle(const QString &input, const QString &output, const QStringList &cutList)
{
    QString out = output;
    //TODO: implement cutPGSSubtitle
    return QString(); //out;
}

QString SubtitleCutter::cutSrtSubtitle(const QString &input, const QString &output, const QStringList &cutList)
{
    QString out = output;
    m_srtCutter->cut(input, out, cutList);
    return out;
}

QString SubtitleCutter::cutAssSubtitle(const QString &input, const QString &output, const QStringList &cutList)
{
    QString out = output;
    m_assCutter->cut(input, out, cutList);
    return out;
}

QString SubtitleCutter::cutIdxSubtitle(const QString &input, const QString &output, const QStringList &cutList)
{
    QString out = output;
    //TODO: implement cutIDXSubtitle
    return QString(); //out;
}

void SubtitleCutter::cutSubtitles(QStringList elements, QStringList cutList, QString tempFolder)
{
    emit sendInfos("cutSubtitles");
    emit sendInfos(" elements:\n  "+elements.join("\n  "));
    emit sendInfos(" cutList:");
    QStringList elems;
    foreach (QString cut, cutList) {
        elems = cut.split("-");
        emit sendInfos("  "+QString::number(Cutter::timeToSeconds(elems.at(0)))+"-"+QString::number(Cutter::timeToSeconds(elems.at(1))));
    }

    emit sendInfos(" tempFolder: "+tempFolder);
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
    QString outputName;
    foreach(QString subtitle, elements) {
        outputName = subtitle;
        outputName = outputName.insert(outputName.lastIndexOf("."), "_cut");
        outputName = outputName.trimmed();
        emit sendInfos( " output name: "+outputName);
        if (outputName.endsWith("pgs", Qt::CaseInsensitive)) {
            emit sendInfos(" cutting pgs subtitle");
            outputName = this->cutPGSSubtitle(subtitle, outputName, cutList);
        } else if (outputName.endsWith("srt", Qt::CaseInsensitive)) {
            emit sendInfos(" cutting srt subtitle");
            outputName = this->cutSrtSubtitle(subtitle, outputName, cutList);
        } else if (outputName.endsWith("ass", Qt::CaseInsensitive) || outputName.endsWith("ssa", Qt::CaseInsensitive)) {
            emit sendInfos(" cutting ass subtitle");
            outputName = this->cutAssSubtitle(subtitle, outputName, cutList);
        } else if (outputName.endsWith("ix", Qt::CaseInsensitive)) {
            emit sendInfos(" cutting idx/sub subtitle");
            outputName = this->cutIdxSubtitle(subtitle, outputName, cutList);
        } else {
            emit sendInfos(tr("Ignoring %1 since I don't know it's format.").arg(outputName));
        }
        if (!outputName.trimmed().isEmpty()) {
            emit sendInfos(tr("Finished cutting %1, output: %2").arg(subtitle).arg(outputName));
            m_cutSubtitles << outputName;
        }
    }
    emit finished(0);
}
