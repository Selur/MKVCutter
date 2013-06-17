#ifndef GLOBALS_H
#define GLOBALS_H

#include <QDir>
#include <QSet>
#include <QList>
#include <QTime>
#include <QTextStream>
#include <QTextCodec>

struct cutTyp
{
    int start, end;
};
struct cutTyp1
{
    int prevKey, nextKey;
    cutTyp cut;
};
struct cutTyp2
{
    int prevKey, nextKey;
    QList<cutTyp> cuts;
};

namespace Globals
{

  QString cutTypToString(cutTyp cut);
  QString cutTyp1ToString(cutTyp1 cut);
  QString cutTyp1ListToString(QList<cutTyp> elems);
  QString intSetToString(QSet<int> keyframes);
  QString secondsToHMS(double seconds);
  QString milliSecondsToHMS(int milli);
  QString intSetToTimes(QSet<int> keyframes, double fps);
  QString removeQuotes(QString input);
  QString getWholeFileName(const QString input);
  QString getFileName(const QString input);
  QString removeLastSeparatorFromPath(QString input);
  QString frameToTime(int number, double fps);
  QString secondsToHMSZZZ(double seconds);
  QString shortFileName(QString inputFile);
  QString getDirectory(const QString input);
  QString readAll(const QString fileName, QString type);
  int saveTextTo(QString fileName, QString to);
  double timeToSeconds(QString value);
  double timeToSeconds(QTime time);
}
#endif // GLOBALS_H
