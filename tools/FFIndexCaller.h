/*
 * FFIndexCaller.h
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#ifndef FFINDEXCALLER_H_
#define FFINDEXCALLER_H_

#include <QObject>
#include <QString>
#include <QProcess>

class FFIndexCaller : public QObject
{
  Q_OBJECT
  public:
    FFIndexCaller(QObject *parent = nullptr);
    virtual ~FFIndexCaller();
    void index(QString inputFile, QString cacheFile);

  private:
    QString m_input, m_cache;
    QProcess *m_process;

  private slots:
    void handleIndexerOutput();
    void indexerFinished(int exitCode, QProcess::ExitStatus exitStatus);

  signals:
    void enableGui(bool enable);
    void sendInfos(QString infos);
    void progress(int position);
    void finished(int state);
};

#endif /* FFINDEXCALLER_H_ */
