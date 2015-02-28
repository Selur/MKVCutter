/*
 * IdxSubCutter.h
 *
 *  Created on: 28.02.2015
 *      Author: Selur
 */

#ifndef TOOLS_SUBTITLECUTTERS_IDXSUBCUTTER_H_
#define TOOLS_SUBTITLECUTTERS_IDXSUBCUTTER_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class IdxSubCutter : public QObject
{
  Q_OBJECT
  public:
    IdxSubCutter(QObject *parent);
    virtual ~IdxSubCutter();
    void cut(const QString &input, const QStringList& cutList, const QString& tempFolder,
        const QString& output);

  private:
    QProcess *m_process;
    QString m_output;

  private slots:
    void idxCutterFinished(int exitState, QProcess::ExitStatus status);
    void idxCutterOutput();

  signals:
    void finished(QString outputidxfilename);
    void sendInfos(QString infos);
    void progress(int position);
};

#endif /* TOOLS_SUBTITLECUTTERS_IDXSUBCUTTER_H_ */
