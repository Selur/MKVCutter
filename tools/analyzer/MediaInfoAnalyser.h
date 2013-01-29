#ifndef MEDIAINFOANALYSER_H
#define MEDIAINFOANALYSER_H

#include <QObject>
#include <QObject>
#include <QProcess>

class MediaInfoAnalyser : public QObject
{
  Q_OBJECT
  public:
    MediaInfoAnalyser(QObject *parent = 0);
    void analyse(QString input);

  private:
    QProcess *m_process;
    QString m_audioFormat;

  private slots:
    void mediainfoFinished(int exitState, QProcess::ExitStatus status);
    void mediainfoOutput();

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void avcProfileLevel(QString pl);
    void refframes(int refframe);
    void cabac(bool use);
    void audioFormat(QString format);
    void finished(int state);
    void averageBitrate(int bitrate);
    void aspectRatio(double aspectRatio);
    void interlaced(QString scanOrder);
    void frameRateMode(bool vfr);
    void x264Settings(QString settings);

};

#endif // MEDIAINFOANALYSER_H
