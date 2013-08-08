/*
 * Cutter.h
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#ifndef CUTTER_H_
#define CUTTER_H_

#include <QTime>
#include <QObject>
#include <QString>
#include <QStringList>

struct SubtitleEntry
{
    double from, to;
    QString text;
    bool fixed;
};

class Cutter : public QObject
{
  Q_OBJECT
  public:
    Cutter(QObject *parent, bool debug = false);
    virtual ~Cutter();
    void cut(const QString input, QString &output, const QStringList cutList);
    static QString readAll(const QString fileName, QString type = "auto");
    static double timeToSeconds(QString value);
    static double timeToSeconds(QTime time);

  protected:
    virtual QString cutContent(const QString &content, const QStringList &cutList) = 0;
    virtual QList<SubtitleEntry> tokenize(const QString content) = 0;
    bool m_debug;
    QString m_input, m_output;
    QStringList m_acceptedExtensions;
    void adjustEntries(QList<SubtitleEntry> &entries, const QStringList &cutList);
    QString secondsToHMSZZZ(double seconds);
    QString getDirectory(const QString input);
    QString removeLastSeparatorFromPath(QString input);
    int saveTextTo(QString text, QString to);
    void adjustEntry(SubtitleEntry &entry, const QStringList &cutList);
    void collectEntriesForCuts(QList<SubtitleEntry> &entries, const QStringList &cutList);
    void adjustTimingsofEntries(QList<SubtitleEntry> &entries, const QStringList &cutList);
    void adjustTimingsofEntries2(QList<SubtitleEntry> &entries, const QStringList &cutList);
    void adjustTimingsofEntries3(QList<SubtitleEntry> &entries, const QStringList &cutList);

  signals:
    void close();
};

#endif /* CUTTER_H_ */

