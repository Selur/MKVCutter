/*
 * FFmpegVideoExtractor.h
 *
 *  Created on: 20.03.2015
 *      Author: Selur
 */

#ifndef FFMPEGVIDEOEXTRACTOR_H_
#define FFMPEGVIDEOEXTRACTOR_H_

#include <QObject>
#include <QProcess>
#include <QString>

class FFmpegVideoExtractor : public QObject
{
  Q_OBJECT
  public:
    FFmpegVideoExtractor(QObject *parent = 0);
    void startExtraction(QString filename, QString tempFolder);

  private:
    QProcess *m_process;
    void call(QString call);
    QString buildCall(QString filename, QString tempFolder);

  private slots:
    void handleFFmpegOutput();
    void ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void finished(int state);
};
#endif /* FFMPEGVIDEOEXTRACTOR_H_ */
