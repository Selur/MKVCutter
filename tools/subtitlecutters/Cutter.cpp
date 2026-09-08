/*
 * Cutter.cpp
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#include "Cutter.h"
#include <QDir>
#include <QFile>
#include <QStringConverter>
#include <QTextStream>

#define UTF8BOM "\xEF\xBB\xBF"
#define UTF8 "UTF-8"
#define UTF16LEBOM "\xFF\xFE"
#define UTF16LE "UTF-16LE"
#define UTF16BEBOM "\xFE\xFF"
#define UTF16BE "UTF-16BE"

#include <iostream>
using namespace std;

Cutter::Cutter(QObject *parent, bool debug)
    : QObject(parent), m_input(QString()), m_output(QString()), m_acceptedExtensions(),
        m_debug(debug)
{
}

Cutter::~Cutter()
{
}

int Cutter::saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
    cerr << "Save text called with empty text!" << endl;
    return -1;
  }
  if (m_debug) {
    cout << " saving subtitles to: " << qPrintable(to) << endl;
  }
  QFile file(to);
  file.remove();
  if (!file.open(QIODevice::WriteOnly)) {
    cerr << "Couldn't open file for writing!" << endl;
    return -1;
  }

  bool avs = to.endsWith(".avs", Qt::CaseInsensitive);
  bool idx = to.endsWith(".idx", Qt::CaseInsensitive);
  bool d2v = to.endsWith(".d2v", Qt::CaseInsensitive);
  bool qp = to.endsWith(".qp", Qt::CaseInsensitive);
  bool meta = to.endsWith(".meta", Qt::CaseInsensitive);
  text = text.replace("\r\n", "\n");
#ifdef Q_OS_WIN
  bool ttxt = to.endsWith(".ttxt", Qt::CaseInsensitive);
  if (!ttxt && !avs && !meta && !qp && !d2v) {
#else
  if (!avs && !meta && !qp && !d2v) {
#endif
    file.write(UTF8BOM);
    file.setTextModeEnabled(true);
  }
  QTextStream out(&file);
  if (avs || meta || idx || qp || d2v) {
    out.setEncoding(QStringConverter::System);
  } else {
    out.setEncoding(QStringConverter::Utf8);
  }
  out << text;
  if (file.exists()) {
    file.close();
    return 0;
  } else {
    cerr << "Couldn't save file content!" << endl;
  }
  return -1;
}

void Cutter::cut(const QString input, QString &output, const QStringList cutList)
{
  m_output = output;
  QString extension = input;
  extension = extension.trimmed();
  extension = extension.remove(0, extension.lastIndexOf(".") + 1);
  extension = extension.toLower();
  if (m_debug) {
    cout << "  input file extension " << qPrintable(extension) << endl;
  }
  bool supported = m_acceptedExtensions.contains(extension);
  if (!supported) {
    cerr << " -> " << qPrintable(this->objectName());
    cerr << " does not support " << qPrintable(input) << endl;
    output = QString();
    emit close();
    return;
  }
  if (m_debug) {
    cout << " -> " << qPrintable(this->objectName());
    cout << " used for input " << qPrintable(input) << endl;
  }
  QString content = this->readAll(input).trimmed();
  if (content.isEmpty()) {
    cerr << " input " << qPrintable(input) << " is empty!" << endl;
    output = QString();
    emit close();
    return;
  }
  QString cutContent = this->cutContent(content, cutList);
  if (cutContent.isEmpty()) {
    cerr << " cut content is empty!" << endl;
    output = QString();
    emit close();
    return;
  }
  if (this->saveTextTo(cutContent, output) != 0) {
    output = QString();
  }
  if (m_debug) {
    cout << "finished!";
  }
  emit close();
  return;
}

QString Cutter::removeLastSeparatorFromPath(QString input)
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

QString Cutter::getDirectory(const QString input)
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

QString Cutter::readAll(const QString fileName, QString type)
{
  Q_UNUSED(type);
  QString input = fileName;
  QFile file(input);
  if (!file.exists()) {
    return QString();
  }
  if (!file.open(QIODevice::ReadOnly)) {
    return QString();
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::encodingForName(type.toUtf8()).value_or(QStringConverter::Utf8));
  input = stream.readAll();
  file.close();
  return input;
}

/**
 * hh:mm:ss.mmm -- Grundlage der SRT-Zeitstempel (SrtCutter ersetzt den Punkt durch ein Komma).
 * In ganzen Millisekunden rechnen: die Zerlegung ueber Ganzzahlen erledigt den Uebertrag,
 * ein auf 1000 ms gerundeter Rest wandert also in die naechste Sekunde statt als ".1000"
 * auszukommen. Die Nullzeit lieferte frueher "00:00:00.00" -- zwei Stellen statt drei, im
 * SRT also ein ungueltiges "00:00:00,00".
 **/
