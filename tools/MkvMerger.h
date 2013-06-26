#ifndef MKVMERGER_H
#define MKVMERGER_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class MkvMerger : public QObject
{
  Q_OBJECT
  public:
    MkvMerger(QObject *parent = 0);
    void start(QStringList toMerge, QStringList audioFile2, QString outputFile, const double fps, const bool interlaced, const bool paff);

  private:
    QProcess *m_process;
    QString m_output;
    QString m_optionsFile;
    void call(QString call);
    QString buildCall(QStringList splitFiles, QStringList audioFiles, double fps, const bool interlaced, const bool paff);
    QString doubleBackSlash(QString text);

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
