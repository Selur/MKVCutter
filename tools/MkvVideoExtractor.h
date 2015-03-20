#ifndef MKVVIDEOEXTRACTOR_H
#define MKVVIDEOEXTRACTOR_H

#include <QObject>
#include <QProcess>
#include <QString>

class MkvVideoExtractor : public QObject
{
  Q_OBJECT
  public:
    explicit MkvVideoExtractor(QObject *parent = 0);
    void startExtraction(QString filename, QString track, QString typ, QString tempFolder);

  private:
    QProcess *m_process;
    void call(QString call);
    QString buildCall(QString filename, QString track, QString typ, QString tempFolder);

  private slots:
    void handleMkvExtractOutput();
    void mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus);

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void finished(int state);
};

#endif // MKVVIDEOEXTRACTOR_H
