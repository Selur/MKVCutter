#include "X264Caller.h"

X264Caller::X264Caller(QObject *parent) :
    QObject(parent), m_process(NULL)
{
}

void X264Caller::start(QString call)
{
    this->sendInfos(" "+tr("x264 call: %1").arg(call));
    delete m_process;
    m_process = new QProcess(this);
    QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                     SLOT(x264Finished(int, QProcess::ExitStatus)));
    QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(handleX264Output()));
    m_process->start(call);
}

void X264Caller::handleX264Output()
{
    QString input = m_process->readAllStandardError().data();
    QStringList lines = input.split("\n");
    QString tmp, percentage;
    int index;
    foreach (QString line, lines) {
        line = line.trimmed();
        //[62.0%] 266/429 frames, 225.23 fps, 1371.11 kb/s, eta 0:00:00
        if (line.startsWith("[") && line.contains(" kb/s, eta ")) {
          tmp = line;
          tmp = tmp.remove(0, 1);
          index = tmp.indexOf("]");
          percentage = tmp.remove(index, tmp.size()).trimmed(); //percentage
          emit progress(int(percentage.toDouble()));
        }
        emit sendInfos(line);
    }
}

void X264Caller::x264Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode < 0) {
      emit sendInfos(tr("ExitCode: %1, ExitStatus: %2").arg(exitCode).arg(exitStatus));
      emit
      finished(-1);
      return;
    }
    emit finished(0);
}
