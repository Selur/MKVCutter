/*
 * Globals.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: Selur
 */

#include "Globals.h"
#include "Windows.h"

QHash<QString, double> Globals::fractionToDecimal = QHash<QString, double>();
QHash<QString, QString> Globals::decimalToFraction = QHash<QString, QString>();

QString Globals::cutTypToString(cutTyp cut)
{
  return QString::number(cut.start) + "-" + QString::number(cut.end);
}

QString Globals::cutTyp1ToString(cutTyp1 cut)
{
  return QString::number(cut.prevKey) + " " + Globals::cutTypToString(cut.cut) + " "
      + QString::number(cut.nextKey);
}

QString Globals::cutTyp1ListToString(QList<cutTyp> elems)
{
  QString cutString;
  foreach (cutTyp tCut, elems) {
    cutString += QString::number(tCut.start) + "-" + QString::number(tCut.end) + ",";
  }
  if (!elems.isEmpty()) {
    cutString.remove(cutString.size() - 1, 1);
  }
  return cutString;
}

QString Globals::intSetToString(QSet<int> keyframes)
{
  QList<int> list = keyframes.toList();
  qSort(list);
  QString keyString;
  foreach(int key, list) {
    keyString += QString::number(key) + ", ";
  }
  if (!list.isEmpty()) {
    keyString.remove(keyString.size() - 2, 2);
  }
  return keyString;
}

QString Globals::milliSecondsToHMS(int milli)
{
  if (milli < 0) {
    return "00:00:00";
  }
  QString ret;
  int hrs = milli / 3600000;
  if (hrs > 24) {
    hrs %= 25;
  }
  if (hrs == 0) {
    ret = "00";
  } else if (hrs < 10) {
    ret = "0" + QString::number(hrs);
  } else {
    ret = QString::number(hrs);
  }
  ret += ":";
  milli -= hrs * 3600000;

  int min = milli / 60000;
  if (min == 0) {
    ret += "00";
  } else if (min < 10) {
    ret += "0" + QString::number(min);
  } else {
    ret += QString::number(min);
  }
  ret += ":";
  milli -= min * 60000;

  int sec = milli / 1000;
  if (sec == 0) {
    ret += "00";
  } else if (sec < 10) {
    ret += "0" + QString::number(sec);
  } else {
    ret += QString::number(sec);
  }
  milli -= sec * 1000;
  QString smilli = QString::number(milli);
  while (smilli.size() < 3) {
    smilli = "0" + smilli;
  }
  ret += "." + smilli;
  return ret;
}

QString Globals::secondsToHMSZZZ(double seconds)
{
  if (seconds == 0) {
    return "00:00:00";
  }
  QString time = QString();
  int hrs = 0;
  if (seconds >= 3600) { //Stunden
    hrs = int(seconds) / 3600;
  } else if (seconds == 3600) {
    hrs = 1;
  }
  time += QString((hrs < 10) ? "0" : QString()) + QString::number(hrs);
  int min = 0;
  seconds = seconds - 3600 * hrs;
  if (seconds >= 60) { //Minuten
    min = int(seconds) / 60;
  }
  time += ":" + QString((min < 10) ? "0" : QString()) + QString::number(min);
  int sec = 0;
  seconds = seconds - 60 * min;
  if (seconds > 0) { //Sekunden
    sec = int(seconds);
  }
  time += ":";
  time += QString((sec < 10) ? "0" : QString());
  time += QString::number(sec);
  int milliseconds = int(1000 * (seconds - (sec * 1.0)));
  if (milliseconds == 0) {
    time += ".000";
    return time;
  }
  time += ".";
  if (milliseconds < 10) {
    time += "00";
  } else if (milliseconds < 100) {
    time += "0";
  }
  time += QString::number(milliseconds);
  return time;
}

