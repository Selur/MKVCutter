#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollbar>
#include <iostream>
#include "Globals.h"
using namespace std;

MkvCutter::MkvCutter(QWidget *parent) :
    QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_indexFile(QString()),
        m_currentOutput(QString()), m_tempFolder(QString()),
        m_avcProfileLevel(QString("High@L4.1")), m_audioFormat(QString()), m_avcCabac(true),
        m_avcRefFrames(1), m_enabled(0), m_frameCount(0), m_keyframes(), m_cuts(), m_splitFiles(),
        m_tempReencodeAvs(), m_videoEncodingCalls(),
        m_reencodedVideoFiles(), m_fps(-1), m_trimming(), m_matroskaKeyFrameTimes(), m_cutList(),
        m_mkvVideoParts(), m_mkvAudioParts(), m_audioFile(QString()), m_averageBitrate(-1), m_audioSplitFiles(),
        m_extractionFiles(), m_videoTrackID(-1), m_extractor(NULL), m_toDelete()
{
  this->setObjectName("MkvCutter-Main");
  m_mkvinfoAnalyser = new MkvInfoSourceAnalyser(this);
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(keyFrameInfos(QStringList)), this,
                  SLOT(setKeyFrames(QStringList)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(videoTrackID(int)), this,
                  SLOT(setVideoTrackID(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(finished()), this, SLOT(mkvAnalysefinished()));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(progress(int)), this, SLOT(mkvAnalyseProgress(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(frameCount(int)), this, SLOT(setFrameCount(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(fps(double)), this, SLOT(setFPS(double)));
  m_mediaInfoAnalyser = new MediaInfoAnalyser(this);
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(finished(int)), this, SLOT(mediaInfoFinished(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(avcProfileLevel(QString)), this,
                  SLOT(setAvcProfileLevel(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(refframes(int)), this, SLOT(setAvcRefFrames(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(cabac(bool)), this, SLOT(setAvcCabac(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(audioFormat(QString)), this,
                  SLOT(setAudioFormat(QString)));
  m_ffindexCaller = new FFIndexCaller(this);
  this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
  this->myconnect(m_ffindexCaller, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  m_mkvVideoSplitCaller = new MkvSplitCaller(this);
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(finished(int)), this, SLOT(mkvSplitFinished(int)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(progress(int)), this, SLOT(mkvsplitProgress(int)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(splitFiles(QStringList)), this,
                  SLOT(setSplitFiles(QStringList)));
  m_mkvAudioCutCaller = new MkvSplitCaller(this);
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(finished(int)), this, SLOT(mkvAudioCutFinished(int)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(progress(int)), this, SLOT(mkvsplitProgress(int)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(splitFiles(QStringList)), this,
                  SLOT(setAudioSplitFiles(QStringList)));
  m_mkvMerger = new MkvMerger(this);
  this->myconnect(m_mkvMerger, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvMerger, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvMerger, SIGNAL(finished(int)), this, SLOT(mkvMergerFinished(int)));
  this->myconnect(m_mkvMerger, SIGNAL(progress(int)), this, SLOT(mkvMergerProgress(int)));
  m_x264 = new X264Caller(this);
  this->myconnect(m_x264, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_x264, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_x264, SIGNAL(finished(int)), this, SLOT(x264Finished(int)));
  this->myconnect(m_x264, SIGNAL(progress(int)), this, SLOT(x264Progress(int)));
  m_extractor = new MkvVideoExtractor(this);
  this->myconnect(m_extractor, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_extractor, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_extractor, SIGNAL(finished(int)), this, SLOT(mkvExtractorFinished(int)));
  this->myconnect(m_extractor, SIGNAL(progress(int)), this, SLOT(mkvExtractorProgress(int)));
  m_viewer = 0;
  ui.setupUi(this);
  ui.mainStackedWidget->setCurrentIndex(0);
}

MkvCutter::~MkvCutter()
{
  this->reset();
}


void MkvCutter::setVideoTrackID(int id)
{
    m_videoTrackID = id;
}

void MkvCutter::setAverageBitrate(int bitrate)
{
    m_averageBitrate = bitrate;
    this->addInfo(" " + tr("video avreage bitrate: %1").arg(bitrate));
}

void MkvCutter::setSplitFiles(QStringList splitFiles)
{
  m_splitFiles = splitFiles;
  this->addInfo(" " + tr("video splitter created the following files:"));
  int i = 1;
  foreach(QString fileName, splitFiles) {
      this->addInfo("  "+tr("%1: %2").arg(i++).arg(fileName));
  }
}


void MkvCutter::setAudioSplitFiles(QStringList splitFiles)
{
  m_audioSplitFiles = splitFiles;
  this->addInfo(" " + tr("audio splitter created the following files:"));
  int i = 1;
  foreach(QString fileName, splitFiles) {
      this->addInfo("  "+tr("%1: %2").arg(i++).arg(fileName));
  }
}


void MkvCutter::setFPS(double framerate)
{
  m_fps = framerate;
  this->addInfo(tr("Video stream frame rate: %1").arg(m_fps));
}

void MkvCutter::on_openSourcePushButton_clicked()
{
  this->reset();
  QString name = tr("Select mkv input file");
  QString select = tr("Input (*.mkv)");
  QString inputPath = QApplication::applicationDirPath();
  QString input = QFileDialog::getOpenFileName(this, name, inputPath, select);
  if (!input.endsWith(".mkv") || input.isEmpty()) { //abort if input does not end with .avs
    QString text = tr("Input needs to be a .mkv file!");
    QMessageBox::critical(this, tr("Error"), text);
    this->reset();
    return;
  }
  m_currentInput = QDir::toNativeSeparators(input); //set current input
  m_mkvinfoAnalyser->analyse(m_currentInput);
}

void MkvCutter::ffindexProgress(int percent)
{
  ui.infoLabel->setText(tr("FFIndex at %1%").arg(percent));
}

void MkvCutter::mkvMergerProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvMerge merging at %1").arg(percent));
}
void MkvCutter::mkvExtractorProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvExtractor at %1").arg(percent));
}

void MkvCutter::mkvsplitProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvMerge splitting at %1").arg(percent));
}

void MkvCutter::x264Progress(int percent)
{
  ui.infoLabel->setText(tr("x264 encoding at %1%,..").arg(percent));
}

void MkvCutter::mkvAnalyseProgress(int linesRead)
{
  ui.infoLabel->setText(tr("MkvInfoAnalyser read %1 lines,..").arg(linesRead));
}

bool MkvCutter::createAVS()
{
  m_indexFile = m_tempFolder;
  if (m_indexFile.isEmpty()) {
      m_indexFile = Globals::getDirectory(m_currentInput);
  }
  m_indexFile += QDir::separator();
  m_indexFile += Globals::getFileName(m_currentInput);
  QString temp = m_indexFile;
  m_indexFile += ".ffindex";
  m_indexFile = QDir::toNativeSeparators(m_indexFile);
  temp += ".avs";
  m_tempAvs = QDir::toNativeSeparators(temp);

  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  QString path = QDir::toNativeSeparators(inputPath + "ffms2.dll");
  if (!QFile::exists(path)
      && QFile::exists(QDir::toNativeSeparators(inputPath + "ffms2-x64.dll"))) {
    path = QDir::toNativeSeparators(inputPath + "ffms2-x64.dll");
  }
  script << "LoadPlugin(\"" + path + "\")";
  QString call = "FFVideoSource(\"" + m_currentInput + "\"";
  call += ", ";
  call += "cachefile=\"" + m_indexFile + "\"";
  //TODO: add fpsnum, fpsden
  call += ", threads=1";
  call += ")";
  script << call;
  return Globals::saveTextTo(script.join("\n"), m_tempAvs) == 0;
}

void MkvCutter::mkvAnalysefinished()
{
  ui.infoLabel->setText(tr("MkvInfoAnalyser finished,.."));
  if (m_keyframes.isEmpty()) {
    QMessageBox::critical(this, tr("Error"), tr("No keyframes found in %1!").arg(m_currentInput));
    this->reset();
    return;
  }
  if (!this->createAVS()) {
    this->reset();
    return;
  }
  ui.infoLabel->setText(tr("Analyzing input file with MediInfo,.."));
  m_mediaInfoAnalyser->analyse(m_currentInput);
}

QString numberToLength3String(int num)
{
  QString ret = QString::number(num);
  while (ret.size() < 3) {
    ret = "0" + ret;
  }
  return ret;
}

/**
 * this method creates the segment split time points
 */
void MkvCutter::createAudioCutCall(QString filename, QString trim)
{
    if (m_audioFormat.isEmpty()) {
        this->cleanUpAndMerge();
        return;
    }
    QStringList call;
    call << "ffmpeg";
    call << "-vn"; // disable video
    call << "-acodec copy"; // copy audio
    QStringList startTimes;
    QStringList endTimes;
    QStringList frames;
    for (int i = 0, c = m_cuts.count(); i < c; ++i) {
        frames = m_cuts.at(i).split("-");
        startTimes << Globals::frameToTime(frames.at(0).toInt(), m_fps);
        endTimes << Globals::frameToTime(frames.at(1).toInt(), m_fps);
    }
    QMessageBox::critical(this, tr("StartTimes"), startTimes.join("\n"));
    QMessageBox::critical(this, tr("EndTimes"), endTimes.join("\n"));
}

void MkvCutter::createAvisynthSkript(QString filename, QString trim)
{
  this->addInfo(" " + tr("createAvisynthSkript(%1, %2)").arg(filename).arg(trim));
  QString avisynthFileName = m_tempFolder;
  avisynthFileName += QDir::separator();
  avisynthFileName += Globals::getFileName(filename);
  avisynthFileName += ".avs";
  avisynthFileName = QDir::toNativeSeparators(avisynthFileName);
  this->addInfo("  "+tr("Avisynth file name: %1").arg(avisynthFileName));
  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  QString path = QDir::toNativeSeparators(inputPath + "ffms2.dll");
  if (!QFile::exists(path)
      && QFile::exists(QDir::toNativeSeparators(inputPath + "ffms2-x64.dll"))) {
    path = QDir::toNativeSeparators(inputPath + "ffms2-x64.dll");
  }
  script << "LoadPlugin(\"" + path + "\")";
  script << "FFVideoSource(\"" + filename + "\", threads=1)";
  script << trim;
  trim = script.join("\n");
  if (Globals::saveTextTo(trim, avisynthFileName) == 0) {
    this->addInfo("  "+tr("Saved avisynth script:"));
    this->addInfo("   ----------------------------");
    this->addInfo(trim);
    this->addInfo("   ----------------------------");
    this->addInfo("  "+tr("to: %1").arg(avisynthFileName));
    m_tempReencodeAvs << avisynthFileName;
    this->createVideoReencodeCall(avisynthFileName);
  } else {
    QMessageBox::critical(
      this,
      tr("Error"),
      tr("createAvisynthSkript: Couldn't create(%1)").arg(avisynthFileName));
    return;
  }
}

cutTyp1 MkvCutter::findCutForFrame(int frame)
{
    cutTyp1 cut;
    cut.prevKey = -1;
    cut.nextKey = -1;
    cut.cut.start = frame;
    cut.cut.end = -1;
    //this->addInfo(" "+tr("findCutForFrame(%1)").arg(frame));
    QString tmp;
    QStringList elems;
    int previousKey = 0, currentKey;
    for (int keyIndex = 0, keyCount = m_keyframes.count(); keyIndex < keyCount; ++keyIndex) {
        tmp = m_keyframes.at(keyIndex);
        this->addInfo("  "+tr("looking at m_keyframes.at(%1): %2").arg(keyIndex).arg(tmp));
        elems = tmp.split(",");
        currentKey = elems.at(0).toInt();
        this->addInfo("  "+tr("looking at current key frame position: %1").arg(currentKey));
        if (currentKey < frame) {
          this->addInfo("  -> "+tr("currentKey(%1) < frame(%2) => previousKey(%3) = currentKey(%4)").arg(currentKey).arg(frame).arg(previousKey).arg(currentKey));
          previousKey = currentKey;
          if (keyIndex+1 < keyCount) {
              continue;
          }
        }
        if (currentKey == frame) {
            if (keyIndex+1 < keyCount) {
                previousKey = frame;
                this->addInfo("  -> "+tr("currentKey(%1) == frame(%2) -> previousKey(%3) = frame(%2)").arg(currentKey).arg(frame).arg(previousKey));
                continue;
            }
            this->addInfo("  -> "+tr("frame(%2) == lastKey(%1) -> previousKey(%3) = frame(%2) && cut.nextKey(%4) = frameCount && cut.cut.end(%5) = frame(%2)").arg(currentKey).arg(frame).arg(previousKey).arg(cut.nextKey).arg(m_frameCount).arg(cut.cut.end));
            cut.prevKey = currentKey;
            cut.nextKey = m_frameCount;
            cut.cut.end = frame;
            break;

        }
        if (currentKey > frame) {
            if (currentKey == m_frameCount) {
                cut.cut.end = currentKey;
                this->addInfo("  -> "+tr("currentKey(%1) > frame(%2) == framecount => cut.prevKey(%3) = previousKey(%4) && cut.nextKey(%5) = currrentKey(%1) && cut.cut.end(%6) = frameCount (%1) && previousKey(%4) = currentKey (%1)").arg(currentKey).arg(frame).arg(cut.prevKey).arg(previousKey).arg(cut.nextKey).arg(cut.cut.end));
            } else {
                cut.cut.end = currentKey-1;
                this->addInfo("  -> "+tr("currentKey(%1) > frame(%2) => cut.prevKey(%3) = previousKey(%4) && cut.nextKey(%5) = currrentKey(%1) && cut.cut.end(%6) = currentKey-1 (%7) && previousKey(%4) = currentKey (%1)").arg(currentKey).arg(frame).arg(cut.prevKey).arg(previousKey).arg(cut.nextKey).arg(cut.cut.end).arg(currentKey-1));
            }
            cut.prevKey = previousKey;
            cut.nextKey = currentKey;
            previousKey = currentKey;
            break;
        }

        this->addInfo("  -> "+tr("frame(%1) > lastKey(%2) => cut.prevKey(%3) = currentKey(%2) && cut.nextKey(%4) = frameCount(%5) && cut.cut.start(%6) = currentKey(%2) && cut.cut.end(%7) = frameCount(%5)").arg(frame).arg(currentKey).arg(cut.prevKey).arg(cut.nextKey).arg(m_frameCount).arg(cut.cut.start).arg(cut.cut.end));
        cut.prevKey = currentKey;
        cut.nextKey = m_frameCount;
        cut.cut.start = currentKey;
        cut.cut.end = frame;
        break;
    }
    if (cut.nextKey == -1) {
        cut.nextKey = m_frameCount;
    }
    this->addInfo(" => " +tr("findCutForFrame(%1): %2").arg(frame).arg(Globals::cutTyp1ToString(cut)));
    return cut;
}


void MkvCutter::calculateMatroskyKeyFrameTimes()
{
    this->addInfo("calculating matroskay key frame times,..");
    m_matroskaKeyFrameTimes.clear();
    QString tmp, temp;
    QStringList elems;
    for (int keyIndex = 0, keyCount = m_keyframes.count(); keyIndex < keyCount; ++keyIndex) {
        tmp = m_keyframes.at(keyIndex);
        elems = tmp.split(",");
        tmp = elems.at(0);
        temp = elems.at(1);
        temp = temp.remove(0, temp.indexOf("(")+1);
        temp = temp.remove(temp.indexOf(")"), temp.length());
        //this->addInfo(" "+tr("key frame: %1 @ time: %2").arg(tmp).arg(temp));
        m_matroskaKeyFrameTimes.insert(tmp.toInt(), temp);
    }
}

void MkvCutter::buildCutList()
{
    this->addInfo("collecting cut list and audio cuts,..");
    m_mkvAudioParts.clear();
    m_cutList.clear();

    QStringList tCuts;
    int start, end;
    QString startTime, endTime;
    cutTyp1 startCut, endCut, tempCut;
    for (int i = 0, c = m_cuts.count(); i < c; ++i) {
        tCuts = m_cuts.at(i).split("-");
        start = tCuts.at(0).toInt();
        end = tCuts.at(1).toInt();
        startCut = findCutForFrame(start);
        endCut = findCutForFrame(end);

        // add audio cut
        if (start == 0) {
            startTime = QString();
        } else {
            startTime = Globals::frameToTime(start, m_fps);
        }
        if (end == 0) {
            endTime = QString();
        } else {
            endTime = Globals::frameToTime(end, m_fps);
        }

        m_mkvAudioParts << startTime+"-"+endTime;

        // CUT LIST

        // A: start&end frame are in the same GOP

        if (startCut.prevKey == endCut.prevKey && endCut.nextKey == startCut.nextKey) {
            //CUT LIST
            tempCut.cut.start = start;
            tempCut.cut.end = end;
            tempCut.prevKey = startCut.prevKey;
            tempCut.nextKey = startCut.nextKey;
            this->addInfo(" " +tr("A1: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
            m_cutList.append(tempCut);
            continue;
        }
        if (startCut.nextKey ==  endCut.prevKey) {
            tempCut.cut.start = start;
            tempCut.cut.end = end;
            tempCut.prevKey = startCut.prevKey;
            tempCut.nextKey = endCut.nextKey;
            this->addInfo(" " +tr("A2: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
            m_cutList.append(tempCut);
            continue;
        }

        // B: start&end frame are in different GOPs
        // start cut
        tempCut.cut.start = start;
        tempCut.cut.end = startCut.nextKey - 1;
        tempCut.prevKey = startCut.prevKey;
        tempCut.nextKey = startCut.nextKey;
        this->addInfo(" " +tr("B: adding startCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
        m_cutList.append(tempCut);

        // middle&end cut
        if (end == endCut.nextKey-1)  {
            tempCut.cut.start = startCut.nextKey;
            tempCut.cut.end = end;
            tempCut.prevKey = endCut.prevKey;
            tempCut.nextKey = endCut.nextKey;
            this->addInfo(" " +tr("B: adding middle&endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
            m_cutList.append(tempCut);
            continue;
        }

        // middle cut
        tempCut.cut.start = startCut.nextKey;
        tempCut.cut.end = endCut.prevKey - 1;
        tempCut.prevKey = startCut.nextKey;
        tempCut.nextKey = endCut.prevKey;
        this->addInfo(" " +tr("B: adding middleCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
        m_cutList.append(tempCut);

        // end cut
        tempCut.cut.start = endCut.prevKey;
        tempCut.cut.end = end;
        tempCut.prevKey = endCut.prevKey;
        tempCut.nextKey = endCut.nextKey;
        this->addInfo(" " +tr("B: adding endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
        m_cutList.append(tempCut);
    }
}

void MkvCutter::buildTrimAndPartsList()
{
    this->addInfo("building trim and video parts,...");
    m_mkvVideoParts.clear();
    m_trimming.clear();
    int cutStart, cutEnd, prevKey, nextKey, clipStart = 0, clipEnd = m_frameCount, cutLength;
    int lastNextKey = -1, lastStartKey = -1;
    bool append;
    int fileStartKey = -1, fileEndKey = -1;
    QStringList mkvparts;
    QString name, trim, negReplace, temp1, temp2 ;
    cutTyp1 cut;
    int fileIndex = 0;
    for (int i = 0, c = m_cutList.count(); i < c; ++i) {
      cut = m_cutList.at(i);
      this->addInfo(" Looking at: "+Globals::cutTyp1ToString(cut));
      cutStart = cut.cut.start;
      cutEnd = cut.cut.end;
      cutLength = cutEnd-cutStart;
      prevKey = cut.prevKey;
      nextKey = cut.nextKey;
      if (cutStart == prevKey && cutEnd == nextKey-1) {
          fileIndex++;
          this->addInfo("  " + tr("!append -> fileIndex %1").arg(fileIndex));
          name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex) + ".mkv";
          trim = "KEEP";
          this->addInfo("  " + tr("adding %1 <> %2").arg(name).arg(trim));
          m_trimming.insert(name, trim);
          temp1 = m_matroskaKeyFrameTimes.value(prevKey);
          if (temp1 == "00:00:00.000") {
              temp1 = QString();
          }
          temp2 = m_matroskaKeyFrameTimes.value(nextKey);
          this->addInfo("  " + tr("keep: mkv parts append: %1").arg(temp1+"-"+temp2));
          mkvparts.append(temp1+"-"+temp2);
          continue;
      }
      if (prevKey >= nextKey) {
        nextKey = m_frameCount;
        this->addInfo("  " + tr("prevKey >= nextKey -> nextKey = %1").arg(m_frameCount));
      }
      if (prevKey == lastStartKey) {
          append = true;
          fileEndKey = nextKey;
          lastNextKey = nextKey;
      } else if (prevKey == lastNextKey) {
          append = true;
          fileEndKey = nextKey;
          prevKey = lastStartKey;
          lastStartKey = prevKey;
          negReplace = QString::number(prevKey);
      } else {
          append = false;
          fileStartKey = prevKey;
          fileEndKey = nextKey;
          lastStartKey = prevKey;
          lastNextKey = nextKey;
      }
      this->addInfo("  " + tr("append: %1").arg((append) ? "true" : "false"));
      this->addInfo("  " + tr("File start %1, end: %2 key").arg(fileStartKey).arg(fileEndKey));
      if (!append) {
        fileIndex++;
        this->addInfo("  " + tr("!append -> fileIndex %1").arg(fileIndex));
        name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex) + ".mkv";
      } else if (!mkvparts.isEmpty()) {
        mkvparts.removeLast();
      }
      temp1 = m_matroskaKeyFrameTimes.value(fileStartKey);
      if (temp1 == "00:00:00.000") {
          temp1 = QString();
      }
      temp2 = m_matroskaKeyFrameTimes.value(fileEndKey);
      this->addInfo("  " + tr("mkv parts append: %1").arg(temp1+"-"+temp2));
      mkvparts.append(temp1+"-"+temp2);

      if (!append && (cutStart == clipStart || cutStart == prevKey)) {
          trim = "Trim(0,";
          if (cutEnd == nextKey || cutEnd == clipEnd) {
              trim = "KEEP";
              this->addInfo("  " + tr("adding %1 <> %2").arg(name).arg(trim));
              m_trimming.insert(name, trim);
              continue;
          }
          //now: cutEnd < nextKey
          trim += "length="+QString::number(cutLength)+")";
          this->addInfo("  " + tr("adding %1 <> %2").arg(name).arg(trim));
          m_trimming.insert(name, trim);
          continue;
      }
      //now: cutStart > prevKey/clipStart
      if (append) {
        trim = m_trimming.value(name)+"+Trim(";
        trim = trim.replace(",-1)", ","+negReplace+")");
      } else {
        trim = "Trim(";
      }
      trim += QString::number(cutStart-prevKey) +",";
      if (cutEnd == nextKey || cutEnd == clipEnd) {
        trim += "-1)";
        this->addInfo("  " + tr("adding %1 <> %2").arg(name).arg(trim));
        m_trimming.insert(name, trim);
        continue;
      }
      //now: cutEnd < nextKey
      trim += "length="+QString::number(cutLength)+")";
      this->addInfo("  " + tr("adding %1 <> %2").arg(name).arg(trim));
      m_trimming.insert(name, trim);
      continue;
    }
    if (m_trimming.count() == 1) {
        QString trim = m_trimming.value(name);
        this->addInfo("  " + tr("removing %1 <> %2 from trim list").arg(name).arg(trim));
        m_trimming.clear();
        this->addInfo("  " + tr("adding %1 <> %2 to trim list").arg(m_currentInput).arg(trim));
        m_trimming.insert(m_currentInput, trim);
    }
    if (mkvparts.count() == 1) {
        if (mkvparts.first().trimmed() == "-") {
            mkvparts.clear();
        }
    }
    m_mkvVideoParts = mkvparts;
}

int maxMainRate(const QString level)
{
    int ret = 0;
    if (level == "1" || level == "1.0")
        ret = 64;
    else if (level == "1b")
        ret = 128;
    else if (level == "1.1")
        ret = 192;
    else if (level == "1.2")
        ret = 384;
    else if (level == "1.3")
        ret = 768;
    else if (level == "2" || level == "2.0")
        ret = 2000;
    else if (level == "2.1")
        ret = 4000;
    else if (level == "2.2")
        ret = 4000;
    else if (level == "3" || level == "3.0")
        ret = 10000;
    else if (level == "3,1")
        ret = 14000;
    else if (level == "3.2")
        ret = 20000;
    else if (level == "4" || level == "4.0")
        ret = 20000;
    else if (level == "4.1")
        ret = 50000;
    else if (level == "4.2")
        ret = 50000;
    else if (level == "5" || level == "5.0")
        ret = 135000;
    else if (level == "5.2")
        ret = 240000;
    return ret;
}

int maxMainBuff(const QString level)
{
  int ret = 720000;
  if (level == QObject::tr("1") || level == "1.0")
    ret = 175;
  else if (level == "1b")
    ret = 350;
  else if (level == "1.1")
    ret = 500;
  else if (level == "1.2")
    ret = 1000;
  else if (level == "1.3")
    ret = 2000;
  else if (level == QObject::tr("2") || level == "2.0")
    ret = 2000;
  else if (level == "2.1")
    ret = 4000;
  else if (level == "2.2")
    ret = 4000;
  else if (level == QObject::tr("3") || level == "3.0")
    ret = 10000;
  else if (level == "3.1")
    ret = 14000;
  else if (level == "3.2")
    ret = 20000;
  else if (level == QObject::tr("4") || level == "4.0")
    ret = 25000;
  else if (level == "4.1")
    ret = 62500;
  else if (level == "4.2")
    ret = 62500;
  else if (level == "5" || level == "5.0")
    ret = 135000;
  else if (level == "5.1" || level == "5.2") {
    ret = 240000;
  }
  return ret;
}

void MkvCutter::createVideoReencodeCall(QString avisynthFile)
{
  this->addInfo(" "+tr("creating x264 reencode call for: %1").arg(avisynthFile));
  QString x264 = QApplication::applicationDirPath() + QDir::separator();
#ifdef Q_OS_WIN32
  x264 += "x264.exe";
#else
  x264 += "x264";
#endif
  QStringList call;
  QString tmp;
  tmp = "\"" + x264 + "\"";
  call << tmp;
  tmp = "--profile ";
  if (m_avcProfileLevel.contains("High") || m_avcProfileLevel.isEmpty()) {
    tmp += "high";
  } else if (m_avcProfileLevel.contains("High")) {
    tmp += "baseline";
  } else {
    tmp += "main";
  }
  call << tmp;

  tmp = m_avcProfileLevel;
  if (!tmp.isEmpty()) {
    tmp = tmp.remove(0, tmp.indexOf("@") + 2);
    int maxBuff = maxMainBuff(tmp);
    int maxRate = maxMainRate(tmp);
    tmp = tmp.remove(".");
    tmp = "--level " + tmp;
    call << tmp;
    if (maxBuff != 0)
        call << "--vbv-bufsize "+QString::number(maxBuff);
    if (maxRate != 0)
        call << "--vbv-maxrate "+QString::number(maxRate);
  }
  if (!m_avcCabac) {
    call << "--no-cabac";
  }
  call << "--thread-input";
  //TODO: bluray check
  call << "--crf 19";
  call << "--demuxer avs";
  tmp = avisynthFile;
  tmp = tmp.remove(tmp.indexOf("."), tmp.size());
  tmp += "_reencode.264";
  m_reencodedVideoFiles << tmp;
  tmp = "-o \"" + tmp + "\"";
  call << tmp;
  tmp = "\"" + avisynthFile + "\"";
  call << tmp;
  tmp = call.join(" ");
  this->addInfo(" -> "+tr("x264 call: %1").arg(tmp));
  m_videoEncodingCalls << tmp;
}

void MkvCutter::startVideoReencoding()
{
  if (m_videoEncodingCalls.isEmpty()) { //encodings finished
    this->addInfo(tr("Finished all the video reencoding,..."));
    //QMessageBox::information(this, tr("PING"), tr("Finished all the video reencoding,..."));
    this->cutAudio();
    return;
  }
  this->addInfo(tr("encoding next file,..."));
  m_x264->start(m_videoEncodingCalls.takeFirst());
}

void MkvCutter::cleanUpAndMerge()
{
  //QMessageBox::information(this, tr("PING"), tr("cleanUpAndMerge,.."));
  this->addInfo(tr("cleanUpAndMerge,..."));
  int videoFileCount = m_reencodedVideoFiles.count();
  int audioFileCount = m_audioSplitFiles.count();
  this->addInfo(" " + tr("video file count: %1").arg(videoFileCount));
  this->addInfo(" " + tr("audio file count: %1").arg(audioFileCount));

  //QMessageBox::information(this, tr("PING"), tr("videoFileCount,.."));
  if (videoFileCount == 1) {
      if (m_audioFile.isEmpty()) {
        this->addInfo(" " + tr("no audio file present -> renaming videoFile,.."));
        QString tmp = m_reencodedVideoFiles.first();
        if (!QFile::rename(tmp, m_currentOutput)) {
            QMessageBox::critical(
                this,
                tr("Error"),
                tr("Couldn't move %1 to %2").arg(tmp).arg(m_currentOutput));
        } else {
            QMessageBox::information(this, tr("Finished!"), tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
        }
      } else {
            this->addInfo(" " + tr("audio file: %1").arg(m_audioFile));
            this->addInfo(" " + tr("Muxing audio&video(1),.."));
        m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_currentOutput);
      }
    this->reset();
    return;
  }

  // generate mkvmerge calls to join all parts
  this->addInfo(" " + tr("Muxing audio&video(2),.."));
  m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_currentOutput);
}

void MkvCutter::x264Finished(int exitstate)
{
  this->addInfo(" " + tr("x264 encoding finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since x264 crashed,.."));
    this->reset();
    return;
  }
  this->startVideoReencoding();
}

void MkvCutter::mkvMergerFinished(int exitstate)
{
  this->addInfo(tr("mkvMerge finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mkv merger crashed,.."));
    this->reset();
    return;
  }
  if (ui.keepIntermediateCheckBox->isChecked()) {
      QMessageBox::information(this, tr("Finished!"), tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
      this->reset();
      return;
  }

  this->addInfo(" "+tr("deleting split list elements,..."));
  foreach (QString file, m_splitFiles) {
      if (file.isEmpty() || (file == m_currentInput || !QFile::exists(file))) {
        continue;
      }

      this->addInfo("  "+tr("deleting video split file: %1").arg(file));
      if(!QFile::remove(file)) {
        this->addInfo("   "+tr("Couldn't delete %1!").arg(file));
      }
  }
  this->addInfo(" "+tr("deleting reencoded video files elements,..."));
  foreach (QString file, m_reencodedVideoFiles) {
    if (file.isEmpty() || file == m_currentInput && !QFile::exists(file)) {
      continue;
    }
    this->addInfo("  "+tr("deleting video file: %1").arg(file));
    if(!QFile::remove(file)) {
      this->addInfo("   "+tr("Couldn't delete %1!").arg(file));
    }
  }
  foreach (QString file, m_toDelete) {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    this->addInfo("  "+tr("deleting video file: %1").arg(file));
    if(!QFile::remove(file)) {
      this->addInfo("   "+tr("Couldn't delete %1!").arg(file));
    }
  }
  foreach (QString file, m_audioSplitFiles) {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    this->addInfo("  "+tr("deleting audio file: %1").arg(file));
    if(!QFile::remove(file)) {
      this->addInfo("   "+tr("Couldn't delete %1!").arg(file));
    }
  }
  if (!m_audioFile.isEmpty() && m_audioFile != m_currentInput) {
      this->addInfo("  "+tr("deleting audio file: %1").arg(m_audioFile));
      if (QFile::exists(m_audioFile)  && !QFile::remove(m_audioFile)) {
        this->addInfo("   "+tr("Couldn't delete %1!").arg(m_audioFile));
      }
  }

  QMessageBox::information(this, tr("Finished!"), tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
  this->reset();
}

void MkvCutter::mkvSplitFinished(int exitstate)
{
  this->addInfo(tr("mkvSplit finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mkv split caller crashed,.."));
    this->reset();
    return;
  }
  if (m_splitFiles.isEmpty()) {
    this->addInfo(tr("Resetting since mkv split did not create any split files"));
    this->reset();
    return;
  }
  this->handleSplitFiles();
}



void MkvCutter::mkvExtractorFinished(int exitstate)
{
  this->addInfo(tr("mkvExtract finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mkv extractor crashed,.."));
    this->reset();
    return;
  }
  this->startExtraction();
}

void MkvCutter::mkvAudioCutFinished(int exitstate)
{
  this->addInfo(tr("mkvAudioCut finished,.."));
  //QMessageBox::information(this, tr("PING"), tr("mkvAudioCut finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mkv split caller crashed,.."));
    this->reset();
    return;
  }
  this->cleanUpAndMerge();
}

void MkvCutter::handleSplitFiles()
{
    m_reencodedVideoFiles.clear();
    m_extractionFiles.clear();
    m_toDelete.clear();
    this->addInfo("handling split files,...");
    //handle splitFiles
    QString toDelete, trim;
    foreach (QString file, m_splitFiles) {
      toDelete = file;
      if (toDelete == m_currentInput || Globals::getWholeFileName(toDelete) == Globals::getWholeFileName(m_currentOutput)) {
          trim = m_trimming.value(m_currentInput);
      } else if (toDelete != m_currentInput) {
          toDelete = Globals::getWholeFileName(file);
          toDelete = toDelete.remove(0, toDelete.indexOf("-") + 1);
          toDelete = Globals::getFileName(m_currentInput)+"_cut_" + toDelete;
          trim = m_trimming.value(toDelete);
      }
      this->addInfo(" "+tr("trim value for %1: %2").arg(toDelete).arg(trim));
      if (trim == "KEEP" || trim.isEmpty()) {
        m_reencodedVideoFiles << file;
        m_extractionFiles << file;
        continue;
      }
      this->createAvisynthSkript(file, trim);
    }
    this->startExtraction();
}

void MkvCutter::startExtraction()
{
    if (m_extractionFiles.isEmpty()) {
      this->startVideoReencoding();
        return;
    }
    QString input = m_extractionFiles.takeFirst();
    QString filename = input;
    filename = filename.remove(filename.lastIndexOf("."), filename.length());
    filename += ".264";
    filename = m_tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
    filename = QDir::toNativeSeparators(filename);
    m_toDelete << filename;
    m_reencodedVideoFiles.replace(m_reencodedVideoFiles.indexOf(input), filename);
    m_extractor->startExtraction(input, "264", m_tempFolder);
}

void MkvCutter::mediaInfoFinished(int exitstate)
{
  ui.infoLabel->setText(tr("MediaInfo analysis finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mediaingo analyzer crashed,.."));
    this->reset();
    return;
  }

  ui.infoLabel->setText(tr("Indexing input file,.."));
  m_ffindexCaller->index(m_currentInput, m_indexFile);
}

void MkvCutter::ffIndexerFinished(int exitstate)
{
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since ffindexer crashed,.."));
    this->reset();
    return;
  }
  ui.infoLabel->setText(tr("Indexing input file finished,.."));
  delete m_viewer;
  m_viewer = new AVSViewer(this, m_tempAvs, 1, true);
  this->myconnect(m_viewer, SIGNAL(finished(int)), this, SLOT(avsViewerFinished(int)));
  this->myconnect(m_viewer, SIGNAL(cuts(QStringList)), this, SLOT(setCutList(QStringList)));
  this->myconnect(m_viewer, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  ui.avsViewerVerticalLayout->insertWidget(0, m_viewer);
  ui.mainStackedWidget->setCurrentIndex(1);
  ui.infoLabel->setText(tr("- Cut View -"));
  m_viewer->init();
}

void MkvCutter::avsViewerFinished(int state)
{
  if (state < 0) {
    this->addInfo(tr("Resetting since avs viewer crashed,.."));
    this->reset();
    return;
  }
  if (m_cuts.isEmpty()) {
    this->addInfo(tr("Resetting since cut-list is empty,.."));
    this->reset();
    return;
  }
  ui.infoLabel->setText(tr("Cut-View finished,.."));
  this->calculateMatroskyKeyFrameTimes();
  this->buildCutList();
  this->buildTrimAndPartsList();
  ui.infoLabel->setText(tr("Set output base file and temp folder,.."));
  ui.mainStackedWidget->setCurrentIndex(2);
}

void MkvCutter::on_outputPushButton_clicked()
{
  QString name = tr("Select mkv output base file");
  QString select = tr("Output (*.mkv)");
  QString inputPath = QApplication::applicationDirPath();
  if (!m_currentOutput.isEmpty()) {
      inputPath = Globals::getDirectory(m_currentOutput);
  } else if (!m_tempFolder.isEmpty()) {
      inputPath = m_tempFolder;
  }
  QString output = QFileDialog::getSaveFileName(this, name, inputPath, select);
  if (!output.isEmpty()) {
    output = QDir::toNativeSeparators(output);
    ui.outputLabel->setText(output);
    m_currentOutput = output;
  }
}

void MkvCutter::on_tempPushButton_clicked()
{
  QString name = tr("Select temp folder");
  QString inputPath = QApplication::applicationDirPath();
  if (!m_tempFolder.isEmpty()) {
      inputPath = m_tempFolder;
  } else if (!m_currentOutput.isEmpty()) {
      inputPath = Globals::getDirectory(m_currentOutput);
  }
  QString tempFolder = QFileDialog::getExistingDirectory(this, name, inputPath);
  if (!tempFolder.isEmpty()) {
    tempFolder = QDir::toNativeSeparators(tempFolder);
    ui.tempFolderLabel->setText(tempFolder);
    m_tempFolder = tempFolder;
    this->addInfo("New temp folder: " + m_tempFolder);
  }
}

void MkvCutter::on_nextPushButton_clicked()
{
  if (m_tempFolder.isEmpty() || this->m_currentOutput.isEmpty()) {
    QMessageBox::information(this, tr("Notice"),
                             tr("You need to specify the output file and the temp folder!"));
    return;
  }
  ui.mainStackedWidget->setCurrentIndex(3);
  int listCount = m_mkvVideoParts.size();
  this->addInfo(tr("mkvParts count: %1").arg(listCount));
  if (listCount == 0) {
    ui.infoLabel->setText(tr("No cuts using mkvmerge needed,.."));
    m_splitFiles << m_currentInput;
    this->handleSplitFiles();
    return;
  }
  ui.infoLabel->setText(tr("Calling mkvmerge,.."));
  this->buildAndCallMkvMerge();
}

void MkvCutter::buildAndCallMkvMerge()
{
  this->addInfo(tr("Calling video cutter,.."));
  m_mkvVideoSplitCaller->start(m_currentInput, m_currentOutput, m_mkvVideoParts, m_tempFolder);
}

void MkvCutter::cutAudio()
{
    this->addInfo(tr("Calling audio cutter,.."));
    m_audioFile = m_tempFolder + QDir::separator() + Globals::getWholeFileName(m_currentOutput);
    m_audioFile = m_audioFile.insert(m_audioFile.lastIndexOf("."),"_AudioCut");
    m_audioFile = QDir::toNativeSeparators(m_audioFile);
    m_mkvAudioCutCaller->start(m_currentInput, m_currentOutput, m_mkvAudioParts, m_tempFolder, true);
}

void MkvCutter::setKeyFrames(QStringList list)
{
  ui.infoLabel->setText(tr("Got key frame list from mkvinfo analyzer."));
  int count = list.count();
  int dist = m_frameCount / count;
  this->addInfo(tr("Video stream key frame count: %1, average distance: %2").arg(count).arg(dist));
  m_keyframes = list;
}

void MkvCutter::enableGui(bool enable)
{
  m_enabled += (enable) ? 1 : -1;
  this->setEnabled(m_enabled == 0);
}

void MkvCutter::addInfo(QString infos)
{
  ui.infoTextBrowser->append(infos);
  int bottom = ui.infoTextBrowser->verticalScrollBar()->maximum();
  ui.infoTextBrowser->verticalScrollBar()->setValue(bottom);
  cout << qPrintable(infos) << endl;
}

void MkvCutter::reset()
{
  if (!m_tempAvs.isEmpty()) {
    QFile::remove(m_tempAvs);
    m_tempAvs = QString();
  }
  if (!m_indexFile.isEmpty()) {
    QFile::remove(m_indexFile);
    m_indexFile = QString();
  }
  foreach(QString file, m_tempReencodeAvs) {
    QFile::remove(file);
  }
  m_currentInput = QString();
  m_currentOutput = QString();
  m_tempFolder  = QString();
  m_avcProfileLevel = QString();
  m_audioFormat  = QString();
  m_avcCabac = true;
  m_avcRefFrames = -1;
  m_enabled = 0;
  m_frameCount = -1;
  m_keyframes.clear();
  m_cuts.clear();
  m_splitFiles.clear();
  m_tempReencodeAvs.clear();
  m_videoEncodingCalls.clear();
  m_reencodedVideoFiles.clear();
  m_fps = -1; m_trimming.clear();
  m_matroskaKeyFrameTimes.clear();
  m_cutList.clear();
  m_mkvmergeIntSplitList.clear();
  m_mkvVideoParts.clear();
  m_mkvAudioParts.clear();
  m_audioFile = QString();
  m_averageBitrate = -1;
  m_audioSplitFiles.clear();
  m_extractionFiles.clear();
  m_toDelete.clear();
  m_videoTrackID = 0;
  ui.mainStackedWidget->setCurrentIndex(0);
}

void MkvCutter::setCutList(QStringList cuts)
{
  m_cuts = cuts;
}

void MkvCutter::setFrameCount(int count)
{
  m_frameCount = count;
  this->addInfo(tr("Video stream frame count: %1").arg(m_frameCount));
}

void MkvCutter::setAvcProfileLevel(QString pl)
{
  m_avcProfileLevel = pl;
}

void MkvCutter::setAvcCabac(bool cabac)
{
  m_avcCabac = cabac;
}

void MkvCutter::setAvcRefFrames(int frames)
{
  m_avcRefFrames = frames;
}

void MkvCutter::setAudioFormat(QString format)
{
  m_audioFormat = format;
}

void MkvCutter::myconnect(const QObject * sender, const char * signal, const QObject * receiver,
                          const char * method, Qt::ConnectionType type)
{
  if (!QObject::connect(sender, signal, receiver, method, type)) {
    QMessageBox::critical(
        this,
        tr("Error"),
        tr("Couldn't connect %1 '%2' to %3 '%4'").arg(sender->objectName()).arg(signal).arg(
            receiver->objectName()).arg(type));
  }
}
