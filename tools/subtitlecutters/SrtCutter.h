/*
 * SrtCutter.h
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#ifndef SRTCUTTER_H_
#define SRTCUTTER_H_

#include "Cutter.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class SrtCutter : public Cutter
{
  Q_OBJECT
  public:
    SrtCutter(QObject *parent = 0, bool debug = false);
    ~SrtCutter();

  protected:
    QString cutContent(const QString &content, const QStringList &cutList);
    QList<SubtitleEntry> tokenize(const QString content);
};

#endif /* SRTCUTTER_H_ */
