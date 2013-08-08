/*
 * AssCutter.h
 *
 *  Created on: 06.07.2013
 *      Author: Selur
 */

#ifndef ASSCUTTER_H_
#define ASSCUTTER_H_

#include "Cutter.h"

class AssCutter : public Cutter
{
  public:
    AssCutter(QObject *parent = 0, bool debug = false);
    virtual ~AssCutter();

  protected:
      QString cutContent(const QString &content, const QStringList &cutList);
      QList<SubtitleEntry> tokenize(const QString dialog);
};

#endif /* ASSCUTTER_H_ */
