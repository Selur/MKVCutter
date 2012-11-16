#include "MkvVideoExtractor.h"
#include <QApplication>
#include "Globals.h"
#include <QDir>

MkvVideoExtractor::MkvVideoExtractor(QObject *parent) :
    QObject(parent)
{
}


void MkvVideoExtractor::startExtraction(QString filename, QString typ, QString tempFolder)
{
    QString call = this->buildCall(filename, typ, tempFolder);
    this->call(call);
}


void MkvVideoExtractor::call(QString call)
{
    this->sendInfos("Mkv extractor call: "+call);
    delete m_process;
    m_process = new QProcess(this);
    QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                     SLOT(mkvextractFinished(int, QProcess::ExitStatus)));
    QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this,
                     SLOT(handleMkvExtractOutput()));
    m_process->start(call);
}

QString MkvVideoExtractor::buildCall(QString filename, QString typ, QString tempFolder)
{
    QString call = QApplication::applicationDirPath();
    call += QDir::separator();
    call += "mkvextract.exe";
    call = "\""+QDir::toNativeSeparators(call)+"\"";
    call += " tracks";
    call += " \""+filename+"\"";
    call += " 0:";
    filename = filename.remove(filename.lastIndexOf("."), filename.length());
    filename += ".264";
    filename = tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
    filename = QDir::toNativeSeparators(filename);
    call += "\""+filename+"\"";
    return call;
}


void MkvVideoExtractor::handleMkvExtractOutput()
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

void MkvVideoExtractor::mkvextractFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode < 0) {
      emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
      emit
      finished(-1);
      return;
    }
    emit finished(0);
}