QString Cutter::secondsToHMSZZZ(double seconds)
{
  if (seconds < 0) {
    seconds = 0;
  }
  qint64 total = qint64(seconds * 1000.0 + 0.5);
  const qint64 milliseconds = total % 1000;
  total /= 1000;
  const qint64 sec = total % 60;
  total /= 60;
  const qint64 min = total % 60;
  const qint64 hrs = total / 60;
  return QString("%1:%2:%3.%4")
      .arg(hrs, 2, 10, QLatin1Char('0'))
      .arg(min, 2, 10, QLatin1Char('0'))
      .arg(sec, 2, 10, QLatin1Char('0'))
      .arg(milliseconds, 3, 10, QLatin1Char('0'));
}

/**
 * h:mm:ss.cc fuer ASS/SSA -- dort ist die Stunde einstellig, das bleibt so.
 * Gleiches Uebertragsproblem wie oben, nur in Hundertstelsekunden: ein auf 100 gerundeter
 * Rest ergab frueher ".100" statt einer Sekunde mehr.
 **/
QString Cutter::secondsToHMSZZ(double seconds)
{
  if (seconds < 0) {
    seconds = 0;
  }
  qint64 total = qint64(seconds * 100.0 + 0.5);
  const qint64 hundredths = total % 100;
  total /= 100;
  const qint64 sec = total % 60;
  total /= 60;
  const qint64 min = total % 60;
  const qint64 hrs = total / 60;
  return QString("%1:%2:%3.%4")
      .arg(hrs)
      .arg(min, 2, 10, QLatin1Char('0'))
      .arg(sec, 2, 10, QLatin1Char('0'))
      .arg(hundredths, 2, 10, QLatin1Char('0'));
}

double Cutter::timeToSeconds(QTime time)
{
  return ((time.hour() * 60.0 + time.minute()) * 60.0 + time.second()) + (time.msec() / 1000.0);
}

/**
 * converts a time string to seconds
 **/
double Cutter::timeToSeconds(QString value)
{
  QString tmp = value.trimmed();
  if (tmp.startsWith("0:")) {
    tmp = "0" + tmp;
  }
  double dtime = tmp.toDouble();
  int index = tmp.indexOf(":");
  bool tripple = tmp.count(":") == 3;
  if (index == -1) {
    if (tmp.contains(".") || tmp.toInt() != 0) {
      return tmp.toDouble();
    }
    return 0;
  }
  if (index == 1) {
    tmp = "0" + tmp;
  }
  bool dot = true;
  index = tmp.indexOf(".");
  if (index == -1) {
    index = tmp.indexOf(",");
    dot = false;
  }
  if (index != -1) {
    int length = tmp.length();
    QTime time;
    if ((length - index) == 2) {
      if (tripple) {
        time = QTime::fromString(tmp, "hh:mm:ss:z");
      } else if (dot) {
        time = QTime::fromString(tmp, "hh:mm:ss.z");
      } else {
        time = QTime::fromString(tmp, "hh:mm:ss,z");
      }
    } else {
      if ((length - index) == 3) {
        tmp += "0";
      }
      if (tripple) {
        time = QTime::fromString(tmp, "hh:mm:ss:zzz");
      } else if (dot) {
        time = QTime::fromString(tmp, "hh:mm:ss.zzz");
      } else {
        time = QTime::fromString(tmp, "hh:mm:ss,zzz");
      }
    }
    dtime = Cutter::timeToSeconds(time);
  } else {
    QTime time;
    if (dot) {
      time = QTime::fromString(tmp, "hh:mm:ss");
    } else {
      time = QTime::fromString(tmp, "hh:mm:ss");
    }
    dtime = (time.hour() * 60.0 + time.minute()) * 60.0 + time.second();
  }
  return dtime;
}

