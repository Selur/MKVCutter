#ifndef MKVTIMEEXTRACTOR_H
#define MKVTIMEEXTRACTOR_H

#include <QObject>
#include <QString>
#include <QProcess>

class MkvTimeExtractor : public QObject
{
    Q_OBJECT
    public:
      explicit MkvTimeExtractor(QObject *parent = 0);
      void startExtraction(QString filename, QString track, QString tempFolder);

    private:
      QProcess *m_process;
      QString m_timecodeFile;
      void call(QString call);
      QString buildCall(QString filename, QString track, QString tempFolder);

    private slots:
      void handleMkvExtractOutput();
      void mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus);

    signals:
      void enableGui(bool enable);
      void sendInfos(QString infos);
      void progress(int position);
      void timecodes(QString timecodefile);
      void finished(int state);
    
};

#endif // MKVTIMEEXTRACTOR_H
