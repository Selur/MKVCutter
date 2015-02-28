#ifndef SUBTITLECUTTER_H
#define SUBTITLECUTTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "tools/subtitlecutters/AssCutter.h"
#include "tools/subtitlecutters/SrtCutter.h"
#include "tools/subtitlecutters/IdxSubCutter.h"

class SubtitleCutter : public QObject
{
  Q_OBJECT
  public:
    explicit SubtitleCutter(QObject *parent = 0);
    void cutSubtitles(QStringList elements, QStringList cutlist, QString tempFolder);
    QStringList getCutSubtitles();

  private:
    QStringList m_cutSubtitles;
    QString m_tempFolder;
    AssCutter *m_assCutter;
    SrtCutter *m_srtCutter;
    IdxSubCutter *m_idxsubCutter;
    QStringList m_inputSubtitles;
    bool m_idxIsRunning;
    QString cutPGSSubtitle(const QString &input, const QString &output, const QStringList &cutList);
    QString cutSrtSubtitle(const QString &input, const QString &output, const QStringList &cutList);
    QString cutAssSubtitle(const QString &input, const QString &output, const QStringList &cutList);
    void cutIdxSubtitle(const QString &input, const QStringList &cutList, const QString &output);

  private slots:
    void idxSubCutterFinished(const QString& outputfile);
    void passThrougInfos(QString infos);
    void passThrougProgress(int position);

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void finished(int state);

};

#endif // SUBTITLECUTTER_H
