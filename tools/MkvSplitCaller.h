/*
 * MkvSplitCaller.h
 *
 *  Created on: Feb 29, 2012
 *      Author: Selur
 */

#ifndef MKVSPLITCALLER_H_
#define MKVSPLITCALLER_H_

#include <QObject>
#include <QProcess>
#include <QStringList>

class MkvSplitCaller : public QObject
{
  Q_OBJECT
  public:
    MkvSplitCaller(QObject *parent);
    virtual ~MkvSplitCaller();
    void start(QString inputFile, QString outputFile, QStringList splitParts, QString outputFolder, bool audio = false);

  private:
    QProcess *m_process;
    QString m_input, m_output, m_outputFolder;
    QStringList m_tempFiles, m_splitParts;
    bool m_audio;
    void call(QString call);
    QString buildCall();


  private slots:
     void handleMkvmergeOutput();
     void mkvmergeFinished(int exitCode, QProcess::ExitStatus exitStatus);

   signals:
     void enableGui(bool enable);
     void sendInfos(QString infos);
     void progress(int position);
     void finished(int state);
     void splitFiles(QStringList splitFiles);
};

#endif /* MKVSPLITCALLER_H_ */
