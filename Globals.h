#ifndef GLOBALS_H
#define GLOBALS_H

#include <QDir>
#include <QSet>
#include <QHash>
#include <QList>
#include <QTime>
#include <QTextStream>
#include <QStringConverter>

struct cutTyp
{
    int start, end;
};
struct cutTyp1
{
    int prevKey, nextKey;
    cutTyp cut;
};

namespace Globals
{

  // Laufzeit einer Matroska-Datei in Millisekunden, oder -1, wenn sie sich nicht
  // ermitteln laesst. Fragt mkvmerge synchron per '-J' -- der Aufruf ist ein reines
  // Identify und braucht keine nennenswerte Zeit.
  double mkvDurationInMs(const QString &file);
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
  QString frameToTime(const int& number, const double& fps, const QStringList* inputTimeCodeList);
  QString secondsToHMSZZZ(double seconds);
  QString shortFileName(QString inputFile);
  QString getDirectory(const QString input);
  QString readAll(const QString fileName, QString type);
  // Achtung: der erste Parameter ist der Inhalt, nicht ein Dateiname.
  int saveTextTo(QString text, QString to);
  QString optionsToJson(const QStringList &options);
  double timeToSeconds(QString value);
  double timeToSeconds(QTime time);

  extern QHash<QString, double> fractionToDecimal;
  extern QHash<QString, QString> decimalToFraction;
  double fractionToDecimalConvert(QString fraction);
  QString decimalToFractionConvert(const double decimal);
  void initDecimalFractionHashs();

}
#endif // GLOBALS_H
