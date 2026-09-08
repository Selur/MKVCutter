#include "MkvTimeExtractor.h"
#include <QApplication>
#include "Globals.h"
#include <QDir>

MkvTimeExtractor::MkvTimeExtractor(QObject *parent) :
    QObject(parent), m_process(nullptr), m_timecodeFile(QString())
{
}

void MkvTimeExtractor::startExtraction(QString filename, QString track, QString tempFolder)
{
 QString call = this->buildCall(filename, track, tempFolder);
 this->call(call);
}

void MkvTimeExtractor::call(QString call)
{
 this->sendInfos("Mkv extractor call: " + call);
 delete m_process;
 m_process = new QProcess(this);
 QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                  SLOT(mkvextractFinished(int, QProcess::ExitStatus)));
 QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                  SLOT(handleMkvExtractOutput()));
 emit enableGui(false);
 m_process->startCommand(call);
}

QString MkvTimeExtractor::buildCall(QString filename, QString track, QString tempFolder)
{
 if (tempFolder.isEmpty()) {
   tempFolder = Globals::getDirectory(filename);
 }
 QString call = QApplication::applicationDirPath();
 call += QDir::separator();
 call += "mkvextract.exe";
 call = "\"" + QDir::toNativeSeparators(call) + "\"";
 call += " timecodes_v2";
 call += " \"" + filename + "\"";
 if (track.isEmpty() || track == "-1") {
     track = "0";
 }
 call += " "+track+":";
 filename = filename.remove(filename.lastIndexOf("."), filename.length());
 filename = tempFolder + QDir::separator() + Globals::getWholeFileName(filename)+"_timecode";
 filename += ".tc";
 filename = QDir::toNativeSeparators(filename);
 m_timecodeFile = filename;
 call += "\"" + filename + "\"";
 return call;
}

void MkvTimeExtractor::handleMkvExtractOutput()
{
 QString text = m_process->readAllStandardOutput().data();
 if (!text.startsWith("Progress: ")) {
   return;
 }
 text = text.remove(0, text.indexOf(":") + 1);
 text = text.remove(text.indexOf("%"), text.size());
 text = text.remove("%").trimmed();
 emit progress(int(text.toDouble()));
}

void MkvTimeExtractor::mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
 emit enableGui(true);
 if (exitCode < 0) {
   emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
   emit finished(-1);
   return;
 }
 emit timecodes(m_timecodeFile);
 emit finished(0);
}
