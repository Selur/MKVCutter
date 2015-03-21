#ifndef X264CALLER_H
#define X264CALLER_H

#include <QObject>
#include <QProcess>
#include <QString>

class X264Caller : public QObject
{
  Q_OBJECT
  public:
    X264Caller(QObject *parent = 0);
    void start(QString call);

  private:
    QProcess *m_process;
    QProcess *m_helper;

  private slots:
   void handleX264Output();
   void x264Finished(int exitCode, QProcess::ExitStatus exitStatus);

 signals:
   void enableGui(bool enable);
   void sendInfos(QString infos);
   void progress(int position);
   void finished(int state);
};

#endif // X264CALLER_H