QString Globals::secondsToHMS(double seconds)
{
  if (seconds == 0) {
    return "00:00:00";
  }
  QString time = QString();
  int hrs = 0;
  if (seconds >= 3600) { //Stunden
    hrs = int(seconds) / 3600;
  } else if (seconds == 3600) {
    hrs = 1;
  }
  time += QString((hrs < 10) ? "0" : QString()) + QString::number(hrs);

  int min = 0;
  seconds = seconds - 3600 * hrs;
  if (seconds >= 60) { //Minuten
    min = int(seconds) / 60;
  }
  time += ":" + QString((min < 10) ? "0" : QString()) + QString::number(min);

  int sec = 0;
  seconds = seconds - 60 * min;
  if (seconds > 0) { //Sekunden
    sec = int(seconds);
  }
  time += ":";
  time += QString((sec < 10) ? "0" : QString());
  time += QString::number(sec);
  return time;
}
//#include <iostream>
QString Globals::intSetToTimes(QSet<int> keyframes, double fps)
{
  QList<int> list = keyframes.toList();
  qSort(list);
  QString keyString;
  foreach(int key, list) {
    //std::cerr << "key: " << key << ", fps: " << qPrintable(QString::number(fps)) << std::endl;
    keyString += milliSecondsToHMS(int(key / fps * 1000.0 + 0.5)) + ",";
  }
  if (!list.isEmpty()) {
    keyString.remove(keyString.size() - 2, 2);
  }
  return keyString;
}

QString Globals::removeQuotes(QString input)
{
  QString ret = input.trimmed();
  if (ret.startsWith("\"")) {
    ret = ret.remove(0, 1);
  }
  if (ret.endsWith("\"")) {
    ret = ret.remove(ret.size() - 1, 1);
  }
  return ret;
}

QString Globals::getWholeFileName(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString output = QDir::toNativeSeparators(input);
  int index = output.lastIndexOf(QDir::separator());
  if (output.endsWith(QDir::separator())) {
    return QString();
  } else if (index != -1) {
    output = output.remove(0, index + 1);
  }
  output = removeQuotes(output);
  return QDir::toNativeSeparators(output);
}

QString Globals::getFileName(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString output = getWholeFileName(input);
  int index = output.lastIndexOf(".");
  if (index != -1) {
    output = output.remove(index, output.size());
  }
  return output;
}
QString Globals::removeLastSeparatorFromPath(QString input)
{
  input = input.trimmed();
  if (input.isEmpty()) {
    return input;
  }
  input = QDir::toNativeSeparators(input);
  int size = input.size();
  if (!input.endsWith(QDir::separator())) {
    return input;
  } else if (size == 1) { //input only consists of the separator
    return QString();
  }
  return input.remove(size - 1, 1);
}

QString Globals::getDirectory(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString path = input;
  QFileInfo info(path);
  if (info.isDir()) {
    return removeLastSeparatorFromPath(path);
  }
  QString output = path;
  output = output.replace("\\", "/");
  int index = output.lastIndexOf("/");
  if (index == -1) {
    return QString();
  }
  output = output.remove(index, output.size());
  return QDir::toNativeSeparators(output);
}

#define UTF8BOM "\xEF\xBB\xBF"
#define UTF8 "UTF-8"
#define UTF16LEBOM "\xFF\xFE"
#define UTF16LE "UTF-16LE"
#define UTF16BEBOM "\xFE\xFF"
#define UTF16BE "UTF-16BE"


QString Globals::readAll(const QString fileName, QString type)
{
  QString input = removeQuotes(fileName);
  if (input.startsWith("./") || input.startsWith(".\\")) {
    input = input.remove(0, 1);
    input = QDir::toNativeSeparators(QDir::currentPath() + input);
  }
  QFile file(fileName);
  if (!file.exists()) {
    return QString();
  }
  if (!file.open(QIODevice::ReadOnly)) {
    return QString();
  }
  QTextStream stream(&file);
  if (type == "auto") {
    stream.autoDetectUnicode();
  } else {
    stream.setCodec(type.toUtf8());
  }
  input = stream.readAll();
  file.close();
  return input;
}

