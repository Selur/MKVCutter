#ifndef GLOBALS_H
#define GLOBALS_H

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

QString cutTypToString(cutTyp cut)
{
    return QString::number(cut.start)+"-"+QString::number(cut.end);
}

QString cutTyp1ToString(cutTyp1 cut)
{
    return QString::number(cut.prevKey)+" "+cutTypToString(cut.cut)+" "+QString::number(cut.nextKey);
}

QString cutTyp1ListToString(QList<cutTyp> elems) {
    QString cutString;
    foreach (cutTyp tCut, elems) {
        cutString += QString::number(tCut.start)+"-"+QString::number(tCut.end)+",";
    }
    if (!elems.isEmpty()) {
        cutString.remove(cutString.size()-1, 1);
    }
    return cutString;
}

QString intSetToString(QSet<int> keyframes)
{
    QList<int> list = keyframes.toList();
    qSort(list);
    QString keyString;
    foreach(int key, list) {
        keyString += QString::number(key) + ", ";
    }
    if (!list.isEmpty()) {
        keyString.remove(keyString.size()-2, 2);
    }
    return keyString;
}

QString secondsToHMS(double seconds)
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

QString intSetToTimes(QSet<int> keyframes, double fps)
{
    QList<int> list = keyframes.toList();
    qSort(list);
    QString keyString;
    foreach(int key, list) {
        keyString += secondsToHMS(key*fps) + ", ";
    }
    if (!list.isEmpty()) {
        keyString.remove(keyString.size()-2, 2);
    }
    return keyString;
}


#endif // GLOBALS_H