void setCutStartEnd(QString cut, double &cStart, double &cEnd, const bool &debug)
{
  QStringList cutElements = cut.split("-");
  cStart = Cutter::timeToSeconds(cutElements.at(0));
  cEnd = Cutter::timeToSeconds(cutElements.at(1));
  if (debug) {
    cout << "  looking at cut " << cStart << "-" << cEnd << endl;
  }
}

struct Cut
{
    double from;
    double to;
};

QList<Cut> converCutTimes(QList<Cut> &cutList)
{
  QString time;
  Cut cut, oldCut;
  double lastEnd = 0;
  QList<Cut> newTimes;
  for (int i = 0, c = cutList.count(); i < c; ++i) { //check cuts to set newEntry from&to
    oldCut = cutList.at(i);
    cut.from = lastEnd;
    cut.to = cut.from + oldCut.to - oldCut.from;
    lastEnd = cut.to;
    newTimes.append(cut);
  }
  return newTimes;
}

QList<Cut> convertCutListToListCut(const QStringList &cutList, const bool &debug)
{
  Cut cut;
  double cStart, cEnd;
  QList<Cut> newTimes;
  for (int i = 0, c = cutList.count(); i < c; ++i) { //check cuts to set newEntry from&to
    setCutStartEnd(cutList.at(i), cStart, cEnd, debug);
    cut.from = cStart;
    cut.to = cEnd;
    newTimes.append(cut);
  }
  return newTimes;
}

void outputCuts(const QList<Cut> &cuts)
{
  foreach(Cut cut, cuts)
  {
    cout << " cut: " << cut.from << "-" << cut.to << endl;
  }
}

void addNewEntries(const SubtitleEntry &oldEntry, const QList<Cut> &cuts, const QList<Cut> &newCuts,
    const int &fromIndex, const int &toIndex, const int &fromBefore, const int &toAfter,
    QList<SubtitleEntry> &adjustedEntries, double &lastEnd, double &previousTo, const bool &debug)
{
  if (debug) {
    cout << " -> fromIndex: " << fromIndex << ", toIndex: " << toIndex << ", fromBefore: "
        << fromBefore << ", toAfter: " << toAfter << endl;
  }
  SubtitleEntry newEntry = oldEntry;
  if (fromIndex != -1) { // entry inside cut number fromIndex
    if (debug) {
      cout << " newEntry.from = qMax(lastEnd[" << lastEnd << "], newCuts.at(fromIndex).from["
          << newCuts.at(fromIndex).from << "]) + oldEntry.from[" << oldEntry.from
          << "] - qMax(previousTo[" << previousTo << "], cuts.at(fromIndex).from)["
          << cuts.at(fromIndex).from << "])" << endl;
    }
    newEntry.from = qMax(lastEnd, newCuts.at(fromIndex).from) + oldEntry.from
        - qMax(previousTo, cuts.at(fromIndex).from);
    if (debug) {
      cout << " -> newEntry.from: " << newEntry.from << endl;
    }
  } else {
    newEntry.from = newCuts.at(fromBefore).from;
    if (debug) {
      cout << " -> newEntry.from = newCuts.at(fromBefore).from: " << newEntry.from << endl;
    }
  }
  if (toIndex != -1) { // entry inside cut number fromIndex
    if (debug) {
      cout << " qMax(newEntry.from[" << newEntry.from << "], newCuts.at(toIndex).from["
          << newCuts.at(toIndex).from << "]) + oldEntry.to[" << oldEntry.to
          << "] -  qMax(oldEntry.from[" << oldEntry.from << "], cuts.at(toIndex).from["
          << cuts.at(toIndex).from << "])" << endl;
    }
    newEntry.to = qMax(newEntry.from, newCuts.at(toIndex).from) + oldEntry.to
        - qMax(oldEntry.from, cuts.at(toIndex).from);
  } else {
    int after = toAfter;
    if (after >= newCuts.count()) {
      after = newCuts.count() - 1;
    }
    newEntry.to = newCuts.at(after).to;
    if (debug) {
      cout << " -> newEntry.to = newCuts.at(after).to: " << newEntry.to << endl;
    }
  }
  lastEnd = newEntry.to;
  previousTo = oldEntry.to;
  if (newEntry.to - newEntry.from < 0.4) {
    if (debug) {
      cout << " -> ignored entry " << newEntry.from << "-" << newEntry.to
          << " since it's displayed to short" << endl;
    }
    return;
  }
  if (debug) {
    cout << " -> new entry " << newEntry.from << "-" << newEntry.to << endl;
  }
  adjustedEntries.append(newEntry);
}