/**
 * saves the content of a QString 'text' into a file 'to'
 * 0 -> no problem
 * -1 -> saving failed
 **/
int Globals::saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
    return -1;
  }
  text = text.replace("\r\n", "\n");
  QFile file(to);
  file.remove();
  if (file.open(QIODevice::WriteOnly)) {
    bool ttxt = to.endsWith(".ttxt", Qt::CaseInsensitive);
    bool avs =  to.endsWith(".avs", Qt::CaseInsensitive);
    bool meta =  to.endsWith(".meta", Qt::CaseInsensitive);
    if (!ttxt && !avs && !meta && !to.endsWith(".cut")) {
      file.write(UTF8BOM);
      file.setTextModeEnabled(true);
    }
    QTextStream out(&file);
    if (ttxt) {
       out.setAutoDetectUnicode(true);
    } else if (avs || meta) {
      out.setCodec(QTextCodec::codecForLocale());
    } else {
      out.setCodec("UTF-8");
    }
    out << text;
    if (file.exists()) {
      file.close();
      return 0;
    }
  }
  return -1;
}
#include <iostream>
QString Globals::frameToTime(const int& number, const double& fps, const QStringList* inputTimeCodeList)
{
  if (inputTimeCodeList->isEmpty()) {
    double seconds = int(number / fps * 1000 + 0.5)/1000.0;
    return secondsToHMSZZZ(seconds);
  }
  double offset = inputTimeCodeList->at(1).toDouble();
  return secondsToHMSZZZ((inputTimeCodeList->at(number).toDouble()-offset)/1000.0);
}

double Globals::timeToSeconds(QTime time)
{
  /*sendMessage(HHELPER, "hour", time.hour());
   sendMessage(HHELPER, "minute", time.minute());
   sendMessage(HHELPER, "second", time.second());
   sendMessage(HHELPER, "milli second", time.msec());*/
  return ((time.hour() * 60.0 + time.minute()) * 60.0 + time.second()) + (time.msec() / 1000.0);
}

double Globals::timeToSeconds(QString value)
{
  QString tmp = value.trimmed();
  double dtime = tmp.toDouble();
  int index = tmp.indexOf(":");
  if (index == -1) {
    return dtime;
  }
  int colonCount = tmp.count(":");
  if (index == 1) {
    tmp = "0" + tmp;
  }
  index = tmp.indexOf(".");
  if (index == -1) {
    QTime time;
    switch (colonCount) {
      case 1 :
        time = QTime::fromString(tmp, "mm:ss");
        break;
      case 2 :
        time = QTime::fromString(tmp, "hh:mm:ss");
        break;
      case 3 :
        return timeToSeconds(QTime::fromString(tmp, "hh:mm:ss:zzz"));
        break;
    }
    return (time.hour() * 60.0 + time.minute()) * 60.0 + time.second();
  }
  int msCount = tmp.length() - index - 1;
  QString ms;
  switch (msCount) {
    case 1 :
      ms = ".z";
      break;
    case 2 :
      ms = ".zzz";
      tmp += "0";
      break;
    case 3 :
      ms = ".zzz";
      break;
    default :

      break;
  }

  QString from;
  switch (colonCount) {
    case 1 :
      from = "mm:ss" + ms;
      break;
    case 2 :
      from = "hh:mm:ss" + ms;
      break;
  }
  if (!from.isEmpty()) {
    dtime = timeToSeconds(QTime::fromString(tmp, from));
  } else {
    dtime = 0;
  }
  return dtime;
}

QString Globals::shortFileName(QString inputFile)
{
  if (inputFile.isEmpty()) {
    return QString();
  }
  inputFile = removeQuotes(inputFile);
  inputFile = QDir::toNativeSeparators(inputFile);
  if (QFile::exists(inputFile)) {
    wchar_t* input = new wchar_t[inputFile.size() + 1];
    inputFile.toWCharArray(input);
    input[inputFile.size()] = L'\0';
    long length = GetShortPathName(input, nullptr, 0);
    wchar_t* output = new wchar_t[length];
    GetShortPathName(input, output, length);
    inputFile = QString::fromWCharArray(output, length - 1);
    delete[] input;
    delete[] output;
  } else {
      inputFile = inputFile+" DOES NOT EXIST!";
  }
  return inputFile;
}


