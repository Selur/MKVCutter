/*
 * H264Parser.h
 *
 *  Created on: 21.06.2013
 *      Author: Selur
 */

#ifndef H264PARSER_H_
#define H264PARSER_H_

#include <QObject>
#include <QProcess>
#include <QString>

class H264Parser : public QObject
{
  Q_OBJECT
  public:
    H264Parser(QObject *parent = 0);
    virtual ~H264Parser();
    void analyse(QString input);

  private:
    QProcess *m_process;
    void analyseOutput(QString output);
    void removeStartOfLine(QString &line);

  private slots:
    void h264ParseFinished(int exitState, QProcess::ExitStatus status);

  signals:
    void finished();
    void sendInfo(QString info);
    void refframes(int count);
    void weightedP(int value);
    void weightedB(int value);
    void bframes(int value);
    void qpMin(int value);
    void chromaOffset(int value);
    void sps(int value);

};

#endif /* H264PARSER_H_ */
