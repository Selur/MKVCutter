/*
 * MkvInfoSourceAnalyser.h
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#ifndef MKVINFOSOURCEANALYSER_H_
#define MKVINFOSOURCEANALYSER_H_

#include <QObject>
#include <QString>
#include <QProcess>

struct SubtitleTrack
{
    int trackID;
    QString type;
};

class MkvInfoSourceAnalyser : public QObject
{
  Q_OBJECT
  public:
    MkvInfoSourceAnalyser(QObject *parent = 0);
    virtual ~MkvInfoSourceAnalyser();
    void analyse(QString input);

  private:
    QProcess *m_process;
    QString m_input, m_outData;
    QStringList m_keyFrameInfos;
    int m_linesread;
    bool m_crashed;
    void reset();
    void analyseOutput();
    void checkMediaInfo(QString mediaInfo);

  private slots:
    void mkvinfoFinished(int exitState, QProcess::ExitStatus status);
    void handleMkvInfoOutput();
    void handleMkvInfoError();

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void frameCount(int count);
    void keyFrameInfos(QStringList infos);
    void finished();
    void fps(double framerate);
    void videoTrackID(int videoTrack);
    void subtitleTrack(SubtitleTrack track);
};

#endif /* MKVINFOSOURCEANALYSER_H_ */
