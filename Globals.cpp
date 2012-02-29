/*
 * Globals.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: Selur
 */

#include "Globals.h"

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

QString Globals::intSetToTimes(QSet<int> keyframes, double fps)
{
  QList<int> list = keyframes.toList();
  qSort(list);
  QString keyString;
  foreach(int key, list) {
    keyString += secondsToHMS(key * fps) + ",";
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

int Globals::saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
    return -1;
  }
  QFile file(to);
  file.remove();
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForUtfText(text.toUtf8()));
    out << text;
    if (file.exists()) {
      file.close();
      return 0;
    }
  }
  return -1;
}

