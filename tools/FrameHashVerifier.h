/*
 * FrameHashVerifier.h
 *
 *  Prueft nach, ob der Schnitt wirklich die angeforderten Frames enthaelt.
 *
 *  Alle anderen Pruefungen im Projekt rechnen den *Plan* nach: verify_cut.py liest das Log,
 *  verifyAndCorrectParts() vergleicht Teillaengen. Beides kann uebereinstimmen, waehrend die
 *  Ausgabe trotzdem verschobene Frames enthaelt -- gemessen an einer Quelle, deren
 *  Frameidentitaet zwischen Decoder und Indexer strittig ist (B20). Nur ein Vergleich der
 *  decodierten Bilder faengt das.
 *
 *  Deshalb: ffmpeg hasht die angeforderten Quellframes und die Frames der Ausgabe, und beide
 *  Listen werden Position fuer Position verglichen. Neu codierte Frames stimmen dabei nicht
 *  ueberein -- entscheidend ist, ob die *kopierten* an der richtigen Stelle sitzen und ob
 *  sich der Rest durch einen konstanten Versatz erklaeren laesst.
 */

#ifndef FRAMEHASHVERIFIER_H_
#define FRAMEHASHVERIFIER_H_

#include <QObject>
#include <QProcess>
#include <QStringList>

class FrameHashVerifier : public QObject
{
  Q_OBJECT
  public:
    explicit FrameHashVerifier(QObject *parent = nullptr);
    ~FrameHashVerifier();
    // ranges: "start-end" je Schnitt, in Frames des Clips, Ende exklusiv.
    void start(const QString &source, const QString &output, const QStringList &ranges,
               const QString &tempFolder);

  private:
    QProcess *m_process;
    QString m_source, m_output, m_reference, m_delivered;
    QStringList m_ranges;
    bool m_readingSource;
    QString tool() const;
    void hashSource();
    void hashOutput();
    void compare();
    void cleanUp();
    static QStringList hashesOf(const QString &file);
    static int matchesAt(const QStringList &reference, const QStringList &delivered, int shift);

  private slots:
    void stepFinished(int exitCode, QProcess::ExitStatus exitStatus);

  signals:
    void sendInfos(QString info);
    // suspicious == true: die Ausgabe passt nicht zur Anforderung.
    void finished(QString summary, bool suspicious);
};

#endif /* FRAMEHASHVERIFIER_H_ */
