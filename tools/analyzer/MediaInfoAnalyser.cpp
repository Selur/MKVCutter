#include "MediaInfoAnalyser.h"

#include <QDir>
#include <QApplication>
#include "tools/Converter.h"

MediaInfoAnalyser::MediaInfoAnalyser(QObject *parent) :
    QObject(parent)
{
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this,
                   SLOT(mediainfoFinished(int,QProcess::ExitStatus)));
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(mediainfoOutput()));
  QObject::connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(mediainfoOutput()));
}

void MediaInfoAnalyser::analyse(QString input)
{
  QString mediainfo = QApplication::applicationDirPath();
  mediainfo += QDir::separator();
#ifdef Q_OS_WIN32
  mediainfo += "MediaInfo.exe";
#else
  mediainfo += "MediaInfo";
#endif
  mediainfo = QDir::toNativeSeparators(mediainfo);
  QStringList call;
  call << "\"" + mediainfo + "\"";
  call << "--Full";
  call << "\"" + input + "\"";
  emit enableGui(false);
  emit sendInfos(tr("MediaInfo call: %1").arg(call.join(" ")));
  m_process->start(call.join(" "));
}

void removeInnerWhiteSpace(QString &line)
{
  line = line.remove(" ");
  line = line.trimmed();
}

void removeStartOfLine(QString &line)
{
  line = line.remove(0, line.indexOf(":") + 1);
  line = line.trimmed();
}

void MediaInfoAnalyser::mediainfoFinished(int exitState, QProcess::ExitStatus status)
{
  emit enableGui(true);
  if (exitState < 0) {
    emit sendInfos(tr("MediaInfo crashed: %1 - %2").arg(exitState).arg(status));
    emit finished(-1);
  }
  emit finished(0);
}

void MediaInfoAnalyser::mediainfoOutput()
{
  QString out = m_process->readAllStandardOutput().data();
  if (!out.isEmpty()) {
    bool audio = false;
    QStringList lines = out.split("\n");
    QString scanorder = "progressive", tmp;
    bool vfr = false;
    foreach(QString line, lines) {
      line = line.trimmed();
      //emit sendInfos(tr("MediaInfo out: %1").arg(line));
      if (line.startsWith("Text #") || line.startsWith("Text")) {
        break;
      }
      if (!audio) {
        if (line.startsWith("Encoding settings")) {
          removeStartOfLine(line);
          emit x264Settings(Converter::encodingSettingsToX264(line));
          continue;
        }
        if (line.startsWith("Format profile")) {
          removeStartOfLine(line);
          emit avcProfileLevel(line);
          continue;
        }
        if (line.startsWith("Format settings, CABAC")) {
          removeStartOfLine(line);
          emit cabac(line == "Yes");
          continue;
        }
        if (line.startsWith("Format settings, ReFrames")) {
          removeStartOfLine(line);
          int num =  line.toInt();
          if (num > 0) {
            emit refframes(num);
          }
          continue;
        }
        if (line.startsWith("Format settings, GOP")) {
            removeStartOfLine(line);
            QStringList elems = line.split(",");
            if (elems.count() == 2) {
                QString tmp = elems.at(0);
                tmp = tmp.remove("M=").trimmed();
                emit minKeyInt(tmp);
                tmp = elems.at(1);
                tmp = tmp.remove("N=").trimmed();
                emit maxKeyInt(tmp);
            } else {
                tmp = elems.at(0).trimmed();
                if (line.startsWith("M=")) {
                    tmp = tmp.remove("M=").trimmed();
                    emit minKeyInt(tmp);
                } else if (line.startsWith("N=")){
                    tmp = tmp.remove("N=").trimmed();
                    emit maxKeyInt(tmp);
                }
            }
        }

        if (line.startsWith("Pixel aspect ratio") && !line.contains("/")) {
          removeStartOfLine(line);
          removeInnerWhiteSpace(line);
          if (line == "1.000") {
            line = "1";
          }
          emit aspectRatio(line.toDouble());
          continue;
        }
        if (line.startsWith("Scan type") && line.endsWith("Interlaced")) {
          scanorder = "MBAFF";
          continue;
        }
        if (line.startsWith("Scan order")) {
          removeStartOfLine(line);
          if (line == "Top Field First" || line == "TFF") {
            scanorder = "TFF";
          } else if (line == "Bottom Field First" || line == "BFF") {
            scanorder = "BFF";
          }
        }
        if (line.startsWith("Frame rate mode")) {
          removeStartOfLine(line);
          if (line == "VFR") {
            vfr = true;
          }
        }

        if (line == "Audio" || line.startsWith("Audio #")) {
          audio = true;
          emit frameRateMode(vfr);
          continue;
        }
      } else {
        if (line.startsWith("Format") && !line.startsWith("Format profile")
            && !line.startsWith("Format/") && !line.startsWith("Format version")
            && !line.startsWith("Format settings")) {
          removeStartOfLine(line);
          if (line == "MPEG Audio") {
            continue;
          }
          m_audioFormat = line;
          emit audioFormat(line);
          continue;
        }
        if (line.startsWith("Format profile")) {
          removeStartOfLine(line);
          if (line == "High efficiency AAC@L2") {
            line = "he-aac";
          } else if (m_audioFormat.contains("DTS", Qt::CaseInsensitive)
              && (line == "MA / Core" || line == "ES" || line == "MA")) {
            line = "dts-hd";
          } else {
            continue;
          }
          m_audioFormat = line;
          emit audioFormat(line);
          continue;
        }

        if (!line.startsWith("Codec ID") && line.startsWith("Codec") && m_audioFormat.isEmpty()) {
          removeStartOfLine(line);
          if (line.startsWith("MPA1L")) {
            if (line.startsWith("MPA1L1")) {
              line = "mp1";
            } else if (line.startsWith("MPA1L2")) {
              line = "mp2";
            } else {
              line = "mp3";
            }
          }
          m_audioFormat = line;
          emit audioFormat(line.toLower());
          continue;
        }
        if (line.startsWith("Bitrate") && line.endsWith("Kbps")) {
          removeStartOfLine(line);
          line = line.remove(line.indexOf("K"), line.size());
          int bitrate = line.trimmed().toInt();
          emit averageBitrate(bitrate);
          continue;
        }
      }
    }
    emit interlaced(scanorder);
  }

  QString err = m_process->readAllStandardOutput().data();
  if (!err.isEmpty()) {
    emit sendInfos(tr("MediaInfo error: %1").arg(err));
  }
}
