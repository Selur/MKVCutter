/*
 * AssCutter.cpp
 *
 *  Created on: 06.07.2013
 *      Author: Selur
 */

#include "AssCutter.h"

#include <iostream>
using namespace std;

AssCutter::AssCutter(QObject *parent, bool debug) :
    Cutter(parent, debug)
{
  this->setObjectName("AssCutter");
  m_acceptedExtensions << "ass" << "ssa";
}

AssCutter::~AssCutter()
{
}

QList<SubtitleEntry> AssCutter::tokenize(const QString dialog)
{
  QList<SubtitleEntry> entries;
  QStringList lines = dialog.split("\n");
  QStringList elements;
  SubtitleEntry entry;
  //Dialogue: 0,0:00:01.77,0:00:05.93,Default,,0000,0000,0000,,Es ist vollendet: Unser eigenes Valkyria Chronicles!
  foreach(QString line, lines)
  {
    if (!line.startsWith("Dialogue: ")) {
      if (m_debug) {
        cout << " ignored: " << qPrintable(line) << endl;
      }
      continue;
    }
    line = line.remove(0, 10);
    //0,0:00:01.77,0:00:05.93,Default,,0000,0000,0000,,Es ist vollendet: Unser eigenes Valkyria Chronicles!
    elements = line.split(",");
    entry.from = this->timeToSeconds(elements.takeAt(1));
    entry.to = this->timeToSeconds(elements.takeAt(2));
    elements.insert(1, ":*:FromToDate:*:");
    entry.text = elements.join(",");
    entries.append(entry);
  }
  return entries;
}

QString AssCutter::cutContent(const QString &content, const QStringList &cutList)
{
  QString scriptInfo = content;
  scriptInfo = scriptInfo.remove(scriptInfo.indexOf("[V"), scriptInfo.size());
  QString style = content;
  style = style.remove(0, style.indexOf("[V"));
  style = style.remove(style.indexOf("[E"), style.size());
  QString events = content;
  events = events.remove(0, events.indexOf("[E"));
  events = events.remove(events.indexOf("Dialogue:"), events.size());
  QString dialog = content;
  dialog = dialog.remove(0, dialog.indexOf("Dialogue:"));
  QList<SubtitleEntry> tokens = this->tokenize(dialog);
  if (m_debug) {
    cout << " -> " << qPrintable(this->objectName()) << " found " << tokens.count();
    cout << " subtitle entries" << endl;
  }
  adjustEntries(tokens, cutList);
  QString subtitles = scriptInfo + style + events;
  QString text;
  foreach(SubtitleEntry entry, tokens)
  {
    subtitles += "Dialogue: ";
    text = entry.text;
    text = text.replace(":*:FromToDate:*:",
                        secondsToHMSZZZ(entry.from) + "," + secondsToHMSZZZ(entry.to));
    subtitles += text;
    subtitles += "\n";
  }
  return subtitles;
}