void setCutIndicesForEntry(int &fromIndex, int &toIndex, int &fromBefore, int &toAfter,
    const QList<Cut> &cuts, bool &finished, const SubtitleEntry &entry, const bool &debug)
{
  Cut cut;
  fromIndex = -1, toIndex = -1;
  fromBefore = -1, toAfter = -1;
  if (debug) {
    cout << " looking at entry " << entry.from << "-" << entry.to << endl;
  }
  for (int i = 0, c = cuts.count(); i < c; ++i) { //check cuts to set newEntry from&to
    cut = cuts.at(i);
    if (fromIndex == -1 && entry.from < cut.from) {
      fromBefore = i;
    }
    if (entry.to < cut.from) {
      if (i == 0) {
        if (debug) {
          cout << "  -> drop, since entry lies before first cut" << endl;
        }
        break;  // check next entry
      }
      if (debug) {
        cout << "  -> check next cut" << endl;
      }
      continue; // check next cut
    }
    if (toIndex == -1 && entry.to > cut.to) {
      toAfter = i;
    }
    if (i + 1 == c) { // last
      if (entry.from >= cut.to) {
        if (debug) {
          cout << "  -> finishing, entry starts after last cut" << endl;
        }
        finished = true;
        break;
      }
    }
    if (entry.from >= cut.from && entry.from <= cut.to) { // from inside cut
      fromIndex = i;
    }
    if (entry.to >= cut.from && entry.to <= cut.to) { // to inside cut
      toIndex = i;
      break;
    }
  }
}

void Cutter::adjustTimingsofEntries(QList<SubtitleEntry> &entries, const QStringList &cutList)
{
  SubtitleEntry entry;
  QList<Cut> cuts = convertCutListToListCut(cutList, m_debug);
  if (m_debug) {
    cout << "CUTS" << endl;
    outputCuts(cuts);
  }
  QList<Cut> newCuts = converCutTimes(cuts);
  if (m_debug) {
    cout << "NEW CUTS" << endl;
    outputCuts(newCuts);
  }
  QList<SubtitleEntry> adjustedEntries;
  double lastEnd = 0, previousTo = 0;
  bool finished = false;
  int fromIndex, toIndex, fromBefore, toAfter;
  for (int j = 0, d = entries.count(); j < d; ++j) {
    entry = entries.at(j);
    setCutIndicesForEntry(fromIndex, toIndex, fromBefore, toAfter, cuts, finished, entry, m_debug);
    if (fromIndex == -1 && toIndex == -1 && (fromBefore >= toAfter || fromBefore == -1)) {
      if (m_debug) {
        cout << " -> dropped" << endl;
      }
    } else {
      addNewEntries(entry, cuts, newCuts, fromIndex, toIndex, fromBefore, toAfter, adjustedEntries,
          lastEnd, previousTo, m_debug);
    }
    if (finished) {
      break;
    }
  }
  entries = adjustedEntries;
}

void outputEntries(const QList<SubtitleEntry> &entries)
{
  cout << "Entries:" << endl;
  foreach(SubtitleEntry entry, entries)
  {
    cout << " entry " << entry.from << "-" << entry.to << " = " << qPrintable(entry.text)
        << ", length: " << entry.to - entry.from << endl;
  }
}

void Cutter::adjustEntries(QList<SubtitleEntry> &entries, const QStringList &cutList)
{
  if (entries.isEmpty()) {
    cerr << "No entries to adjust!" << endl;
    return;
  }
  if (m_debug) {
    cout << " adjusting subtitle entries to cut list,.." << endl;
  }
  adjustTimingsofEntries(entries, cutList);
  if (m_debug) {
    outputEntries(entries);
  }
}
