#ifndef MKVMERGER_H
#define MKVMERGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include <QHash>
#include <QStringList>
#include "tools/Subtitletrack.h"

class MkvMerger : public QObject
{
  Q_OBJECT
  public:
    MkvMerger(QObject *parent = 0);
    void start(QStringList toMerge, QStringList audioFile2, QStringList subtitleFiles, QString outputFile, const double fps, const bool interlaced, const bool paff, const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays, const QStringList& audioSyncOffsets, const QString& timecodes, const bool keepIntermediate);

  private:
    QProcess *m_process;
    QString m_output;
    QString m_optionsFile;
    bool m_keepIntermediate;
    void call(QString call);
    QString buildCall(QStringList splitFiles, QStringList audioFiles, QStringList subtitleFiles, double fps, const bool interlaced, const bool paff, const QList<SubtitleTrack>& subtitles, const QHash<QString, QString>& audioDelays, const QStringList& audioSyncOffsets, const QString& timecodes);

  private slots:
    void handleMkvmergeOutput();
    void mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus);

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void finished(int state);

};

#endif // MKVMERGER_H
