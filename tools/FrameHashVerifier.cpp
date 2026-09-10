/*
 * FrameHashVerifier.cpp
 *
 *  Created on: Sep 10, 2026
 *      Author: Selur
 */

#include "FrameHashVerifier.h"
#include "Globals.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

FrameHashVerifier::FrameHashVerifier(QObject *parent)
    : QObject(parent), m_process(nullptr), m_source(QString()), m_output(QString()),
        m_reference(QString()), m_delivered(QString()), m_ranges(), m_readingSource(true)
{
}

FrameHashVerifier::~FrameHashVerifier()
{
}

QString FrameHashVerifier::tool() const
{
#ifdef Q_OS_WIN32
  const QString name = "ffmpeg.exe";
#else
  const QString name = "ffmpeg";
#endif
  return QDir::toNativeSeparators(
      QApplication::applicationDirPath() + QDir::separator() + name);
}

void FrameHashVerifier::start(const QString &source, const QString &output,
    const QStringList &ranges, const QString &tempFolder)
{
  m_source = source;
  m_output = output;
  m_ranges = ranges;
  QString folder = tempFolder.trimmed();
  if (folder.isEmpty() || !QDir(folder).exists()) {
    folder = QFileInfo(output).absolutePath();
  }
  const QString base = folder + QDir::separator() + Globals::getFileName(output);
  m_reference = QDir::toNativeSeparators(base + "_reference.framehash");
  m_delivered = QDir::toNativeSeparators(base + "_delivered.framehash");
  this->hashSource();
}

void FrameHashVerifier::hashSource()
{
  // 'select' laesst nur die angeforderten Frames durch. Die Kommas innerhalb von between()
  // muessen escaped werden, sonst liest ffmpeg sie als Trennung zwischen zwei Filtern.
  QStringList pieces;
  QStringList elems;
  foreach(QString range, m_ranges)
  {
    elems = range.split("-");
    if (elems.count() != 2) {
      continue;
    }
    const int from = elems.at(0).toInt();
    const int to = elems.at(1).toInt() - 1; // between() ist beidseitig einschliessend
    if (to < from) {
      continue;
    }
    pieces << QString("between(n\\,%1\\,%2)").arg(from).arg(to);
  }
  if (pieces.isEmpty()) {
    emit finished(tr("Nothing to check -- the cut list is empty."), false);
    return;
  }
  emit sendInfos(tr("Checking the cut: hashing the requested source frames,.."));
  m_readingSource = true;
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
      SLOT(stepFinished(int, QProcess::ExitStatus)));
  QStringList args;
  args << "-hide_banner" << "-loglevel" << "error" << "-i" << m_source << "-an";
  args << "-vf" << ("select=" + pieces.join("+"));
  args << "-fps_mode" << "passthrough" << "-f" << "framehash" << "-hash" << "md5";
  args << "-y" << m_reference;
  m_process->start(this->tool(), args);
}

void FrameHashVerifier::hashOutput()
{
  emit sendInfos(tr("Checking the cut: hashing the frames of the output,.."));
  m_readingSource = false;
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
      SLOT(stepFinished(int, QProcess::ExitStatus)));
  QStringList args;
  args << "-hide_banner" << "-loglevel" << "error" << "-i" << m_output << "-an";
  args << "-fps_mode" << "passthrough" << "-f" << "framehash" << "-hash" << "md5";
  args << "-y" << m_delivered;
  m_process->start(this->tool(), args);
}

void FrameHashVerifier::stepFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus != QProcess::NormalExit || exitCode != 0) {
    const QString error = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    this->cleanUp();
    emit finished(
        tr("Couldn't check the cut: ffmpeg failed (%1). %2").arg(exitCode).arg(error), false);
    return;
  }
  if (m_readingSource) {
    this->hashOutput();
    return;
  }
  this->compare();
}

QStringList FrameHashVerifier::hashesOf(const QString &file)
{
  QStringList hashes;
  QFile handle(file);
  if (!handle.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return hashes;
  }
  QTextStream stream(&handle);
  while (!stream.atEnd()) {
    const QString line = stream.readLine().trimmed();
    if (line.isEmpty() || line.startsWith("#")) {
      continue;
    }
    // stream, dts, pts, duration, size, hash
    const int last = line.lastIndexOf(",");
    if (last < 0) {
      continue;
    }
    hashes << line.mid(last + 1).trimmed();
  }
  handle.close();
  return hashes;
}

int FrameHashVerifier::matchesAt(const QStringList &reference, const QStringList &delivered,
    int shift)
{
  int matches = 0;
  for (int i = 0, c = delivered.count(); i < c; ++i) {
    const int index = i + shift;
    if (index < 0 || index >= reference.count()) {
      continue;
    }
    if (reference.at(index) == delivered.at(i)) {
      ++matches;
    }
  }
  return matches;
}

void FrameHashVerifier::compare()
{
  const QStringList reference = hashesOf(m_reference);
  const QStringList delivered = hashesOf(m_delivered);
  this->cleanUp();
  if (reference.isEmpty() || delivered.isEmpty()) {
    emit finished(tr("Couldn't check the cut: no frame hashes were written."), false);
    return;
  }
  const int aligned = matchesAt(reference, delivered, 0);
  // Neu codierte Frames stimmen nie ueberein -- entscheidend ist, ob sich der Rest durch
  // einen konstanten Versatz besser erklaeren laesst als durch keinen. Genau so sah der
  // Fehler aus, der B20 aufgedeckt hat: null Treffer an Ort und Stelle, viele bei -1.
  int bestShift = 0;
  int bestMatches = aligned;
  for (int shift = -30; shift <= 30; ++shift) {
    if (shift == 0) {
      continue;
    }
    const int matches = matchesAt(reference, delivered, shift);
    if (matches > bestMatches) {
      bestMatches = matches;
      bestShift = shift;
    }
  }
  QStringList lines;
  const bool countsDiffer = reference.count() != delivered.count();
  lines
      << tr("Requested: %1 frames, the output holds %2.").arg(reference.count()).arg(
          delivered.count());
  lines
      << tr("%1 of them are bit identical with the source; the rest were re-encoded.").arg(
          aligned);
  bool suspicious = countsDiffer;
  if (bestShift != 0) {
    suspicious = true;
    lines
        << tr("The output matches the source much better when shifted by %1 frames "
            "(%2 hits instead of %3) -- the cut sits in the wrong place.").arg(bestShift).arg(
            bestMatches).arg(aligned);
  }
  if (!suspicious) {
    lines << tr("The cut is where it should be.");
  }
  emit finished(lines.join("\n"), suspicious);
}

void FrameHashVerifier::cleanUp()
{
  QFile::remove(m_reference);
  QFile::remove(m_delivered);
}
