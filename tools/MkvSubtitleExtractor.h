#ifndef MKVSUBTITLEEXTRACTOR_H
#define MKVSUBTITLEEXTRACTOR_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <tools/analyzer/MkvInfoSourceAnalyser.h>

class MkvSubtitleExtractor : public QObject
{
    Q_OBJECT
    public:
        MkvSubtitleExtractor(QObject *parent = 0);
        void startExtraction(QString inputFile,QList<SubtitleTrack> tracks, QString tempFolder);
        QStringList getOutputFiles();

      private:
        QProcess *m_process;
        QStringList m_outputFiles;
        void call(QString call);
        QString buildCall(QString inputFile, QList<SubtitleTrack> tracks, QString tempFolder);

      private slots:
        void handleMkvExtractOutput();
        void mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus);

      signals:
        void enableGui(bool enable);
        void sendInfos(QString infos);
        void progress(int position);
        void finished(int state);

};

#endif // MKVSUBTITLEEXTRACTOR_H