/**
 * initializes search lists
 */
void Globals::initDecimalFractionHashs()
{
  QList<double> fractionParts;
  fractionParts << 1000.0 << 1001.0 << 11988.0 << 12000.0 << 14986.0 << 15000.0;
  fractionParts << 23976.0 << 24000.0 << 25000.0 << 29970.0 << 30000.0 << 59940.0;
  fractionParts << 60000.0 << 120000.0 << 240000.0;


  int i, j, count = fractionParts.count();
  QString label;
  double value;
  for (i = 0; i < count; ++i) {
    for (j = 0; j < count; ++j) {
      if (i == j) {
        continue;
      }
      value = fractionParts.at(i) / fractionParts.at(j);
      label = QString::number(int(fractionParts.at(i))) + "/" + QString::number(int(fractionParts.at(j)));
      Globals::decimalToFraction.insert(QString::number(value), label);
      Globals::fractionToDecimal.insert(label, value);
    }
  }
}

/**
 * converts a decimal into a fraction
 */
QString Globals::decimalToFractionConvert(const double decimal)
{
  if (decimal < 0.0001 || qAbs(decimal - 1) < 0.0001) { // difference is too small
    return "1/1";
  }

  QString current, ret;
  QStringList decimals = Globals::decimalToFraction.keys();
  double precision = 100000.0, myDec = int(decimal * precision + 0.5) / precision, rCurrent;
  double tolerance = 1000.0;
  double bestDiff = 1000, diff = 1000;
  for (int i = 0, c = decimals.count(); i < c; ++i) { // find best match
    current = decimals.at(i);
    rCurrent = int(current.toDouble() * precision + 0.5) / precision;
    //sendMessage(HHELPER, QObject::tr("comparing (%1 vs %2 - %4) = %3 < %5").arg(myDec).arg(rCurrent).arg(qAbs(rCurrent - myDec)).arg(StaticHelper::decimalToFraction.value(current)).arg(1 / precision), qAbs(rCurrent - myDec) <= (1 / tolerance));
    diff = qAbs(rCurrent - myDec);
    if ((diff  <= (1 / tolerance)) && diff < bestDiff) { // tolerance check
      ret = Globals::decimalToFraction.value(current);
      bestDiff = diff;
      continue;
    }
    diff = qAbs(rCurrent - decimal);
    if ((diff <= (1 / tolerance)) && diff < bestDiff) { // tolerance check
      ret = Globals::decimalToFraction.value(current);
      bestDiff = diff;
      continue;
    }
  }
  if (ret == QString()) { // not match found
    QString frac = QString::number(int(myDec * precision + 0.5)) + "/" + QString::number(precision);
    return frac;
  }
  decimals = ret.split("/");
  if (decimals.at(0) == decimals.at(1)) { // match was x/x
    return "1/1";
  }
  return ret; // return found value
}


/**
 * converts a fraction into a decimal
 */
double Globals::fractionToDecimalConvert(QString fraction)
{
  fraction = fraction.replace(":", "/");
  if (fraction == "1/1" || fraction.isEmpty() || fraction == QObject::tr("?") || fraction.isEmpty()) {
    return 1;
  }
  if (fraction.endsWith("/1000")) {
    QStringList elems = fraction.split("/");
    return elems.at(0).toDouble() / elems.at(1).toDouble();
  }
  double decimal = Globals::fractionToDecimal.value(fraction);
  if (decimal == 0 && fraction.contains("/")) { // unknown
    QStringList elems = fraction.split("/");
    return elems.at(0).toDouble() / elems.at(1).toDouble();
  }
  if (!fraction.contains("/")) {
    decimal = fraction.toDouble();
    if (decimal == 0) {
      decimal = 1;
    }
  }
  return decimal;
}
