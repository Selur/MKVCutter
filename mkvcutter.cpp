#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollbar>
#include <QApplication>
#include <iostream>
#include "Globals.h"
using namespace std;

MkvCutter::MkvCutter(QWidget *parent) :
    QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_indexFile(QString()),
        m_currentOutput(QString()), m_tempFolder(QString()),
        m_avcProfileLevel(QString("High@L4.1")), m_audioFormat(QString()), m_avcCabac(true),
        m_avcRefFrames(1), m_enabled(0), m_frameCount(0), m_keyframes(), m_cuts(), m_splitFiles(),
        m_tempReencodeAvs(), m_videoEncodingCalls(), m_reencodedVideoFiles(), m_fps(-1),
        m_trimming(), m_cutList(), m_mkvVideoParts(), m_mkvAudioParts(), m_audioFile(QString()),
        m_averageBitrate(-1), m_audioSplitFiles(), m_extractionFiles(), m_videoTrackID(-1),
        m_extractor(NULL), m_timeextractor(NULL), m_toDelete(), m_aspectRatio(1),
        m_interlaced("progressive"), m_vfr(false), m_timecodes(QString()),
        m_x264Settings(QString()), m_minKey(QString()), m_maxKey(QString()), m_weightedP(0),
        m_weightedB(0), m_bframes(0), m_qpMin(0), m_chromaOffset(0)
{
  this->setObjectName("MkvCutter-Main");
  m_mkvinfoAnalyser = new MkvInfoSourceAnalyser(this);
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(keyFrameInfos(QStringList)), this,
                  SLOT(setKeyFrames(QStringList)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(videoTrackID(int)), this, SLOT(setVideoTrackID(int)));
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
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(x264Settings(QString)), this,
                  SLOT(setX264Settings(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(aspectRatio(double)), this,
                  SLOT(setAspectRatio(double)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(refframes(int)), this, SLOT(setAvcRefFrames(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(cabac(bool)), this, SLOT(setAvcCabac(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(frameRateMode(bool)), this,
                  SLOT(setFrameRateMode(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(interlaced(QString)), this,
                  SLOT(setInterlaced(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(audioFormat(QString)), this,
                  SLOT(setAudioFormat(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(minKeyInt(QString)), this,
                  SLOT(setMinKeyInt(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(maxKeyInt(QString)), this,
                  SLOT(setMaxKeyInt(QString)));

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
  m_timeextractor = new MkvTimeExtractor(this);
  this->myconnect(m_timeextractor, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_timeextractor, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_timeextractor, SIGNAL(timecodes(QString)), this, SLOT(setTimecodes(QString)));
  this->myconnect(m_timeextractor, SIGNAL(finished(int)), this,
                  SLOT(finishedTimeCodeExtraction(int)));
  this->myconnect(m_timeextractor, SIGNAL(progress(int)), this, SLOT(mkvExtractorProgress(int)));
  m_h264Parser = new H264Parser(this);
  this->myconnect(m_h264Parser, SIGNAL(sendInfo(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_h264Parser, SIGNAL(finished()), this, SLOT(h264ParseFinished()));
  this->myconnect(m_h264Parser, SIGNAL(refframes(int)), this, SLOT(setAvcRefFrames(int)));
  this->myconnect(m_h264Parser, SIGNAL(weightedP(int)), this, SLOT(setWeightedP(int)));
  this->myconnect(m_h264Parser, SIGNAL(weightedB(int)), this, SLOT(setWeightedB(int)));
  this->myconnect(m_h264Parser, SIGNAL(bframes(int)), this, SLOT(setBFrames(int)));
  this->myconnect(m_h264Parser, SIGNAL(qpMin(int)), this, SLOT(setQPmin(int)));
  this->myconnect(m_h264Parser, SIGNAL(chromaOffset(int)), this, SLOT(setChromaOffset(int)));

  m_viewer = 0;
  ui.setupUi(this);
  ui.mainStackedWidget->setCurrentIndex(0);
  QObject::connect(ui.openSourcePushButton, SIGNAL(droppedInput(QString)), this,
                   SLOT(setInput(QString)));
  ui.openSourcePushButton->acceptDrops(true);
  QString tmp = Globals::getDirectory(qApp->applicationFilePath());
  tmp += QDir::separator();
  tmp += "LSMASHSource.dll";
  tmp = QDir::toNativeSeparators(tmp);
  m_useLibAV = QFile::exists(tmp);
  if (!m_useLibAV) {
    QMessageBox::information(this, "ARGH", tr("%1 doesn't exist!").arg(tmp));
    m_ffindexCaller = new FFIndexCaller(this);
    this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
    this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
    this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
    this->myconnect(m_ffindexCaller, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  } else {
    this->addInfo(tr("found %1").arg(tmp));
  }
  Globals::initDecimalFractionHashs();
}

MkvCutter::~MkvCutter()
{
  this->reset();
}

void MkvCutter::setWeightedP(int value)
{
  m_weightedP = value;
  this->addInfo(" " + tr("weigthed p-frames: %1").arg(value));
}
void MkvCutter::setWeightedB(int value)
{
  m_weightedB = value;
  this->addInfo(" " + tr("weigthed b-frames: %1").arg(value));
}
void MkvCutter::setBFrames(int value)
{
  m_bframes = value;
  this->addInfo(" " + tr("b-frames: %1").arg(value));
}
void MkvCutter::setChromaOffset(int value)
{
  m_chromaOffset = value;
  this->addInfo(" " + tr("chroma offset: %1").arg(value));
}

void MkvCutter::setQPmin(int value)
{
  m_qpMin = value;
  this->addInfo(" " + tr("qpMin: %1").arg(value));
}

void MkvCutter::setMinKeyInt(QString value)
{
  m_minKey = value;
  this->addInfo(" " + tr("min gop size: %1").arg(value));
}

void MkvCutter::setMaxKeyInt(QString value)
{
  m_maxKey = value;
  this->addInfo(" " + tr("max gop size: %1").arg(value));
}

void MkvCutter::setTimecodes(QString timecodeFile)
{
  m_timecodes = timecodeFile;
  if (QFile::exists(m_timecodes)) {
    this->addInfo(" " + tr("time code file was extracted to: %1").arg(m_timecodes));
  } else {
    this->addInfo(" " + tr("%1 doesn't exist,...").arg(m_timecodes));
    m_timecodes = QString();
  }
}

void MkvCutter::setInterlacedMode(QString interlacedMode)
{
  if (interlacedMode == tr("auto")) {
    m_interlaced = m_mediaInfoScanorder;
  } else {
    m_mediaInfoScanorder = interlacedMode;
    this->addInfo(" " + tr("changed video scan order to: %1").arg(interlacedMode));
  }
}

void MkvCutter::setInterlaced(QString interlaced)
{
  bool mbaff = interlaced == "MBAFF";
  m_interlaced = (mbaff) ? "tff" : interlaced;
  m_paff = interlaced != "progressive" && !mbaff;
  m_mediaInfoScanorder = interlaced;
  this->addInfo(" " + tr("video scan order: %1").arg(interlaced));
}

void MkvCutter::setVideoTrackID(int id)
{
  m_videoTrackID = id;
}

void MkvCutter::setAverageBitrate(int bitrate)
{
  m_averageBitrate = bitrate;
  this->addInfo(" " + tr("video average bitrate: %1").arg(bitrate));
}

void MkvCutter::setSplitFiles(QStringList splitFiles)
{
  m_splitFiles = splitFiles;
  this->addInfo(" " + tr("video splitter created the following files:"));
  int i = 1;
  foreach(QString fileName, splitFiles)
  {
    this->addInfo("  " + tr("%1: %2").arg(i++).arg(fileName));
  }
}

void MkvCutter::setAudioSplitFiles(QStringList splitFiles)
{
  m_audioSplitFiles = splitFiles;
  this->addInfo(" " + tr("audio splitter created the following files:"));
  int i = 1;
  foreach(QString fileName, splitFiles)
  {
    this->addInfo("  " + tr("%1: %2").arg(i++).arg(fileName));
  }
}

void MkvCutter::setFPS(double framerate)
{
  m_fps = framerate;
  this->addInfo(" " + tr("video stream frame rate: %1").arg(m_fps));
}

void MkvCutter::setInput(QString input)
{
  this->reset();
  if (!input.endsWith(".mkv") || input.isEmpty()) { //abort if input does not end with .avs
    QString text = tr("Input needs to be a .mkv file!");
    QMessageBox::critical(this, tr("Error"), text);
    this->reset();
    return;
  }
  m_currentInput = QDir::toNativeSeparators(input); //set current input
  m_mkvinfoAnalyser->analyse(m_currentInput);
}

void MkvCutter::on_openSourcePushButton_clicked()
{
  QString name = tr("Select mkv input file");
  QString select = tr("Input (*.mkv)");
  QString inputPath = QApplication::applicationDirPath();
  QString input = QFileDialog::getOpenFileName(this, name, inputPath, select);
  m_x264Settings = QString();
  this->setInput(input);
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

bool MkvCutter::createLibAVSourceAVS()
{
  m_tempAvs = m_tempFolder;
  QString shortName = Globals::shortFileName(m_currentInput);
  if (m_tempAvs.isEmpty()) {
    m_tempAvs = Globals::getDirectory(shortName);
  }
  m_tempAvs += QDir::separator();
  m_tempAvs += Globals::getFileName(shortName);
  m_tempAvs += ".avs";
  m_tempAvs = QDir::toNativeSeparators(m_tempAvs);

  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  QString path = QDir::toNativeSeparators(inputPath + "LSMASHSource.dll");
  script << "LoadPlugin(\"" + path + "\")";
  QString call = "LWLibavVideoSource(\"" + shortName + "\"";
  call += ", cache=false)";
  script << call;
  return Globals::saveTextTo(script.join("\n"), m_tempAvs) == 0;
}

bool MkvCutter::createAVS()
{
  m_indexFile = m_tempFolder;
  QString shortName = Globals::shortFileName(m_currentInput);
  if (m_indexFile.isEmpty()) {
    m_indexFile = Globals::getDirectory(shortName);
  }
  m_indexFile += QDir::separator();
  m_indexFile += Globals::getFileName(shortName);
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
  QString call = "FFVideoSource(\"" + shortName + "\"";
  call += ", ";
  call += "cachefile=\"" + m_indexFile + "\"";
  //TODO: add fpsnum, fpsden
  call += ", threads=1";
  call += ")";
  script << call;
  return Globals::saveTextTo(script.join("\n"), m_tempAvs) == 0;
}

void MkvCutter::setX264Settings(QString settings)
{
  m_x264Settings = settings;
}

void MkvCutter::mkvAnalysefinished()
{
  ui.infoLabel->setText(tr("MkvInfoAnalyser finished,.."));
  if (m_keyframes.isEmpty()) {
    QMessageBox::critical(this, tr("Error"), tr("No keyframes found in %1!").arg(m_currentInput));
    this->reset();
    return;
  }
  if (m_useLibAV) {
    if (!this->createLibAVSourceAVS()) {
      this->reset();
      return;
    }
  } else if (!this->createAVS()) {
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

void MkvCutter::createAvisynthSkript(QString filename, QString trim)
{
  this->addInfo(" " + tr("createAvisynthSkript(%1, %2)").arg(filename).arg(trim));
  QString avisynthFileName = m_tempFolder;
  avisynthFileName += QDir::separator();
  avisynthFileName += Globals::getFileName(filename);
  avisynthFileName += ".avs";
  avisynthFileName = QDir::toNativeSeparators(avisynthFileName);
  this->addInfo("  " + tr("Avisynth file name: %1").arg(avisynthFileName));
  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  QString path;
  if (m_useLibAV) {
    path = QDir::toNativeSeparators(inputPath + "LSMASHSource.dll");
  } else {
    path = QDir::toNativeSeparators(inputPath + "ffms2.dll");
    if (!QFile::exists(path)
        && QFile::exists(QDir::toNativeSeparators(inputPath + "ffms2-x64.dll"))) {
      path = QDir::toNativeSeparators(inputPath + "ffms2-x64.dll");
    }
  }
  if (path.isEmpty() || !QFile::exists(path)) {
    QMessageBox::critical(this, tr("Error"), tr("Couldn't find avisynth plugins,.."));
    return;
  }
  QString assume;
  bool bff = m_interlaced == "bff" || m_interlaced == "BFF";
  bool tff = m_interlaced == "tff" || m_interlaced == "TFF";
  if (bff) {
    assume = "AssumeBFF()";
  } else if (tff) {
    assume = "AssumeTFF()";
  }
  script << "LoadPlugin(\"" + path + "\")";
  if (m_useLibAV) {
    QString tmp = "LWLibavVideoSource(\"" + filename + "\"";
    if (bff || tff) {
      tmp += ", threads=1";
    }
    tmp += ", cache=false)";
    script << tmp;
  } else {
    script << "FFVideoSource(\"" + filename + "\", threads=1)";
  }
  script << assume;
  script << trim;
  trim = script.join("\n");
  if (Globals::saveTextTo(trim, avisynthFileName) == 0) {
    this->addInfo("  " + tr("Saved avisynth script:"));
    this->addInfo("   ----------------------------");
    this->addInfo(assume);
    this->addInfo(trim);
    this->addInfo("   ----------------------------");
    this->addInfo("  " + tr("to: %1").arg(avisynthFileName));
    m_tempReencodeAvs << avisynthFileName;
    this->createVideoReencodeCall(avisynthFileName);
  } else {
    QMessageBox::critical(this, tr("Error"),
                          tr("createAvisynthSkript: Couldn't create(%1)").arg(avisynthFileName));
    return;
  }
}

cutTyp1 MkvCutter::findCutForFrame(int frame, const bool start)
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
    elems = tmp.split(",");
    currentKey = elems.at(0).toInt();

    if (currentKey < frame) {
      previousKey = currentKey;
      if (keyIndex + 1 < keyCount) {
        continue;
      }
    } // -> current Key >= frame

    if (currentKey == frame) {
      if (keyIndex + 1 < keyCount) {  // key is not last key
        previousKey = currentKey;
        continue;
      } // -> currentKey == frame && key is last key
      cut.prevKey = currentKey;
      cut.nextKey = m_frameCount;
      if (start) {
        cut.cut.end = frame;
      } else {
        continue;
      }
      break;
    } // -> current Key > frame

    if (start) {
      cut.cut.start = frame;
      cut.cut.end = currentKey - 1;
    } else {
      cut.cut.start = previousKey;
      cut.cut.end = frame;
    }
    cut.prevKey = previousKey;
    cut.nextKey = currentKey;
    previousKey = currentKey;
    break;
  }
  if (cut.nextKey == -1) {
    cut.nextKey = m_frameCount;
  }
  //this->addInfo("  => " + tr("findCutForFrame(%1): %2").arg(frame).arg(Globals::cutTyp1ToString(cut)));
  return cut;
}

void MkvCutter::addAudioCut(const int &start, const int &end)
{
  QString startTime, endTime;
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
  this->addInfo(" -> time cut: " + startTime + "-" + endTime);
  m_mkvAudioParts << startTime + "-" + endTime;
}

void MkvCutter::addVideoCut(const int &start, const int &end, const bool &interlaced)
{
  cutTyp1 tempCut;
  cutTyp1 startCut = findCutForFrame(start, true);
  cutTyp1 endCut = findCutForFrame(end, false);
  //this->addInfo(" -> start cut: " + Globals::cutTyp1ToString(startCut));
  //this->addInfo(" -> end cut: " + Globals::cutTyp1ToString(endCut));

  // CUT LIST

  // A: start&end frame are in the same GOP
  // two cuts in one gop
  if (startCut.prevKey == endCut.prevKey && endCut.nextKey == startCut.nextKey) {
    //CUT LIST
    tempCut.cut.start = start * ((interlaced) ? 2 : 1);
    tempCut.cut.end = end * ((interlaced) ? 2 : 1);
    tempCut.prevKey = startCut.prevKey * ((interlaced) ? 2 : 1);
    tempCut.nextKey = startCut.nextKey * ((interlaced) ? 2 : 1);
    this->addInfo("   " + tr("A1: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }
  // two cuts in two adjacent gops
  if (startCut.nextKey == endCut.prevKey) {
    tempCut.cut.start = start * ((interlaced) ? 2 : 1);
    tempCut.cut.end = end * ((interlaced) ? 2 : 1);
    tempCut.prevKey = startCut.prevKey * ((interlaced) ? 2 : 1);
    tempCut.nextKey = endCut.nextKey * ((interlaced) ? 2 : 1);
    this->addInfo("   " + tr("A2: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }

  // B: start&end frame are in different GOPs
  // start cut
  tempCut.cut.start = start * ((interlaced) ? 2 : 1);
  tempCut.cut.end = (startCut.nextKey - 1) * ((interlaced) ? 2 : 1);
  tempCut.prevKey = startCut.prevKey * ((interlaced) ? 2 : 1);
  tempCut.nextKey = startCut.nextKey * ((interlaced) ? 2 : 1);
  this->addInfo(
      "   " + tr("B1: adding startCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);

  // middle cut
  tempCut.cut.start = startCut.nextKey;
  tempCut.cut.end = (endCut.prevKey - 1) * ((interlaced) ? 2 : 1);
  tempCut.prevKey = (startCut.nextKey) * ((interlaced) ? 2 : 1);
  tempCut.nextKey = (endCut.prevKey) * ((interlaced) ? 2 : 1);
  this->addInfo(
      "   " + tr("B2: adding middleCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);

  if (end == endCut.prevKey) {
    tempCut.cut.end = (endCut.prevKey) * ((interlaced) ? 2 : 1);
    this->addInfo(" " + tr("no end cut needed, middle cut ends with end"));
    return;
  }

  // middle&end cut
  if (end == endCut.nextKey - 1) {
    tempCut.cut.start = endCut.prevKey * ((interlaced) ? 2 : 1);
    tempCut.cut.end = end * ((interlaced) ? 2 : 1);
    tempCut.prevKey = endCut.prevKey * ((interlaced) ? 2 : 1);
    tempCut.nextKey = endCut.nextKey * ((interlaced) ? 2 : 1);
    this->addInfo(
        "   " + tr("B3: adding middle&endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }

  // end cut
  tempCut.cut.start = endCut.prevKey * ((interlaced) ? 2 : 1);
  tempCut.cut.end = end * ((interlaced) ? 2 : 1);
  tempCut.prevKey = endCut.prevKey * ((interlaced) ? 2 : 1);
  tempCut.nextKey = endCut.nextKey * ((interlaced) ? 2 : 1);
  this->addInfo("   " + tr("B4: adding endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);
}

void MkvCutter::buildCutList()
{
  this->addInfo("collecting cut list and audio cuts,..");
  m_mkvAudioParts.clear();
  m_cutList.clear();

  QStringList tCuts;
  int start, end;

  bool interlaced = false;
  if (!m_x264Settings.isEmpty()
      && (m_x264Settings.contains("--bff") || m_x264Settings.contains("--tff"))) {
    interlaced = true;
  } else {
    interlaced = m_interlaced != "progressive";
  }
  int outputFrameCount = 0;
  for (int i = 0, c = m_cuts.count(); i < c; ++i) {
    this->addInfo(" current cut: " + m_cuts.at(i));
    tCuts = m_cuts.at(i).split("-");
    start = tCuts.at(0).toInt();
    end = tCuts.at(1).toInt();
    outputFrameCount += end - start;
    this->addAudioCut(start, end);
    this->addVideoCut(start, end, interlaced);
  }
  QStringList elems;
  double audioLength = 0;
  foreach(QString part, m_mkvAudioParts)
  {
    elems = part.split("-");
    audioLength += Globals::timeToSeconds(elems.at(1));
    audioLength -= Globals::timeToSeconds(elems.at(0));
  }
  double videoLength = outputFrameCount / m_fps;

  this->addInfo(
      " -> calculated audio length: " + Globals::secondsToHMSZZZ(audioLength) + ", in seconds: "
          + QString::number(audioLength));
  this->addInfo(
      " -> calculated video length: " + Globals::secondsToHMSZZZ(videoLength) + ", in seconds: "
          + QString::number(videoLength));
}

void MkvCutter::buildTrimAndPartsList()
{
  this->addInfo("building trim and video parts,...");
  m_mkvVideoParts.clear();
  m_trimming.clear();
  int cutStart, cutEnd, prevKey, nextKey, clipStart = 0, clipEnd = m_frameCount, cutLength;
  int lastNextKey = -1, lastStartKey = -1;
  bool append;
  QStringList mkvparts;
  QString name, trim, negReplace;
  cutTyp1 cut;
  int fileIndex = 0;
  for (int i = 0, c = m_cutList.count(); i < c; ++i) {
    cut = m_cutList.at(i);
    this->addInfo(" looking at: " + Globals::cutTyp1ToString(cut));
    cutStart = cut.cut.start;
    cutEnd = cut.cut.end;
    cutLength = cutEnd - cutStart;
    if (cutLength == 0) {
        cutLength = 1;
    }
    prevKey = cut.prevKey;
    nextKey = cut.nextKey;
    //this->addInfo(tr("  cutStart: %1, cutEnd: %2, prevKey: %3, nextKey: %4").arg(cutStart).arg(cutEnd).arg(prevKey).arg(nextKey));
    //this->addInfo(tr("  lastStartKey: %1, lastNextKey: %2, fileStartKey: %3, fileEndKey: %4").arg(lastStartKey).arg(lastNextKey).arg(fileStartKey).arg(fileEndKey));

    if (cutStart == prevKey && cutEnd == nextKey - 1) {
      trim = "KEEP";
      QString part = mkvparts.last();
      part = part.remove(0, part.indexOf("-") + 1);
      if (!name.isEmpty() && m_trimming.value(name) == trim && part.toInt() == prevKey) {
        part = mkvparts.takeLast();
        part = part.remove(part.indexOf("-")+1, part.size());
        part += QString::number(nextKey);
        this->addInfo("   " + tr("removed last, now adding(7) %1 <> %2 for %3").arg(name).arg(trim).arg(part));
        mkvparts.append(part);
        m_trimming.remove(name);
        m_trimming.insert(name, trim);
        continue;
      }
      fileIndex++;
      //this->addInfo("  " + tr("!append, keep whole gop -> fileIndex %1").arg(fileIndex));
      name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex)
          + ".mkv";
      this->addInfo("  " + tr("adding(6) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      mkvparts.append(QString::number(prevKey) + "-" + QString::number(nextKey));
      continue;
    }
    if (prevKey >= nextKey) {
      nextKey = m_frameCount;
      //this->addInfo("  " + tr("prevKey >= nextKey"));
      //this->addInfo("   " + tr("nextKey = m_frameCount(%1)").arg(m_frameCount));
    }
    //bool prevKey_LastStartKey = prevKey == lastStartKey;
    //this->addInfo("  " + tr("prevKey == lastStartKey: %1").arg(prevKey_LastStartKey));
    //bool prevKey_LastNextKey = prevKey <= lastNextKey;
    //this->addInfo("  " + tr("prevKey <= lastNextKey: %1").arg(prevKey_LastNextKey));
    if (prevKey == lastStartKey) {
      //this->addInfo("  " + tr("prevKey == lastStartKey"));
      //this->addInfo("   " + tr("append = true"));
      //this->addInfo("   " + tr("fileEndKey && lastNextKey = nextKey(%1)").arg(nextKey));
      append = true;
      lastNextKey = nextKey;
    } else if (prevKey <= lastNextKey) {
      //this->addInfo("  " + tr("prevKey <= lastNextKey"));
      //this->addInfo("   " + tr("append = true"));
      //this->addInfo("   " + tr("prevKey = lastStartKey(%1)").arg(lastStartKey));
      if (nextKey > lastNextKey) {
        //this->addInfo("   " + tr(" nextKey > lastNextKey -> lastNextKey = nextKey(%1)").arg(nextKey));
        lastNextKey = nextKey;
      }
      append = true;
      prevKey = lastStartKey;
      negReplace = QString::number(prevKey);
    } else {
      append = false;
      lastStartKey = prevKey;
      lastNextKey = nextKey;
    }
    //this->addInfo("  " + tr("append: %1").arg((append) ? "true" : "false"));
    //this->addInfo("  " + tr("File start %1, end: %2 key").arg(fileStartKey).arg(fileEndKey));
    if (!append) {
      fileIndex++;
      name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex)
          + ".mkv";
      //this->addInfo("  " + tr("reencode -> adding %1 for %2-%3").arg(name).arg(cutStart).arg(cutEnd));
    } else if (!mkvparts.isEmpty()) {
      mkvparts.removeLast();
    }
    //this->addInfo("  " + tr("mkv parts append: %1-%2").arg(prevKey).arg(nextKey));
    mkvparts.append(QString::number(prevKey) + "-" + QString::number(nextKey));

    if (!append && (cutStart == clipStart || cutStart == prevKey)) {
      trim = "Trim(0,";
      if (cutEnd == nextKey || cutEnd == clipEnd) {
        trim = "KEEP";
        this->addInfo("  " + tr("adding(1) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
        m_trimming.insert(name, trim);
        continue;
      }
      //now: cutEnd < nextKey
      trim += "length=" + QString::number(cutLength) + ")";
      this->addInfo("  " + tr("adding(2) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      continue;
    }
    //now: cutStart > prevKey/clipStart
    if (append) {
      trim = m_trimming.value(name) + "+Trim(";
      trim = trim.replace(",-1)", "," + negReplace + ")");
    } else {
      trim = "Trim(";
    }
    trim += QString::number(cutStart - prevKey) + ",";
    if (cutEnd == nextKey || cutEnd == clipEnd) {
      trim += "-1)";
      this->addInfo("  " + tr("adding(3) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      continue;
    }
    //now: cutEnd < nextKey
    trim += "length=" + QString::number(cutLength) + ")";
    this->addInfo("  " + tr("adding(4) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
    m_trimming.insert(name, trim);
    continue;
  }
  if (m_trimming.count() == 1) {
    QString trim = m_trimming.value(name);
    this->addInfo("  single trim: " + tr("removing %1 <> %2 from trim list").arg(name).arg(trim));
    m_trimming.clear();
    this->addInfo("  single trim: " + tr("adding(5) %1 <> %2 to trim list").arg(m_currentInput).arg(trim));
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

QString adjustParDotToColon(QString value)
{
  if (value.endsWith(":1000")) {
    QStringList v = value.split(":");
    value = QString::number(v.at(0).toDouble() / v.at(1).toDouble());
  }
  if (value.contains(":")) {
    return value;
  }
  QString par = value;
  if (par.startsWith("0.0000")) {
    return "1";
  } else if (par.startsWith("1.066") || par.startsWith("1.067")) {
    return "16:15";
  } else if (par.startsWith("0.888")) {
    return "8:9";
  } else if (par.startsWith("1.42")) {
    return "64:45";
  } else if (par.startsWith("1.18")) {
    return "32:27";
  } else if (par.startsWith("1.094")) {
    return "128:117";
  } else if (par.startsWith("0.911") || par.startsWith("0.912")) {
    return "4320:4739";
  } else if (par.startsWith("1.458") || par.startsWith("1.459")) {
    return "512:351";
  } else if (par.startsWith("1.215") || par.startsWith("1.22")) {
    return "5760:4739";
  } else if (par.startsWith("1.090") || par.startsWith("1.091")) {
    return "12:11";
  } else if (par.startsWith("0.90") || par.startsWith("0.91")) {
    return "10:11";
  } else if (par.startsWith("1.45")) {
    return "16:11";
  } else if (par.startsWith("1.21")) {
    return "40:33";
  } else if (par.startsWith("1.896")) {
    return "256:135";
  }
  par = "1:1";
  double dvalue = value.toDouble();
  if (dvalue == 0) {
    return par;
  }
  if (dvalue < 1.0) {
    par = "1000:" + QString::number(int(dvalue * 1000));
  } else if (dvalue > 1.0 && dvalue < 3.0) {
    par = QString::number(int(dvalue * 1000)) + ":1000";
  }

  return par;
}

void MkvCutter::createVideoReencodeCall(QString avisynthFile)
{
  this->addInfo(" " + tr("creating x264 reencode call for: %1").arg(avisynthFile));
  QString x264 = QApplication::applicationDirPath() + QDir::separator();
#ifdef Q_OS_WIN32
  x264 += "x264.exe";
#else
  x264 += "x264";
#endif
  x264 = QDir::toNativeSeparators(x264);
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
  int maxBuff = 0;
  int maxRate = 0;
  if (!tmp.isEmpty()) {
    tmp = tmp.remove(0, tmp.indexOf("@") + 2);
    maxBuff = maxMainBuff(tmp);
    maxRate = maxMainRate(tmp);
    tmp = tmp.remove(".");
    tmp = "--level " + tmp;
    call << tmp;
  }
  if (m_x264Settings.isEmpty()) {
    if (maxBuff != 0 && maxRate != 0) {
      if (maxBuff != 0)
        call << "--vbv-bufsize " + QString::number(maxBuff);
      if (maxRate != 0)
        call << "--vbv-maxrate " + QString::number(maxRate);
    }
    if (!m_avcCabac) {
      call << "--no-cabac";
    }
    if (m_interlaced != "progressive") {
      if (m_interlaced == "BFF" || m_interlaced == "bff") {
        call << "--bff";
      } else if (m_interlaced == "TFF" || m_interlaced == "tff") {
        call << "--tff";
      }
    }
    if (m_avcRefFrames != 0) {
      call << "--ref " + QString::number(m_avcRefFrames);
    } else {
      call << "--ref 1";
    }
    if (m_chromaOffset != 0) {
        call << "--chroma-qp-offset " + QString::number(m_chromaOffset);
    }
    call << "--bframes " + QString::number(m_bframes);

    if (m_bframes > 0) {
      if (m_weightedB == 0) {
        call << "--b-pyramid none";
      } else if (m_weightedB == 1) {
        call << "--b-pyramid normal";
      } else {
        call << "--b-pyramid strict";
      }
    }
    call << "--weightp " + QString::number(m_weightedP);
    call << "--stitchable";
    //TODO: bluray check
    if (m_minKey != QString()) {
      call << "--min-keyint " + m_minKey;
    }
    if (m_maxKey != QString()) {
      call << "--keyint " + m_maxKey;
    } else {
      call << "--keyint " + QString::number(m_averageKeyDistance);
    }
    if (m_qpMin > 0) {
      call << "--qpmin " + QString::number(m_qpMin);
    }
  } else {
    call << m_x264Settings;
  }
  call << "--non-deterministic";
  call << "--thread-input";
  call << "--crf 19";
  call << "--demuxer avs";
  call << "--fps " + Globals::decimalToFractionConvert(m_fps);
  QString par = QString::number(m_aspectRatio);
  par = adjustParDotToColon(par);
  if (par != "1:1") {
    call << "--sar " + par;
  }
  tmp = avisynthFile;
  tmp = tmp.remove(tmp.indexOf("."), tmp.size());
  tmp += "_reencode.264";
  m_reencodedVideoFiles << tmp;
  tmp = "-o \"" + tmp + "\"";
  call << tmp;
  tmp = "\"" + avisynthFile + "\"";
  call << tmp;
  tmp = call.join(" ");
  this->addInfo(" -> " + tr("x264 call: %1").arg(tmp));
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
        QMessageBox::critical(this, tr("Error"),
                              tr("Couldn't move %1 to %2").arg(tmp).arg(m_currentOutput));
      } else {
        QMessageBox::information(this, tr("Finished!"),
                                 tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
      }
    } else {
      this->addInfo(" " + tr("audio file: %1").arg(m_audioFile));
      this->addInfo(" " + tr("Muxing audio&video(1),.."));
      m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_currentOutput, m_fps,
                         m_interlaced != "progressive", m_paff);
    }
    this->reset();
    return;
  }

  // generate mkvmerge calls to join all parts
  this->addInfo(" " + tr("Muxing audio&video(2),.."));
  m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_currentOutput, m_fps,
                     m_interlaced != "progressive", m_paff);
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

void MkvCutter::parseOriginal()
{
  if (m_toAnalyse.isEmpty()) {
    this->startVideoReencoding();
    return;
  }
  m_h264Parser->analyse(m_toAnalyse);
}

void MkvCutter::h264ParseFinished()
{
  this->addInfo(" " + tr("h264_parse finished analysing the input,.."));
  this->startVideoReencoding();
}

void MkvCutter::deleteFiles()
{
  this->addInfo(" " + tr("deleting split list elements,..."));
  foreach (QString file, m_splitFiles)
  {
    if (file.isEmpty() || (file == m_currentInput || !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo("  " + tr("deleting video split file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
    }
  }
  this->addInfo(" " + tr("deleting reencoded and extracted elements,..."));
  foreach (QString file, m_reencodedVideoFiles)
  {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo("  " + tr("deleting video file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
    }
  }
  foreach (QString file, m_toDelete)
  {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo("  " + tr("deleting video file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
    }
  }
  foreach (QString file, m_audioSplitFiles)
  {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo(" " + tr("deleting audio file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
    }
  }
  if (!m_audioFile.isEmpty() && m_audioFile != m_currentInput) {
    if (QFile::exists(m_audioFile)) {
      this->addInfo("  " + tr("deleting audio file: %1").arg(m_audioFile));
      if (!QFile::remove(m_audioFile)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(m_audioFile));
      }
    }
  }

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
    QMessageBox::information(this, tr("Finished!"),
                             tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
    this->reset();
    return;
  }

  this->deleteFiles();
  QMessageBox::information(this, tr("Finished!"),
                           tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
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
  foreach (QString file, m_splitFiles)
  {
    toDelete = file;
    if (toDelete == m_currentInput
        || Globals::getWholeFileName(toDelete) == Globals::getWholeFileName(m_currentOutput)) {
      trim = m_trimming.value(m_currentInput);
    } else if (toDelete != m_currentInput) {
      toDelete = Globals::getWholeFileName(file);
      toDelete = toDelete.remove(0, toDelete.indexOf("-") + 1);
      toDelete = Globals::getFileName(m_currentInput) + "_cut_" + toDelete;
      trim = m_trimming.value(toDelete);
    }
    this->addInfo(" " + tr("trim value for %1: %2").arg(toDelete).arg(trim));
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
    this->parseOriginal();
    return;
  }
  QString input = m_extractionFiles.takeFirst();
  QString filename = input;
  filename = filename.remove(filename.lastIndexOf("."), filename.length());
  filename += ".264";
  filename = m_tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
  filename = QDir::toNativeSeparators(filename);
  if (m_toAnalyse.isEmpty()) {
    m_toAnalyse = filename;
  }
  m_toDelete << filename;
  m_reencodedVideoFiles.replace(m_reencodedVideoFiles.indexOf(input), filename);
  m_extractor->startExtraction(input, QString::number(m_videoTrackID), "264", m_tempFolder);
}

void MkvCutter::extractTimeCodes()
{
  m_timeextractor->startExtraction(m_currentInput, QString::number(m_videoTrackID), m_tempFolder);
}

void MkvCutter::finishedTimeCodeExtraction(int state)
{
  if (state < 0) {
    this->addInfo(tr("Resetting since time code extraction crashed,.."));
    this->reset();
    return;
  }

  if (!m_useLibAV) {
    ui.infoLabel->setText(tr("Indexing input file,.."));
    m_ffindexCaller->index(m_currentInput, m_indexFile);
  } else {
    this->startViewer();
  }
}

void MkvCutter::mediaInfoFinished(int exitstate)
{
  ui.infoLabel->setText(tr("MediaInfo analysis finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since mediaingo analyzer crashed,.."));
    this->reset();
    return;
  }
  if (m_vfr) {
    ui.infoLabel->setText(tr("extracting time codes with mkvextract,.."));
    this->extractTimeCodes();
    return;
  }
  if (!m_useLibAV) {
    ui.infoLabel->setText(tr("Indexing input file,.."));
    m_ffindexCaller->index(m_currentInput, m_indexFile);
    return;
  } else {
    this->startViewer();
  }
}

void MkvCutter::startViewer()
{
  delete m_viewer;
  QStringList keyframes;
  foreach(QString key, m_keyframes)
  {
    key = key.remove(key.indexOf(","), key.size());
    //this->addInfo("key "+key);
    keyframes << key;
  }
  m_viewer = new AVSViewer(this, m_tempAvs, m_aspectRatio, true, keyframes);
  this->myconnect(m_viewer, SIGNAL(finished(int)), this, SLOT(avsViewerFinished(int)));
  this->myconnect(m_viewer, SIGNAL(cuts(QStringList)), this, SLOT(setCutList(QStringList)));
  this->myconnect(m_viewer, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_viewer, SIGNAL(setInterlacedMode(QString)), this,
                  SLOT(setInterlacedMode(QString)));
  ui.avsViewerVerticalLayout->insertWidget(0, m_viewer);
  ui.mainStackedWidget->setCurrentIndex(1);
  ui.infoLabel->setText(tr("- Cut View -"));
  m_viewer->init();
}

void MkvCutter::ffIndexerFinished(int exitstate)
{
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since ffindexer crashed,.."));
    this->reset();
    return;
  }
  ui.infoLabel->setText(tr("Indexing input file finished,.."));
  this->startViewer();
}

QString MkvCutter::cutTimecodes(QString timecodes)
{
  QStringList outputLines;
  QStringList lines = timecodes.split("\n");
  //TODO: cut timecodes based on m_mkvAudioParts
  foreach (QString line, lines)
  {
    this->addInfo("Looking at: " + line);
    outputLines << line;
  }
  return outputLines.join("\r\n");
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
  this->buildCutList();
  if (!m_timecodes.isEmpty()) {
    QString text = Globals::readAll(m_timecodes, "auto");
    text = this->cutTimecodes(text);
    QFile::remove(m_timecodes);
    if (Globals::saveTextTo(text, m_timecodes) == 0) {
      this->addInfo(tr("Successfully cut and saved timecodes, to: %1").arg(m_timecodes));
    }
  }
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
  this->addInfo(" m_mkvVideoParts:\n" + m_mkvVideoParts.join("\n "));

  m_mkvVideoSplitCaller->setKeepIntermediate(ui.keepIntermediateCheckBox->isChecked());
  m_mkvVideoSplitCaller->start(m_currentInput, m_currentOutput, m_mkvVideoParts, m_tempFolder);
}

void MkvCutter::cutAudio()
{
  this->addInfo(tr("Calling audio cutter,.."));
  m_audioFile = m_tempFolder + QDir::separator() + Globals::getWholeFileName(m_currentOutput);
  m_audioFile = m_audioFile.insert(m_audioFile.lastIndexOf("."), "_AudioCut");
  m_audioFile = QDir::toNativeSeparators(m_audioFile);
  m_mkvAudioCutCaller->setKeepIntermediate(ui.keepIntermediateCheckBox->isChecked());
  m_mkvAudioCutCaller->start(m_currentInput, m_currentOutput, m_mkvAudioParts, m_tempFolder, true);
}

void MkvCutter::setKeyFrames(QStringList list)
{
  ui.infoLabel->setText(tr("Got key frame list from mkvinfo analyzer."));
  int count = list.count();
  int dist = m_frameCount / count;
  m_keyframes = list;
  m_averageKeyDistance = dist;
  this->addInfo(
      " " + tr("video stream key frame count: %1, average distance: %2").arg(count).arg(dist));
}

void MkvCutter::setFrameRateMode(bool vfr)
{
  m_vfr = vfr;
  this->addInfo(" " + tr("frame rate mode: %1").arg(vfr ? "vfr" : "cfr"));
}

void MkvCutter::setAspectRatio(double aspect)
{
  m_aspectRatio = aspect;
  this->addInfo(" " + tr("aspect ratio of input: %1").arg(aspect));
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
  bool keepIntermediate = ui.keepIntermediateCheckBox->isChecked();
  if (!m_tempAvs.isEmpty()) {
    if (!keepIntermediate) {
      this->addInfo(tr("Deleting %1,..").arg(m_tempAvs));
      QFile::remove(m_tempAvs);
    }
    m_tempAvs = QString();
  }
  if (!m_indexFile.isEmpty()) {
    if (!keepIntermediate) {
      QFile::remove(m_indexFile);
    }
    m_indexFile = QString();
  }
  foreach(QString file, m_tempReencodeAvs)
  {
    if (!keepIntermediate) {
      QFile::remove(file);
    }
  }
  m_currentInput = QString();
  m_currentOutput = QString();
  m_tempFolder = QString();
  m_avcProfileLevel = QString();
  m_audioFormat = QString();
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
  m_fps = -1;
  m_trimming.clear();
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
  ui.infoLabel->setText(QString());
  ui.outputLabel->setText(QString());
  ui.tempFolderLabel->setText(QString());
  m_aspectRatio = 1;
  m_interlaced = "progressive";
  m_mediaInfoScanorder = QString();
  m_paff = false;
  m_vfr = false;
  m_x264Settings = QString();
  m_timecodes = QString();
  m_minKey = QString();
  m_maxKey = QString();
  m_weightedP = 0;
  m_weightedB = 0;
  m_bframes = 0;
  m_qpMin = 0;
  m_averageKeyDistance = 0;
  m_toAnalyse = QString();
}

void MkvCutter::setCutList(QStringList cuts)
{
  m_cuts = cuts;
}

void MkvCutter::setFrameCount(int count)
{
  m_frameCount = count;
  this->addInfo(" " + tr("frame count: %1").arg(count));
}

void MkvCutter::setAvcProfileLevel(QString pl)
{
  m_avcProfileLevel = pl;
  this->addInfo(" " + tr("profile@Level: %1").arg(pl));
}

void MkvCutter::setAvcCabac(bool cabac)
{
  m_avcCabac = cabac;
  this->addInfo(" " + tr("cabac: %1").arg(cabac));
}

void MkvCutter::setAvcRefFrames(int frames)
{
  m_avcRefFrames = frames;
  this->addInfo(" " + tr("reference frames: %1").arg(frames));
}

void MkvCutter::setAudioFormat(QString format)
{
  m_audioFormat = format;
  this->addInfo(" " + tr("audio format: %1").arg(format));
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
