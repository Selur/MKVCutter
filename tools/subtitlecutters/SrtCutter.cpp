/*
 * SrtCutter.cpp
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#include "SrtCutter.h"

#include <iostream>
using namespace std;

SrtCutter::SrtCutter(QObject *parent, bool debug) :
    Cutter(parent, debug)
{
  this->setObjectName("SrtCutter");
  m_acceptedExtensions << "srt";
}

SrtCutter::~SrtCutter()
{
}

QList<SubtitleEntry> SrtCutter::tokenize(const QString content)
{
  if (m_debug) {
    cout << " tokenizing subtitle content,.." << endl;
  }
  QList<SubtitleEntry> tokens;
  QStringList lines = content.split("\n");

  bool gotLineNumber = false;
  bool gotFromTo = false;
  bool gotContent = false;
  SubtitleEntry currentEntry;
  QStringList fromTo;
  foreach(QString line, lines)
  {
    line = line.trimmed();
    if (line.isEmpty()) {
      if (!gotLineNumber || !gotContent || !gotFromTo) {
        gotLineNumber = false;
        gotFromTo = false;
        gotContent = false;
        currentEntry.from = -1;
        currentEntry.to = -1;
        currentEntry.text = QString();
        continue;
      }
      currentEntry.fixed = false;
      tokens.append(currentEntry);
      gotLineNumber = false;
      gotFromTo = false;
      gotContent = false;
      currentEntry.from = -1;
      currentEntry.to = -1;
      currentEntry.text = QString();
      continue;
    }
    if (line.toInt() != 0 && !gotLineNumber) {
      gotLineNumber = true;
      continue;
    }
    if (line.contains(" --> ") && !gotFromTo) {
      fromTo = line.split(" --> ");
      currentEntry.from = this->timeToSeconds(fromTo.at(0));
      currentEntry.to = this->timeToSeconds(fromTo.at(1));
      gotFromTo = true;
      continue;
    }
    if (!currentEntry.text.isEmpty()) {
      currentEntry.text += "\n";
    }
    currentEntry.text += line;
    gotContent = true;
  }
  if (gotLineNumber && gotContent && gotFromTo) {
    tokens.append(currentEntry);
  }
  return tokens;
}

QString SrtCutter::cutContent(const QString &content, const QStringList &cutList)
{
  QList<SubtitleEntry> tokens = this->tokenize(content);
  if (m_debug) {
    cout << " -> " << qPrintable(this->objectName()) << " found " << tokens.count();
    cout << " subtitle entries" << endl;
  }
  this->adjustEntries(tokens, cutList);

  QString subtitles, tmp;
  int i = 1;
  foreach(SubtitleEntry entry, tokens)
  {
    subtitles += QString::number(i++);
    subtitles += "\r\n";
    tmp = secondsToHMSZZZ(entry.from);
    if (tmp.contains(".")) {
        tmp = tmp.replace(".", ",");
    } else {
        tmp += ",000";
    }
    subtitles += tmp.trimmed();
    subtitles += " ---> ";
    tmp = secondsToHMSZZZ(entry.to);
    if (tmp.contains(".")) {
      tmp = tmp.replace(".", ",");
    } else {
      tmp += ",000";
    }
    subtitles += tmp.trimmed();
    subtitles += "\r\n";
    subtitles += entry.text.trimmed();
    subtitles += "\r\n";
    subtitles += "\r\n";
  }
  return subtitles;
}
