#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QApplication>
#include <QFileInfo>
#include <iostream>
#include <QDateTime>
#include "Globals.h"

using namespace std;

MkvCutter::MkvCutter(QWidget *parent)
    : QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_currentOutput(QString()),
        m_tempFolder(QString()), m_avcProfileLevel(QString("High@L4.1")), m_audioFormat(QString()),
        m_avcCabac(true), m_avcRefFrames(1), m_enabled(0), m_frameCount(0), m_frameScale(1), m_keyframes(), m_cuts(),
        m_splitFiles(), m_tempReencodeAvs(), m_videoEncodingCalls(), m_reencodedVideoFiles(),
        m_fps(-1), m_trimming(), m_cutList(), m_mkvmergeIntSplitList(), m_mkvinfoAnalyser(nullptr),
        m_mediaInfoAnalyser(nullptr), m_viewer(nullptr), m_mkvVideoSplitCaller(nullptr),
        m_mkvAudioCutCaller(nullptr), m_mkvMerger(nullptr), m_x264(nullptr), m_mkvVideoParts(),
        m_mkvAudioAndSubtitleParts(), m_audioFile(QString()), m_averageBitrate(-1),
        m_audioSplitFiles(), m_audioSyncOffsets(), m_chapterFile(QString()),
        m_extractionFiles(), m_toDelete(),
        m_videoTrackID(-1),
        m_extractor(nullptr), m_timeextractor(nullptr), m_aspectRatio(1),
        m_interlaced("progressive"), m_mediaInfoScanorder(), m_scanType(), m_chroma("4:2:0"),
        m_bitDepth(8), m_vfr(false), m_timecodes(QString()),
        m_x264Settings(QString()), m_averageKeyDistance(0), m_paff(false), m_minKey(QString()),
        m_maxKey(QString()), m_h264Parser(nullptr), m_weightedP(0), m_weightedB(0), m_bframes(0),
        m_qpMin(0), m_chromaOffset(0), m_toAnalyse(QString()), m_subtitles(),
        m_mkvSubtitleExtractor(nullptr), m_subtitleCutter(nullptr), m_cutSubtitles(),
        m_subtitleToCut(), m_keyframeonly(false), m_hasAudio(false), m_sps(-1), m_width(-1),
        m_height(-1), m_audioDelays(), m_inputTimeCodes(), m_ffindexCaller(nullptr),
        m_cliCutList(QString()), m_cliCommit(false), m_cliNext(false)
{
  this->setObjectName("MkvCutter-Main");
  ui.setupUi(this);
  ui.mainStackedWidget->setCurrentIndex(0);
  this->myconnect(ui.openSourcePushButton, SIGNAL(droppedInput(QString)), this,
      SLOT(setInput(QString)));
  ui.openSourcePushButton->acceptDrops(true);
  Globals::initDecimalFractionHashs();
  this->initTools();
  this->setWindowTitle("Mkv Cutter - " + QString::fromLocal8Bit(BUILDDATE));
}

void MkvCutter::setTheAudioDelays(QHash<QString, QString> audioDelays)
{
  m_audioDelays = audioDelays;
}

void MkvCutter::initTools()
{
  cout << " init tools" << endl;

  cout << "  init m_mkvinfoAnalyser" << endl;
  delete m_mkvinfoAnalyser;
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
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(subtitleTrack(SubtitleTrack)), this,
      SLOT(subtitleTrack(SubtitleTrack)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(avcProfileLevel(QString)), this,
      SLOT(setAvcProfileLevel(QString)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(resolution(QString, QString)), this,
      SLOT(setResolution(QString, QString)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(hasAudio(bool)), this, SLOT(setHasAudio(bool)));
  cout << "  init m_mediaInfoAnalyser" << endl;
  delete m_mediaInfoAnalyser;
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
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(scanType(QString)), this,
      SLOT(setScanType(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(bitDepth(int)), this, SLOT(setBitDepth(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(chromaSubsampling(QString)), this,
      SLOT(setChromaSubsampling(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(audioFormat(QString)), this,
      SLOT(setAudioFormat(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(minKeyInt(QString)), this,
      SLOT(setMinKeyInt(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(maxKeyInt(QString)), this,
      SLOT(setMaxKeyInt(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(theAudioDelays(QHash<QString, QString>)), this,
      SLOT(setTheAudioDelays(QHash<QString, QString>)));
  cout << "  init m_mkvVideoSplitCaller" << endl;
  delete m_mkvVideoSplitCaller;
  m_mkvVideoSplitCaller = new MkvSplitCaller(this);
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(finished(int)), this, SLOT(mkvSplitFinished(int)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(progress(int)), this, SLOT(mkvsplitProgress(int)));
  this->myconnect(m_mkvVideoSplitCaller, SIGNAL(splitFiles(QStringList)), this,
      SLOT(setSplitFiles(QStringList)));
  cout << "  init m_mkvAudioCutCaller" << endl;
  delete m_mkvAudioCutCaller;
  m_mkvAudioCutCaller = new MkvSplitCaller(this);
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(finished(int)), this, SLOT(mkvAudioCutFinished(int)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(progress(int)), this, SLOT(mkvsplitProgress(int)));
  this->myconnect(m_mkvAudioCutCaller, SIGNAL(splitFiles(QStringList)), this,
      SLOT(setAudioSplitFiles(QStringList)));
  cout << "  init m_mkvMerger" << endl;
  delete m_mkvMerger;
  m_mkvMerger = new MkvMerger(this);
  this->myconnect(m_mkvMerger, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvMerger, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvMerger, SIGNAL(finished(int)), this, SLOT(mkvMergerFinished(int)));
  this->myconnect(m_mkvMerger, SIGNAL(progress(int)), this, SLOT(mkvMergerProgress(int)));
  cout << "  init m_x264" << endl;
  delete m_x264;
  m_x264 = new X264Caller(this);
  this->myconnect(m_x264, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_x264, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_x264, SIGNAL(finished(int)), this, SLOT(x264Finished(int)));
  this->myconnect(m_x264, SIGNAL(progress(int)), this, SLOT(x264Progress(int)));
  cout << "  init video extractor" << endl;
  delete m_extractor;
  m_extractor = new FFmpegVideoExtractor(this);
  this->myconnect(m_extractor, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_extractor, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_extractor, SIGNAL(finished(int)), this, SLOT(mkvExtractorFinished(int)));
  this->myconnect(m_extractor, SIGNAL(progress(int)), this, SLOT(mkvExtractorProgress(int)));
  cout << "  init m_timeextractor" << endl;
  delete m_timeextractor;
  m_timeextractor = new MkvTimeExtractor(this);
  this->myconnect(m_timeextractor, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_timeextractor, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_timeextractor, SIGNAL(timecodes(QString)), this, SLOT(setTimecodes(QString)));
  this->myconnect(m_timeextractor, SIGNAL(finished(int)), this,
      SLOT(finishedTimeCodeExtraction(int)));
  this->myconnect(m_timeextractor, SIGNAL(progress(int)), this, SLOT(mkvExtractorProgress(int)));
  cout << "  init m_h264Parser" << endl;
  delete m_h264Parser;
  m_h264Parser = new H264Parser(this);
  this->myconnect(m_h264Parser, SIGNAL(sendInfo(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_h264Parser, SIGNAL(finished()), this, SLOT(h264ParseFinished()));
  this->myconnect(m_h264Parser, SIGNAL(refframes(int)), this, SLOT(setAvcRefFrames(int)));
  this->myconnect(m_h264Parser, SIGNAL(weightedP(int)), this, SLOT(setWeightedP(int)));
  this->myconnect(m_h264Parser, SIGNAL(weightedB(int)), this, SLOT(setWeightedB(int)));
  this->myconnect(m_h264Parser, SIGNAL(bframes(int)), this, SLOT(setBFrames(int)));
  this->myconnect(m_h264Parser, SIGNAL(qpMin(int)), this, SLOT(setQPmin(int)));
  this->myconnect(m_h264Parser, SIGNAL(chromaOffset(int)), this, SLOT(setChromaOffset(int)));
  this->myconnect(m_h264Parser, SIGNAL(sps(int)), this, SLOT(setSps(int)));
  cout << "  init m_mkvSubtitleExtractor" << endl;
  delete m_mkvSubtitleExtractor;
  m_mkvSubtitleExtractor = new MkvSubtitleExtractor(this);
  this->myconnect(m_mkvSubtitleExtractor, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvSubtitleExtractor, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvSubtitleExtractor, SIGNAL(finished(int)), this,
      SLOT(mkvSubtitleExtractorFinished(int)));
  this->myconnect(m_mkvSubtitleExtractor, SIGNAL(progress(int)), this,
      SLOT(mkvExtractorProgress(int)));
  cout << "  init m_subtitleCutter" << endl;
  delete m_subtitleCutter;
  m_subtitleCutter = new SubtitleCutter(this);
  this->myconnect(m_subtitleCutter, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_subtitleCutter, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_subtitleCutter, SIGNAL(finished(int)), this, SLOT(mkvSubtitleCutterFinished(int)));
  this->myconnect(m_subtitleCutter, SIGNAL(progress(int)), this, SLOT(mkvExtractorProgress(int)));
  if (m_viewer != nullptr) {
    cout << "  reset m_viewer" << endl;
    delete m_viewer;
    m_viewer = nullptr;
  }
  QString tmp = Globals::getDirectory(qApp->applicationFilePath());
  tmp += QDir::separator();
  tmp += "LSMASHSource.dll";
  tmp = QDir::toNativeSeparators(tmp);
  // Wie bei allen anderen Werkzeugen: erst weg, dann ggf. neu. Frueher wurde der Zeiger nur
  // im Nicht-LSMASH-Fall gesetzt und sonst nie -- er behielt dann den alten Wert (beim
  // ersten Aufruf uninitialisierter Speicher). 'm_ffindexCaller == nullptr' ist aber genau
  // die Abfrage, an der ueberall der LWLibav-Weg haengt, und spaeter wurde auf dem Zeiger
  // index() gerufen. Mit LSMASHSource.dll neben der EXE stuerzte die Anwendung deshalb
  // reproduzierbar nach der Timecode-Extraktion ab.
  delete m_ffindexCaller;
  m_ffindexCaller = nullptr;
  if (!QFile::exists(tmp)) {
    cout << "  init ffmindexCaller" << endl;
    m_ffindexCaller = new FFIndexCaller(this);
    this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
    this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
    this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
    this->myconnect(m_ffindexCaller, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  }
  cout << " finished initializing tools" << endl;
}

MkvCutter::~MkvCutter()
{
  this->reset(false);
}

void MkvCutter::subtitleTrack(SubtitleTrack track)
{
  this->addInfo(
      " " + tr("subtitle track: %1, language: %2").arg(track.trackID).arg(track.language));
  m_subtitles.append(track);
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
  if (interlacedMode.compare("auto", Qt::CaseInsensitive) == 0) {
    m_interlaced = m_mediaInfoScanorder;
    this->addInfo(
        " " + tr("reset video scan order to the detected value: %1").arg(m_interlaced));
  } else {
    m_interlaced = interlacedMode;
    this->addInfo(" " + tr("changed video scan order to: %1").arg(interlacedMode));
  }
}

void MkvCutter::setInterlaced(QString interlaced)
{
  // 'interlaced' ist die Feldreihenfolge (progressive/TFF/BFF). Ob die Quelle MBAFF oder
  // feldcodiert ist, steht im Scan-*Type* und kommt ueber setScanType(); m_paff wird
  // deshalb erst in mediaInfoFinished() bestimmt, wenn beide Werte vorliegen.
  m_interlaced = interlaced;
  // gemappten Wert merken, damit "auto" im Viewer genau die Erkennung wiederherstellt
  m_mediaInfoScanorder = m_interlaced;
  this->addInfo(" " + tr("video scan order: %1").arg(interlaced));
}

void MkvCutter::setScanType(QString type)
{
  m_scanType = type;
  this->addInfo(" " + tr("video scan type: %1").arg(type.isEmpty() ? "-" : type));
}

void MkvCutter::setBitDepth(int bits)
{
  // Fallback auf 8, falls MediaInfo nichts liefert.
  m_bitDepth = (bits > 0) ? bits : 8;
  this->addInfo(" " + tr("video bit depth: %1").arg(m_bitDepth));
}

void MkvCutter::setChromaSubsampling(QString chroma)
{
  m_chroma = chroma.trimmed().isEmpty() ? QString("4:2:0") : chroma.trimmed();
  this->addInfo(" " + tr("chroma subsampling: %1").arg(m_chroma));
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
  m_currentOutput = m_currentInput;
  m_currentOutput = m_currentOutput.insert(m_currentOutput.lastIndexOf("."),
      "_" + QDateTime::currentDateTime().toString("hh_mm_ss"));
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
  ui.infoLabel->setText(tr("MkvMerge merging at %1%").arg(percent));
}
void MkvCutter::mkvExtractorProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvExtractor at %1%").arg(percent));
}

void MkvCutter::mkvsplitProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvMerge splitting at %1%").arg(percent));
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
  QString apath = QDir::toNativeSeparators(inputPath + "waveform.dll");
  script << "LoadPlugin(\"" + apath + "\")";
  script << "function m4(float x) {return(x<16?16:int(round(x/4.0)*4))}";
  QString call = "V = LWLibavVideoSource(\"" + shortName + "\"";
  // Fuer die Vorschau reichen 8 Bit, also die Quelle gleich so anfordern statt spaeter
  // umzurechnen. Bittiefe kommt aus MediaInfo, nicht aus dem Profilnamen.
  if (m_bitDepth > 8) {
    call += ", format=\"YUV420P8\"";
  }
  call += ", cache=false";
  // ConvertBits(8) auch hier, aus demselben Grund wie im FFMS2-Zweig: ConvertToYV12()
  // lehnt mehr als 8 Bit ab. Bei erkanntem High10 liefert die Quelle oben bereits
  // YUV420P8, dann ist es ein No-Op -- es deckt aber die Faelle ab, in denen die Bittiefe
  // nicht am Profilstring zu erkennen ist (12/16 Bit, High 4:2:2).
  call += ").ConvertBits(8).ConvertToYv12()";
  script << call;

  // Audiospur nur anhaengen, wenn es eine gibt -- siehe die gleiche Stelle in createAVS().
  if (m_hasAudio) {
    script << QString("A = LWLibavAudioSource(\"%1\", cache=false).ConvertToMono").arg(shortName);
  }
  QString resizer = "V = V.BicubicResize(Ceil(V.Width*" + QString::number(m_aspectRatio) +  ")-(Ceil(V.Width*" + QString::number(m_aspectRatio) + ")) % 4, V.Height)";
  script << resizer;
  if (m_hasAudio) {
    script << QString("AudioDub(V,A).WaveForm(window=1, height=m4(V.Height/8.0)).ConvertToYv12()");
  } else {
    script << "V.ConvertToYv12()";
  }
  return Globals::saveTextTo(script.join("\n"), m_tempAvs) == 0;
}

bool MkvCutter::createAVS()
{

  QString name = Globals::getFileName(m_currentInput);
  QString shortName = Globals::shortFileName(m_currentInput);
  m_indexFile = m_tempFolder;
  if (m_indexFile.isEmpty()) {
    m_indexFile = Globals::getDirectory(shortName);
  }
  m_indexFile += QDir::separator();
  m_indexFile += name;
  m_tempAvs = m_indexFile + ".avs";
  m_indexFile += ".ffindex";
  m_indexFile = QDir::toNativeSeparators(m_indexFile);

  // QMessageBox::information(this, tr("Notice"), QString("current input: %1, name: %2, shotName: %3 => indexFile: %4").arg(m_currentInput).arg(name).arg(shortName).arg(m_indexFile));
  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  QString path = QDir::toNativeSeparators(inputPath + "ffms2.dll");
  script << "LoadPlugin(\"" + path + "\")";
  QString apath = QDir::toNativeSeparators(inputPath + "waveform.dll");
  script << "LoadPlugin(\"" + apath + "\")";
  script << "function m4(float x) {return(x<16?16:int(round(x/4.0)*4))}";
  QString call = "V = FFVideoSource(\"" + shortName + "\"";
  call += ", ";
  call += "cachefile=\"" +  m_indexFile + "\"";
  call += ", threads=1";
  // ConvertToYV12() lehnt Quellen mit mehr als 8 Bit ab ("only 8 bit sources allowed"),
  // eine High10-Quelle scheiterte hier also schon in der Vorschau. ConvertBits(8) davor
  // ist bei 8-Bit-Quellen ein No-Op und deckt neben 10 auch 12/16 Bit ab. Fuer die
  // Vorschau genuegen 8 Bit; der Re-Encode-Pfad bekommt die Quelle davon unberuehrt.
  call += ").ConvertBits(8).ConvertToYv12()";
  script << call;
  // Die Audiospur nur anhaengen, wenn es ueberhaupt eine gibt. Sonst bricht die Vorschau
  // schon beim Import ab ("FFAudioSource: No audio track found") -- m_hasAudio steht zu
  // diesem Zeitpunkt aus der mkvinfo-Analyse laengst fest.
  if (m_hasAudio) {
    script << QString("A = FFAudioSource(\"%1\", cache=false).ConvertToMono").arg(shortName);
  }
  QString resizer = "V = V.BicubicResize(Ceil(V.Width*" + QString::number(m_aspectRatio)
      + ")-(Ceil(V.Width*" + QString::number(m_aspectRatio) + ")) % 4, V.Height)";
  script << resizer;
  if (m_hasAudio) {
    script << QString("AudioDub(V,A).WaveForm(window=1, height=m4(V.Height/8.0))");
  } else {
    script << "V";
  }
  return Globals::saveTextTo(script.join("\n"), m_tempAvs) == 0;
}

void MkvCutter::setX264Settings(QString settings)
{
  this->addInfo(tr("Found x264 encoding settings!"));
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

  // Genau *ein* Vorschau-Script erzeugen. Liegt LSMASHSource.dll neben der EXE, gibt es
  // keinen FFIndexCaller und damit auch kein .ffindex -- dann muss der LWLibav-Weg her.
  // Vorher lief createAVS() unbedingt und ueberschrieb m_tempAvs auch dann, wenn
  // createLibAVSourceAVS() gerade ein LWLibav-Script geschrieben hatte: die Vorschau
  // verwies anschliessend auf eine .ffindex-Datei, die mangels FFIndexCaller nie jemand
  // erzeugt hat.
  bool avsCreated = false;
  if (m_ffindexCaller == nullptr) {
    this->addInfo(tr("LSMASHSource.dll found -> using LWLibavVideoSource for the preview"));
    avsCreated = this->createLibAVSourceAVS();
  } else {
    avsCreated = this->createAVS();
  }
  if (!avsCreated) {
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
 * Erzeugt den Trim-AviSynth-Script fuer eine Split-Datei.
 * Liefert false, wenn kein Script entstanden ist -- der Aufrufer muss dann abbrechen:
 * ohne Script wird der Teil weder neu codiert noch sonst irgendwo eingetragen, und die
 * Ausgabe waere still unvollstaendig.
 **/
bool MkvCutter::createAvisynthSkript(QString filename, QString trim)
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
  QString path = QDir::toNativeSeparators(inputPath + "LSMASHSource.dll");
  if (m_ffindexCaller == nullptr) {
    path = QDir::toNativeSeparators(inputPath + "LSMASHSource.dll");
    } else {
    path = QDir::toNativeSeparators(inputPath + "ffms2.dll");
  }

  if (path.isEmpty() || !QFile::exists(path)) {
    this->addInfo("  " + tr("ERROR: couldn't find avisynth plugin %1").arg(path));
    QMessageBox::critical(this, tr("Error"), tr("Couldn't find avisynth plugins,.."));
    return false;
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
  if (m_ffindexCaller == nullptr) {
    QString tmp = "LWLibavVideoSource(\"" + filename + "\"";
    if (bff || tff) {
      tmp += ", threads=1";
    }
    // Format nur bei mehr als 8 Bit erzwingen, und dann mit der *echten* Unterabtastung.
    // Frueher stand hier fest "YUV420P10" -- bei einer 4:4:4- oder 4:2:2-Quelle waere das
    // eine verlustbehaftete Herunterrechnung gewesen, bevor x264 sie ueberhaupt sieht.
    if (m_bitDepth > 8) {
      QString format = m_chroma.contains("4:4:4") ? "YUV444P"
          : (m_chroma.contains("4:2:2") ? "YUV422P" : "YUV420P");
      format += QString::number(m_bitDepth);
      tmp += ", format=\"" + format + "\"";
    }
    //QString tmpFps = Globals::decimalToFractionConvert(m_fps);
    //QStringList fps = tmpFps.split("/");
    //tmp += ", fpsnum=" + fps[0];
    //tmp += ", fpsden=" + fps[1];
    tmp += ", cache=false";
    //tmp += ", repeat=true";
    tmp += ")";
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
    return true;
  }
  this->addInfo("  " + tr("ERROR: couldn't write %1").arg(avisynthFileName));
  QMessageBox::critical(this, tr("Error"), tr("createAvisynthSkript: Couldn't create(%1)").arg(avisynthFileName));
  return false;
}

cutTyp1 MkvCutter::findCutForFrame(int frame, const bool start)
{
  cutTyp1 cut;
  cut.prevKey = -1;
  cut.nextKey = -1;
  cut.cut.start = frame;
  cut.cut.end = -1;
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
  return cut;
}

void MkvCutter::addAudioAndSubtitleCuts(const int &start, const int &end)
{
  QString startTime, endTime;
  if (start == 0) {
    startTime = QString();
  } else {
    startTime = Globals::frameToTime(start, m_fps, &m_inputTimeCodes);
  }
  if (end == 0) {
    endTime = QString();
  } else {
    endTime = Globals::frameToTime(end, m_fps, &m_inputTimeCodes);
  }
  this->addInfo(" -> time cut: " + startTime + "-" + endTime);
  m_mkvAudioAndSubtitleParts << startTime + "-" + endTime;
}

void MkvCutter::addVideoCut(const int &start, const int &end)
{
  cutTyp1 tempCut;
  cutTyp1 startCut = findCutForFrame(start, true);
  cutTyp1 endCut = findCutForFrame(end, false);

  // CUT LIST

  // A: start&end frame are in the same GOP
  // two cuts in one gop
  if (startCut.prevKey == endCut.prevKey && endCut.nextKey == startCut.nextKey) {
    //CUT LIST
    tempCut.cut.start = start;
    tempCut.cut.end = end;
    tempCut.prevKey = startCut.prevKey;
    tempCut.nextKey = startCut.nextKey;
    this->addInfo("   " + tr("A1: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }
  // two cuts in two adjacent gops
  if (startCut.nextKey == endCut.prevKey) {
    tempCut.cut.start = start;
    tempCut.cut.end = end;
    tempCut.prevKey = startCut.prevKey;
    tempCut.nextKey = endCut.nextKey;
    this->addInfo("   " + tr("A2: adding to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }

  // B: start&end frame are in different GOPs
  // Achtung Konvention: cutTyp::end ist -- wie das Ende in der Schnittliste, wie
  // 'mkvmerge --split parts-frames:A-B' und wie Trim(a,length=n) -- *exklusiv*. A1/A2/B3/B4
  // haben das immer so gehandhabt, B1 und B2 setzten dagegen '<keyframe> - 1', also den
  // letzten enthaltenen Frame. buildTrimAndPartsList() rechnet aber durchgehend
  // 'cutLength = cutEnd - cutStart' -- B1 lieferte dadurch eine um eins zu kurze Laenge und
  // verlor das letzte Frame der Start-GOP.
  // start cut
  tempCut.cut.start = start;
  tempCut.cut.end = startCut.nextKey;
  tempCut.prevKey = startCut.prevKey;
  tempCut.nextKey = startCut.nextKey;
  this->addInfo(
      "   " + tr("B1: adding startCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);

  // middle cut -- ebenfalls exklusiv, siehe oben
  tempCut.cut.start = startCut.nextKey;
  tempCut.cut.end = endCut.prevKey;
  tempCut.prevKey = (startCut.nextKey);
  tempCut.nextKey = (endCut.prevKey);
  this->addInfo(
      "   " + tr("B2: adding middleCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);

  if (end == endCut.prevKey) {
    this->addInfo(" " + tr("no end cut needed, middle cut ends with end"));
    return;
  }

  // middle&end cut -- baut denselben Eintrag wie B4 weiter unten, die Unterscheidung dient
  // nur der Logzeile. Bedingung ebenfalls exklusiv gelesen: der Schnitt endet genau am Ende
  // der End-GOP.
  if (end == endCut.nextKey) {
    tempCut.cut.start = endCut.prevKey;
    tempCut.cut.end = end;
    tempCut.prevKey = endCut.prevKey;
    tempCut.nextKey = endCut.nextKey;
    this->addInfo(
        "   " + tr("B3: adding middle&endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
    m_cutList.append(tempCut);
    return;
  }

  // end cut
  tempCut.cut.start = endCut.prevKey;
  tempCut.cut.end = end;
  tempCut.prevKey = endCut.prevKey;
  tempCut.nextKey = endCut.nextKey;
  this->addInfo("   " + tr("B4: adding endCut to cuts: %1").arg(Globals::cutTyp1ToString(tempCut)));
  m_cutList.append(tempCut);
}

void MkvCutter::buildCutList()
{
  this->addInfo("collecting cut list and audio cuts,..");
  m_mkvAudioAndSubtitleParts.clear();
  m_cutList.clear();

  QStringList tCuts;
  int start, end;

  // Frueher wurden bei interlaced Quellen saemtliche Frame- und Keyframe-Nummern verdoppelt.
  // Der Rest der Kette rechnet aber durchgehend in Frames: die Keyframeliste von mkvinfo,
  // m_frameCount, die Schnittliste aus dem Viewer, 'mkvmerge --split parts-frames:' und
  // Trim() im AviSynth-Script. Die verdoppelten Werte waren reine Feldzahlen, die keine
  // dieser Stellen so interpretiert -- gemessen an einer MBAFF-Quelle (1194 Frames) kamen
  // statt 200 nur 178 Frames heraus, und ein Teilbereich lag jenseits des Clipendes.
  int outputFrameCount = 0;
  for (int i = 0, c = m_cuts.count(); i < c; ++i) {
    this->addInfo(" current cut: " + m_cuts.at(i));
    tCuts = m_cuts.at(i).split("-");
    start = tCuts.at(0).toInt();
    end = tCuts.at(1).toInt();
    outputFrameCount += end - start;
    this->addAudioAndSubtitleCuts(start, end);
    this->addVideoCut(start, end);
  }
  if (m_hasAudio) {
    QStringList elems;
    double audioLength = 0;
    foreach(QString part, m_mkvAudioAndSubtitleParts)
    {
      elems = part.split("-");
      audioLength += Globals::timeToSeconds(elems.at(1));
      audioLength -= Globals::timeToSeconds(elems.at(0));
    }
    this->addInfo(
        " -> calculated audio length: " + Globals::secondsToHMSZZZ(audioLength) + ", in seconds: "
            + QString::number(audioLength));
  }
  double videoLength = outputFrameCount / m_fps;
  this->addInfo(
      " -> calculated video length: " + Globals::secondsToHMSZZZ(videoLength) + ", in seconds: "
          + QString::number(videoLength));
}

// Trim() arbeitet auf dem AviSynth-Clip, die Schnittliste und die Keyframes von mkvinfo
// dagegen in Container-Einheiten. Bei feldcodierten (PAFF) Quellen sind das zwei Einheiten
// je Frame -- die fertigen Trim-Angaben werden deshalb zurueckgerechnet (siehe B14).
static QString scaleTrimToClipUnits(const QString &trim, const int scale)
{
  if (scale <= 1 || !trim.startsWith("Trim(")) {
    return trim;
  }
  QStringList scaled;
  foreach(QString piece, trim.split("+"))
  {
    const int open = piece.indexOf("(");
    const int comma = piece.indexOf(",", open);
    const int equal = piece.indexOf("=", comma);
    const int close = piece.lastIndexOf(")");
    if (open < 0 || comma < 0 || equal < 0 || close < 0) {
      scaled << piece;
      continue;
    }
    const int offset = piece.mid(open + 1, comma - open - 1).toInt() / scale;
    int length = piece.mid(equal + 1, close - equal - 1).toInt() / scale;
    if (length < 1) { // ein Schnitt ueber ein einzelnes Feld bleibt ein ganzes Frame
      length = 1;
    }
    scaled << "Trim(" + QString::number(offset) + ",length=" + QString::number(length) + ")";
  }
  return scaled.join("+");
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
  QString name, trim, negReplace, part;
  cutTyp1 cut;
  int fileIndex = 0;
  m_keyframeonly = true;
  QHash<QString, QString> overview;
  for (int i = 0, c = m_cutList.count(); i < c; ++i) {
    cut = m_cutList.at(i);
    cutStart = cut.cut.start;
    cutEnd = cut.cut.end;
    cutLength = cutEnd - cutStart;
    if (cutLength == 0) {
      cutLength = 1;
    }
    prevKey = cut.prevKey;
    nextKey = cut.nextKey;

    // KEEP heisst: der Schnitt deckt die GOP-Gruppe vollstaendig ab. Mit dem exklusiven
    // Ende ist das 'cutEnd == nextKey'; die frühere Prüfung auf 'nextKey - 1' liess einen
    // Schnitt, der einen Frame vor der naechsten GOP endet, als KEEP durchgehen und nahm
    // dieses Frame faelschlich mit.
    if (cutStart == prevKey && cutEnd == nextKey) {
      trim = "KEEP";
      if (!mkvparts.isEmpty()) {
        part = mkvparts.last();
        part = part.remove(0, part.indexOf("-") + 1);
        if (!name.isEmpty() && m_trimming.value(name) == trim && part.toInt() == prevKey) {
          part = mkvparts.takeLast();
          part = part.remove(part.indexOf("-") + 1, part.size());
          if (part == QString::number(nextKey) + "-") {
            m_trimming.remove(name);
            this->addInfo("   " + tr("removed last"));
            continue;
          }
          part += QString::number(nextKey);
          this->addInfo(
              "   "
                  + tr("removed last, now adding(7) %1 <> %2 for %3").arg(name).arg(trim).arg(
                      part));
          mkvparts.append(part);
          m_trimming.remove(name);
          m_trimming.insert(name, trim);
          overview.insert(name, trim + " " + part);
          continue;
        }
      }
      fileIndex++;
      name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex)
          + ".mkv";
      this->addInfo(
          "  " + tr("adding(6) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      part = QString::number(prevKey) + "-" + QString::number(nextKey);
      mkvparts.append(part);
      overview.insert(name, trim + " " + part);
      continue;
    }
    m_keyframeonly = false;
    if (prevKey >= nextKey) {
      nextKey = m_frameCount;
    }
    if (prevKey == lastStartKey) {
      append = true;
      lastNextKey = nextKey;
    } else if (prevKey <= lastNextKey) {
      if (nextKey > lastNextKey) {
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
    if (!append) {
      fileIndex++;
      name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex)
          + ".mkv";
    } else if (!mkvparts.isEmpty()) {
      mkvparts.removeLast();
    }
    part = QString::number(prevKey) + "-" + QString::number(nextKey);
    mkvparts.append(part);
    overview.insert(name, trim + " " + part);
    if (!append && (cutStart == clipStart || cutStart == prevKey)) {
      if (cutStart == prevKey && (cutEnd == nextKey || cutEnd == clipEnd)
          && trim.startsWith("KEEP")) {
        part = mkvparts.takeLast();
        part = part.remove(part.indexOf("-") + 1, part.size());
        if (part == QString::number(nextKey) + "-") {
          m_trimming.remove(name);
          this->addInfo("   " + tr("removed last"));
          continue;
        }
        part += QString::number(nextKey);
        this->addInfo(
            "   "
                + tr("removed last, now adding(8) %1 <> %2 for %3").arg(name).arg(trim).arg(part));
        mkvparts.append(part);
        m_trimming.remove(name);
        m_trimming.insert(name, trim);
        overview.insert(name, trim + " " + part);
        continue;
      }
      trim = "Trim(0,";
      if (cutEnd == nextKey || cutEnd == clipEnd) {
        trim = "KEEP";
        this->addInfo(
            "  "
                + tr("adding(1) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));

        m_trimming.insert(name, trim);
        overview.insert(name, trim + " " + part);
        continue;
      }
      //now: cutEnd < nextKey
      trim += "length=" + QString::number(cutLength) + ")";
      this->addInfo(
          "  " + tr("adding(2) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      overview.insert(name, trim + " " + part);
      continue;
    }
    //now: cutStart > prevKey/clipStart
    if (append) {
      trim = m_trimming.value(name) + "+Trim(";
      // Das Ersetzen des alten "-1)"-Platzhalters ist gegenstandslos, seit ueberall
      // length= geschrieben wird; die Zeile bleibt nur als Absicherung fuer Trim-Werte,
      // die noch aus einem aelteren Lauf stammen koennten.
      trim = trim.replace(",-1)", "," + negReplace + ")");
    } else {
      trim = "Trim(";
    }
    trim += QString::number(cutStart - prevKey) + ",";
    // Frueher stand hier "-1)", gemeint als "bis zum Ende des Teils". AviSynth liest ein
    // negatives zweites Argument aber als *Anzahl* Frames: Trim(82,-1) liefert genau ein
    // Frame, nicht den Rest. Aufgefallen ist das erst, als B13 diesen Zweig ueberhaupt
    // erreichbar machte -- der Schnitt verlor dadurch 35 von 36 Frames.
    // 'cutLength' ist ohnehin immer die richtige Anzahl, also durchgaengig length= nutzen.
    if (cutEnd == nextKey || cutEnd == clipEnd) {
      trim += "length=" + QString::number(cutLength) + ")";
      this->addInfo(
          "  " + tr("adding(3) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
      m_trimming.insert(name, trim);
      overview.insert(name, trim + " " + part);
      continue;
    }
    //now: cutEnd < nextKey
    trim += "length=" + QString::number(cutLength) + ")";
    this->addInfo(
        "  " + tr("adding(4) %1 <> %2 for %3-%4").arg(name).arg(trim).arg(cutStart).arg(cutEnd));
    m_trimming.insert(name, trim);
    overview.insert(name, trim + " " + part);
    continue;
  }

  QStringList keys = overview.keys(), tmplist1, tmplist2;
  keys.sort();
  fileIndex = 0;
  mkvparts.clear();
  m_trimming.clear();
  QString value, nextValue, tmp;
  int index;
  for (int i = 0, c = keys.count(); i < c; ++i) {
    value = overview.value(keys.at(i));
    for (int j = i + 1; j < c; ++j) {
      nextValue = overview.value(keys.at(j));
      if (!value.startsWith("KEEP") || !nextValue.startsWith("KEEP")) {
        break;
      }
      tmp = value;
      tmp = tmp.remove(0, 4).trimmed();
      tmplist1 = tmp.split("-");
      nextValue = nextValue.remove(0, 4).trimmed();
      tmplist2 = nextValue.split("-");
      if (tmplist1.at(1) == tmplist2.at(0)) {
        value = "KEEP " + tmplist1.at(0) + "-" + tmplist2.at(1);
        i = j;
      }
    }
    ++fileIndex;
    name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(fileIndex)
        + ".mkv";
    if (value.startsWith("KEEP")) {
      value = value.remove(0, 4).trimmed();
      mkvparts.append(value);
      m_trimming.insert(name, "KEEP");
      continue;
    }
    tmp = value;
    index = tmp.lastIndexOf(")");
    tmp = tmp.remove(0, index + 1).trimmed(); // 1400-1526
    value = value.remove(index + 1, value.size()); // Trim(120,length=5)
    mkvparts.append(tmp);
    m_trimming.insert(name, value);
  }
  if (m_trimming.count() == 1) {
    QString trim = m_trimming.value(name);
    this->addInfo("  single trim: " + tr("removing %1 <> %2 from trim list").arg(name).arg(trim));
    m_trimming.clear();
    this->addInfo(
        "  single trim: " + tr("adding(5) %1 <> %2 to trim list").arg(m_currentInput).arg(trim));
    m_trimming.insert(m_currentInput, trim);
  }
  if (mkvparts.count() == 1) {
    if (mkvparts.first().trimmed() == "-") {
      mkvparts.clear();
    }
  }
  if (m_frameScale > 1) {
    foreach(QString trimName, m_trimming.keys())
    {
      const QString clipTrim = scaleTrimToClipUnits(m_trimming.value(trimName), m_frameScale);
      m_trimming.insert(trimName, clipTrim);
      this->addInfo("  " + tr("trim in clip units: %1 <> %2").arg(trimName).arg(clipTrim));
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
  if (qAbs(dvalue) >= 0) {
    return par;
  }
  if (dvalue > 1.0 && dvalue < 3.0) {
    par = "1000:" + QString::number(int(dvalue * 1000));
  } else if (dvalue < 1.0) {
    par = QString::number(int(dvalue * 1000)) + ":1000";
  }

  return par;
}

void MkvCutter::createVideoReencodeCall(QString avisynthFile)
{
  this->addInfo(" " + tr("creating x264 reencode call for: %1").arg(avisynthFile));
  QString base = QApplication::applicationDirPath() + QDir::separator();
  // Bittiefe und Chroma kommen aus MediaInfo, nicht mehr aus dem Profilnamen. Die alte
  // Abfrage auf "High10"/"High 10" verfehlte z.B. "High 4:4:4 Predictive" mit 10 Bit:
  // die Quelle galt als 8 Bit, x264 bekam --demuxer avs und scheiterte an
  // "avs [error]: not supported pixel type: YUV444P10".
  const bool highBitDepth = m_bitDepth > 8;
  const bool is444 = m_chroma.contains("4:4:4");
  const bool is422 = m_chroma.contains("4:2:2");
  // x264 deckt 8 und 10 Bit in einer Binary ab; sein AviSynth-Demuxer kann aber nur
  // 8 Bit (gemessen 2026-09-08/09 mit YUV420P10 und YUV444P10). Ab 10 Bit wandelt deshalb
  // avs2yuv das Script in Rohdaten und pipet sie in x264, das mit --demuxer raw von stdin
  // liest. 8-Bit-Quellen -- auch 4:2:2 und 4:4:4 -- liest x264 direkt ueber --demuxer avs.
  const bool usePipe = highBitDepth;
  QString x264 = base;
#ifdef Q_OS_WIN32
  x264 += "x264.exe";
#else
  x264 += "x264";
#endif
  x264 = QDir::toNativeSeparators(x264);
  QStringList call;
  QString tmp;
  if (usePipe) {
    // 64-Bit-Build bevorzugen, wie FFIndexCaller es mit ffmsindex64.exe auch macht.
    QString avs2yuv = QDir::toNativeSeparators(base + "avs2yuv64.exe");
    if (!QFile::exists(avs2yuv)) {
      avs2yuv = QDir::toNativeSeparators(base + "avs2yuv.exe");
    }
    if (!QFile::exists(avs2yuv)) {
      this->addInfo(
          " " + tr("ERROR: neither avs2yuv64.exe nor avs2yuv.exe found -- sources with more "
                   "than 8 bit cannot be re-encoded."));
      QMessageBox::critical(this, tr("Error"),
          tr("Couldn't find avs2yuv64.exe/avs2yuv.exe, which is required for sources with "
             "more than 8 bit."));
      return;
    }
    call << "\"" + avs2yuv + "\"";
    call << "-raw \"" + avisynthFile + "\"";
    call << "-o -";
    call << "|";
  }
  tmp = "\"" + x264 + "\"";
  call << tmp;
  tmp = "--profile ";
  // 4:4:4 und 4:2:2 brauchen ihr eigenes Profil -- frueher stand hier fuer beide "high",
  // weil m_avcProfileLevel "High 4:4:4 Predictive" enthaelt und auf "High" geprueft wurde.
  if (is444) {
    tmp += "high444";
  } else if (is422) {
    tmp += "high422";
  } else if (highBitDepth) {
    tmp += "high10";
  } else if (m_avcProfileLevel.contains("High", Qt::CaseInsensitive)
      || m_avcProfileLevel.isEmpty()) {
    tmp += "high";
  } else if (m_avcProfileLevel.contains("Base", Qt::CaseInsensitive)) {
    tmp += "baseline";
  } else {
    tmp += "main";
  }
  call << tmp;

  tmp = m_avcProfileLevel;
  int maxBuff = 0;
  int maxRate = 0;
  if (!tmp.isEmpty()) {
    tmp = tmp.remove(0, tmp.indexOf("@") + 2).trimmed();
    maxBuff = maxMainBuff(tmp);
    maxRate = maxMainRate(tmp);
    tmp = tmp.remove(".");
    tmp = "--level " + tmp;
    call << tmp;
  }
  if (m_sps != -1) {
    call << "--sps-id " + QString::number(m_sps);
  }

  if (m_x264Settings.trimmed().isEmpty()) {
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
    call << "--bframes " + QString::number(m_bframes);

    if (m_bframes > 0) {
      if (m_weightedB == 0) {
        call << "--b-pyramid none";
      } else if (m_weightedB == 1) {
        call << "--b-pyramid strict";
      } else {
        call << "--b-pyramid normal";
      }
    }
    call << "--weightp " + QString::number(m_weightedP);
    //TODO: bluray check
    if (m_minKey != QString()) {
      call << "--min-keyint " + m_minKey;
    }
    if (m_maxKey != QString()) {
      call << "--keyint " + m_maxKey;
    } else {
      call << "--keyint " + QString::number(m_averageKeyDistance);
    }
  } else {
    call << m_x264Settings;
  }
  tmp = call.join(" ");
  if (m_chromaOffset != 0 && !tmp.contains("--chroma-qp-offset")) {
    call << "--chroma-qp-offset " + QString::number(m_chromaOffset);
  }
  if (m_qpMin > 0 && !tmp.contains("--qpmin")) {
    call << "--qpmin " + QString::number(m_qpMin);
  }

  call << "--stitchable";
  call << "--non-deterministic";
  call << "--thread-input";
  call << "--crf 19";
  if (usePipe) {
    // Rohdaten aus der avs2yuv-Pipe, siehe oben. avs2yuv schreibt die Ebenen unveraendert,
    // x264 muss also wissen, wie sie zu lesen sind -- ohne --input-csp nimmt der
    // Raw-Demuxer 4:2:0 an und wuerde 4:2:2/4:4:4 falsch interpretieren.
    call << "--demuxer raw";
    call << "--input-depth " + QString::number(m_bitDepth);
    call << "--input-res " + QString::number(m_width) + "x" + QString::number(m_height);
    call << QString("--input-csp ") + (is444 ? "i444" : (is422 ? "i422" : "i420"));
    // --input-depth beschreibt nur die Eingabe. Ohne --output-depth encodiert x264 trotz
    // "--profile high10" nach 8 Bit -- gemessen 2026-09-08: aus einer 10-Bit-Quelle kam
    // eine 8-Bit-Ausgabe (High@L5.1 statt High 10@L5.1). x264 kann als Ausgabe nur 8 oder
    // 10 Bit, tiefere Quellen werden also auf 10 gebracht.
    call << "--output-depth 10";
  } else {
    call << "--demuxer avs";
  }
  // Gegenstueck zu --output-depth: x264 schreibt sonst i420, egal was hereinkam. Gemessen
  // 2026-09-09: aus einer 4:4:4-Quelle kam trotz --profile high444 und --input-csp i444
  // eine 4:2:0-Datei ("High 10@L5" statt "High 4:4:4 Predictive").
  if (is444) {
    call << "--output-csp i444";
  } else if (is422) {
    call << "--output-csp i422";
  }
  // m_fps kommt von mkvinfo und zaehlt bei feldcodierten Quellen Felder je Sekunde;
  // x264 bekommt hier aber den AviSynth-Clip, der Frames zaehlt (siehe B14).
  call << "--fps " + Globals::decimalToFractionConvert(m_fps / m_frameScale);
  QString par = QString::number(m_aspectRatio);
  par = adjustParDotToColon(par);
  if (par != "1:1") {
    call << "--sar " + par;
  }
  tmp = avisynthFile;
  tmp = tmp.remove(tmp.lastIndexOf("."), tmp.size());
  tmp += "_reencode.264";
  m_reencodedVideoFiles << tmp;
  tmp = "-o \"" + tmp + "\"";
  call << tmp;
  if (usePipe) {
    call << "-"; // Eingabe kommt aus der Pipe
  } else {
    tmp = "\"" + avisynthFile + "\"";
    call << tmp;
  }
  tmp = call.join(" ");
  this->addInfo(" -> " + tr("x264 call: %1").arg(tmp));
  m_videoEncodingCalls << tmp;
}

void MkvCutter::startVideoReencoding()
{
  if (m_videoEncodingCalls.isEmpty()) { //encodings finished
    this->addInfo(tr("Finished all the video reencoding,..."));
    // Frueher lief der Keyframe-Pfad hier direkt weiter, weil der Ton beim Videoschnitt
    // mitgeschnitten wurde -- ohne die Moeglichkeit, die Stuecke einzeln zu versetzen
    // (B16). Er wird jetzt wie im Re-Encode-Pfad getrennt geschnitten; den Fall ohne Ton
    // behandelt cutAudio() genauso, wie es dieser Zweig getan hat.
    this->cutAudio();
    return;
  }
  this->addInfo(tr("encoding next file,... (%1 left)").arg(m_videoEncodingCalls.count()));
  m_x264->start(m_videoEncodingCalls.takeFirst());
}

void MkvCutter::cleanUpAndMerge()
{
  this->addInfo(tr("cleanUpAndMerge,..."));
  this->buildChapterFile();
  int videoFileCount = m_reencodedVideoFiles.count();
  int audioFileCount = m_audioSplitFiles.count();
  int subtitleCount = m_cutSubtitles.count();

  this->addInfo(" " + tr("video file count: %1").arg(videoFileCount));
  if (videoFileCount > 0) {
    this->addInfo(" " + tr("video files:\n   ") + m_reencodedVideoFiles.join("\n   "));
  }
  this->addInfo(" " + tr("audio file count: %1").arg(audioFileCount));
  if (audioFileCount > 0) {
    this->addInfo("  " + tr("audio files:\n   ") + m_audioSplitFiles.join("\n   "));
  }
  this->addInfo(" " + tr("subtitle file count: %1").arg(subtitleCount));
  if (subtitleCount > 0) {
    this->addInfo("  " + tr("subtitle files:\n   ") + m_cutSubtitles.join("\n   "));
  }
  if (videoFileCount == 0) {
    // Ohne Videoteile gibt es nichts zu muxen; first() waere hier ein Zugriff auf eine
    // leere Liste. Der Fall wird oben schon als moeglich protokolliert.
    this->addInfo(tr("Resetting since there are no video files to merge,.."));
    QMessageBox::critical(this, tr("Error"),
        tr("No video parts to merge -- see the log for what went wrong."));
    this->reset();
    return;
  }
  QString tmp = m_reencodedVideoFiles.first();
  if (videoFileCount == 1 && tmp.endsWith(".mkv")) {
    if (audioFileCount == 0 && subtitleCount == 0) {
      this->addInfo(" " + tr("no audio&subtitle files present -> renaming videoFile,.."));
      if (!QFile::rename(tmp, m_currentOutput)) {
        QMessageBox::critical(this, tr("Error"), tr("Couldn't move %1 to %2").arg(tmp).arg(m_currentOutput));
      } else {
        QMessageBox::information(this, tr("Finished!"), tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
      }
      this->reset();
    } else {
      if (audioFileCount != 0) {
        this->addInfo(" " + tr("audio file: %1").arg(m_audioFile));
      }
      this->addInfo(" " + tr("Muxing content,.."));
      m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_cutSubtitles, m_currentOutput,
          m_fps, m_interlaced != "progressive", m_paff, m_subtitles, m_audioDelays,
      m_audioSyncOffsets, m_timecodes, m_currentInput, m_chapterFile,
      ui.keepIntermediateCheckBox->isChecked());
    }
    return;
  }

  // generate mkvmerge calls to join all parts
  this->addInfo(" " + tr("Muxing audio&video(2),.."));
  m_mkvMerger->start(m_reencodedVideoFiles, m_audioSplitFiles, m_cutSubtitles, m_currentOutput,
      m_fps, m_interlaced != "progressive", m_paff, m_subtitles, m_audioDelays,
      m_audioSyncOffsets, m_timecodes, m_currentInput, m_chapterFile,
      ui.keepIntermediateCheckBox->isChecked());
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

void MkvCutter::createReencodeCalls()
{
  if (m_tempReencodeAvs.isEmpty()) {
    return;
  }
  QStringList detected;
  detected << "  " + tr("Detected settings:");
  detected << "    AVCProfileLevel: " + m_avcProfileLevel;
  bool hasNoX264Settings = m_x264Settings.trimmed().isEmpty();
  if (hasNoX264Settings) {
    detected << "    Interlaced: " + m_interlaced;
    detected << "    Cabac: " + QString((m_avcCabac) ? "true" : "false");
    detected << "    AVCRefFrames: " + QString::number(m_avcRefFrames);
    detected << "    Chroma offset: " + QString::number(m_chromaOffset);
    detected << "    BFrames: " + QString::number(m_bframes);
    detected << "    WeightedB: " + QString::number(m_weightedB);
    detected << "    WeightP: " + QString::number(m_weightedP);
    detected << "    Min Keyint: " + m_minKey;
    detected << "    Max Keyint: " + m_maxKey;
    detected << "    QPMin: " + QString::number(m_qpMin);
    detected << "    AspectRatio: " + QString::number(m_aspectRatio);
  } else {
    detected << "   x264Settings: " + m_x264Settings;
  }
  detected << "     SPS: " + QString::number(m_sps);
  detected << "     FPS: " + QString::number(m_fps);
  detected << "     Bit depth: " + QString::number(m_bitDepth);
  detected << "     Chroma subsampling: " + m_chroma;
  this->addInfo(detected.join("\n"));
  foreach(QString avsSkript, m_tempReencodeAvs)
  {
    this->createVideoReencodeCall(avsSkript);
  }
}

void MkvCutter::parseOriginal()
{
  if (m_toAnalyse.isEmpty()) {
    std::cerr << "no more to analyse,..." << std::endl;
    this->createReencodeCalls();
    this->startVideoReencoding();
    return;
  }
  m_h264Parser->analyse(m_toAnalyse);
}

void MkvCutter::h264ParseFinished()
{
  this->addInfo(" " + tr("h264_parse finished analysing the input,.."));
  this->createReencodeCalls();
  this->startVideoReencoding();
}

void MkvCutter::deleteFiles()
{
  this->addInfo(" " + tr("deleting split list elements,..."));
  foreach (QString file, m_splitFiles)
  {
    if (file.isEmpty()
        || (file == m_currentInput || !QFile::exists(file) || file == m_currentOutput)) {
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
  foreach (QString file, m_subtitleToCut)
  {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo(" " + tr("deleting subtitle file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
      if (file.endsWith(".idx", Qt::CaseInsensitive)) {
        file = file.remove(file.lastIndexOf("."), file.size());
        file += ".sub";
        QFile::remove(file);
      }
    }
  }
  foreach (QString file, m_cutSubtitles)
  {
    if (file.isEmpty() || (file == m_currentInput && !QFile::exists(file))) {
      continue;
    }
    if (QFile::exists(file)) {
      this->addInfo(" " + tr("deleting cut-subtitle file: %1").arg(file));
      if (!QFile::remove(file)) {
        this->addInfo("   " + tr("Couldn't delete %1!").arg(file));
      }
      if (file.endsWith(".idx", Qt::CaseInsensitive)) {
        file = file.remove(file.lastIndexOf("."), file.size());
        file += ".sub";
        QFile::remove(file);
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
    QMessageBox::information(this, tr("Finished!"), tr("Finished, hopefully %1 was created.").arg(m_currentOutput));
    this->reset();
    return;
  }
  this->deleteFiles();
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
  this->addInfo(tr("video extraction finished,.."));
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since video extractor crashed,.."));
    this->reset();
    return;
  }
  cerr << "PING" << endl;
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
  this->computeAudioSyncOffsets();
  if (!m_subtitles.isEmpty()) {
    m_mkvSubtitleExtractor->startExtraction(m_currentInput, m_subtitles, m_tempFolder);
    return;
  }
  this->cleanUpAndMerge();
}

/**
 * Prueft, ob die Teile das enthalten, was 'parts-frames:' versprochen hat -- und zieht die
 * Trim-Werte nach, wenn nicht.
 *
 * mkvmerge legt Teilgrenzen selbst auf Keyframes und haelt sich dabei nicht immer an die
 * angeforderte Framenummer (B19). Gemessen an einer Quelle mit zwei benachbarten I-Frames:
 * 'parts-frames:40-58' lieferte 17 statt 18 Bloecke, '58-65' dafuer 8 statt 7 -- die Naht lag
 * ein Frame zu frueh. Der Plan stimmte, die Ausfuehrung nicht, und die Ausgabe verlor ein
 * Frame, ohne dass irgendetwas Alarm schlug.
 *
 * Nachgerechnet statt vorhergesagt: die Sollaenge steht in m_mkvVideoParts, die Istlaenge
 * wird gemessen. Stimmen alle Teile, bleibt alles unveraendert -- der eingespielte Weg wird
 * also nicht angefasst. Weicht einer ab, werden die tatsaechlichen Grenzen bestimmt und die
 * Trim-Werte daraus neu gebildet.
 */
void MkvCutter::verifyAndCorrectParts()
{
  if (m_mkvVideoParts.isEmpty() || m_splitFiles.isEmpty() || m_cuts.isEmpty()) {
    return;
  }
  QStringList files = m_splitFiles;
  files.sort();
  QList<int> wanted;
  QStringList elems;
  foreach(QString part, m_mkvVideoParts)
  {
    elems = part.split("-");
    wanted << ((elems.count() == 2) ? (elems.at(1).toInt() - elems.at(0).toInt()) : -1);
  }
  if (wanted.contains(-1)) {
    return;
  }
  this->addInfo(tr("checking the video parts,.."));
  // Im Keyframe-Pfad haengt mkvmerge alle Teile in eine Datei -- dort laesst sich nur die
  // Summe pruefen. Zu korrigieren gibt es da ohnehin nichts, alle Teile sind KEEP.
  if (files.count() != m_mkvVideoParts.count()) {
    if (files.count() != 1) {
      return;
    }
    int total = 0;
    foreach(int count, wanted)
    {
      total += count;
    }
    const int got = Globals::mkvFrameCount(files.first());
    if (got < 0) {
      return;
    }
    if (got != total) {
      this->addInfo(
          " " + tr("WARNING: the parts should hold %1 frames, mkvmerge delivered %2").arg(
              total).arg(got));
    }
    return;
  }

  QList<int> actual;
  bool deviates = false;
  for (int i = 0, c = files.count(); i < c; ++i) {
    const int got = Globals::mkvFrameCount(files.at(i));
    if (got < 0) {
      this->addInfo(
          " " + tr("couldn't measure %1 -> leaving the trim values alone").arg(files.at(i)));
      return;
    }
    actual << got;
    if (got != wanted.at(i)) {
      deviates = true;
      this->addInfo(
          " " + tr("part %1 (%2): %3 frames wanted, %4 delivered").arg(i + 1).arg(
              m_mkvVideoParts.at(i)).arg(wanted.at(i)).arg(got));
    }
  }
  if (!deviates) {
    this->addInfo(" " + tr("all parts hold what was asked for"));
    return;
  }
  this->addInfo(
      tr("WARNING: mkvmerge moved part boundaries -- recomputing the trim values,.."));

  // Tatsaechliche Grenzen: ein Teil beginnt dort, wo er angefordert wurde, ausser er
  // schliesst unmittelbar an seinen Vorgaenger an -- dann beginnt er hinter dessen echtem
  // Ende. Genau dort verschiebt sich die Naht.
  QList<int> starts;
  for (int i = 0, c = m_mkvVideoParts.count(); i < c; ++i) {
    elems = m_mkvVideoParts.at(i).split("-");
    const int askedStart = elems.at(0).toInt();
    if (i == 0) {
      starts << askedStart;
      continue;
    }
    const QString previous = m_mkvVideoParts.at(i - 1);
    const int previousEnd = previous.section("-", 1, 1).toInt();
    starts << ((askedStart == previousEnd) ? (starts.at(i - 1) + actual.at(i - 1)) : askedStart);
  }

  // Schnittbereiche in Container-Einheiten
  QList<int> cutFrom, cutTo;
  foreach(QString cut, m_cuts)
  {
    elems = cut.split("-");
    if (elems.count() != 2) {
      continue;
    }
    cutFrom << elems.at(0).toInt();
    cutTo << elems.at(1).toInt();
  }

  QStringList parts;
  QHash<QString, QString> trimming;
  QString name;
  for (int i = 0, c = starts.count(); i < c; ++i) {
    const int from = starts.at(i);
    const int to = from + actual.at(i);
    parts << QString::number(from) + "-" + QString::number(to);
    QStringList pieces;
    int covered = 0;
    for (int k = 0, kc = cutFrom.count(); k < kc; ++k) {
      const int start = qMax(cutFrom.at(k), from);
      const int end = qMin(cutTo.at(k), to);
      if (start >= end) {
        continue;
      }
      covered += end - start;
      int length = (end - start) / m_frameScale;
      if (length < 1) {
        length = 1;
      }
      pieces
          << QString("Trim(%1,length=%2)").arg((start - from) / m_frameScale).arg(length);
    }
    name = Globals::getFileName(m_currentInput) + "_cut_" + numberToLength3String(i + 1)
        + ".mkv";
    const QString trim = (covered == to - from) ? QString("KEEP") : pieces.join("+");
    if (pieces.isEmpty() && covered == 0) {
      this->addInfo(
          " " + tr("part %1 holds nothing that was asked for -- keeping it whole").arg(i + 1));
      trimming.insert(name, "KEEP");
      continue;
    }
    this->addInfo("  " + tr("part %1 (%2) <> %3").arg(i + 1).arg(parts.last()).arg(trim));
    trimming.insert(name, trim);
  }
  // Bleibt nur ein Teil uebrig, laeuft er unter dem Namen der Quelle -- so wie es
  // buildTrimAndPartsList() am Ende auch macht.
  if (trimming.count() == 1) {
    const QString trim = trimming.constBegin().value();
    trimming.clear();
    trimming.insert(m_currentInput, trim);
  }
  m_mkvVideoParts = parts;
  m_trimming = trimming;
  m_keyframeonly = false;
  // Die Zeitstempeldatei wurde aus dem alten Plan gebaut und zaehlt jetzt falsch.
  if (!m_timecodes.isEmpty()) {
    const QString text = this->cutTimecodes();
    if (Globals::saveTextTo(text, m_timecodes) == 0) {
      this->addInfo(" " + tr("rewrote the time codes for the corrected parts"));
    }
  }
}

void MkvCutter::handleSplitFiles()
{
  m_reencodedVideoFiles.clear();
  m_extractionFiles.clear();
  m_toDelete.clear();
  // Vor dem Verzweigen: im Keyframe-Pfad gibt es zwar nichts zu korrigieren, aber genau
  // dort faellt ein verlorenes Frame sonst niemandem auf.
  this->verifyAndCorrectParts();
  if (m_keyframeonly) {
    foreach (QString file, m_splitFiles)
    {
      m_reencodedVideoFiles << file;
    }
    this->startExtraction();
    return;
  }
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
      toDelete = toDelete.remove(0, toDelete.lastIndexOf("-") + 1);
      toDelete = Globals::getFileName(m_currentInput) + "_cut_" + toDelete;
      trim = m_trimming.value(toDelete);
    }
    this->addInfo(" " + tr("trim value for %1: %2").arg(toDelete).arg(trim));
    if (trim == "KEEP" || trim.isEmpty()) {
      m_reencodedVideoFiles << file;
      m_extractionFiles << file;
      continue;
    }
    if (!this->createAvisynthSkript(file, trim)) {
      // Ohne Script fiele dieser Teil aus beiden Listen -- die Ausgabe waere still
      // unvollstaendig, und faellt es fuer *alle* Teile aus, laeuft cleanUpAndMerge()
      // spaeter auf eine leere Liste. Lieber hier sauber abbrechen.
      this->addInfo(
          tr("Resetting: couldn't create the avisynth script for %1").arg(file));
      this->reset();
      return;
    }
  }
  this->startExtraction();
}

QString MkvCutter::getSmallest()
{
  qint64 size = -1;
  qint64 sSize = -1;
  int count = m_extractionFiles.count();
  int indexOfSmallest = -1;
  QString fileName;
  for(int i = 0; i < count; ++i) {
    QFile file;
    file.setFileName(m_extractionFiles.at(i));
    if (!file.exists()) {
      continue;
    }
    sSize = qint64(file.size());
    if (sSize < size || size == -1) {
      size = sSize;
      indexOfSmallest = i;
    }
  }

  if (indexOfSmallest == -1) {
    // Keine der gelisteten Dateien liegt auf der Platte -- mkvmerge hatte sie aber als
    // erzeugt gemeldet. takeAt(-1) waere ein Zugriff ausserhalb der Liste; stattdessen den
    // ersten Eintrag nehmen. Die Liste *muss* schrumpfen, sonst dreht startExtraction()
    // endlos. Der nachfolgende Extraktionsschritt scheitert dann sichtbar.
    this->addInfo(
        " " + tr("WARNING: none of the %1 file(s) to extract exists, taking the first one: %2").arg(
            count).arg(m_extractionFiles.first()));
    indexOfSmallest = 0;
  }
  return m_extractionFiles.takeAt(indexOfSmallest);
}

void MkvCutter::startExtraction()
{
  if (m_extractionFiles.isEmpty()) {
    std::cerr << "no more to extract,..." << std::endl;
    this->parseOriginal();
    return;
  }
    QString input = this->getSmallest();
  QString filename = input;
  filename = filename.remove(filename.lastIndexOf("."), filename.length());
  filename += ".264";
  filename = m_tempFolder + QDir::separator() + Globals::getWholeFileName(filename);
  filename = QDir::toNativeSeparators(filename);
  if (m_toAnalyse.isEmpty()) {
    m_toAnalyse = filename;
  }
  m_toDelete << filename;
  // handleSplitFiles() traegt KEEP-Dateien immer in beide Listen ein, indexOf() sollte also
  // treffen. Falls doch nicht, waere replace(-1, ...) ein Zugriff ausserhalb der Liste.
  const int indexInReencoded = m_reencodedVideoFiles.indexOf(input);
  if (indexInReencoded == -1) {
    this->addInfo(
        " " + tr("WARNING: %1 is not in the video file list, appending it").arg(input));
    m_reencodedVideoFiles << filename;
  } else {
    m_reencodedVideoFiles.replace(indexInReencoded, filename);
  }
  m_extractor->startExtraction(input, m_tempFolder);
}

void MkvCutter::extractTimeCodes()
{
  m_timeextractor->startExtraction(m_currentInput, QString::number(m_videoTrackID - 1), m_tempFolder);
}

void MkvCutter::setHasAudio(bool hasAudio)
{
  m_hasAudio = hasAudio;
}

void MkvCutter::mkvSubtitleCutterFinished(int state)
{
  if (state < 0) {
    this->addInfo(tr("Resetting since subtitle cutter crashed,.."));
    this->reset();
    return;
  }
  m_cutSubtitles = m_subtitleCutter->getCutSubtitles();
  this->addInfo("mkvSubtitleCutterFinished, output:\r\n" + m_cutSubtitles.join("\r\n"));
  this->cleanUpAndMerge();
}

void MkvCutter::mkvSubtitleExtractorFinished(int state)
{
  if (state < 0) {
    this->addInfo(tr("Resetting since subtitle extraction crashed,.."));
    this->reset();
    return;
  }
  m_subtitleToCut = m_mkvSubtitleExtractor->getOutputFiles();
  if (m_subtitleToCut.isEmpty()) {
    this->cleanUpAndMerge();
    return;
  }
  m_subtitleCutter->cutSubtitles(m_subtitleToCut, m_mkvAudioAndSubtitleParts, m_tempFolder);
}

void MkvCutter::finishedTimeCodeExtraction(int state)
{
  if (state < 0) {
    this->addInfo(tr("Resetting since time code extraction crashed,.."));
    this->reset();
    return;
  }
  if (m_ffindexCaller != nullptr) {
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
  // Jetzt liegen Feldreihenfolge und Scan-Type beide vor. PAFF heisst feldcodiert; MBAFF
  // ist frame-codiert und braucht beim finalen mkvmerge die verdoppelte --default-duration
  // ("i" meint Felder pro Sekunde). Frueher wurde auf den Scan-*Order*-Wert "MBAFF" geprueft,
  // den MediaInfo dort nie liefert -- MBAFF-Quellen galten deshalb faelschlich als PAFF.
  m_paff = (m_interlaced != "progressive")
      && !m_scanType.contains("MBAFF", Qt::CaseInsensitive);
  if (m_interlaced != "progressive") {
    this->addInfo(
        " " + tr("interlaced source, scan type: %1 -> paff: %2").arg(
            m_scanType.isEmpty() ? "-" : m_scanType).arg(m_paff ? "true" : "false"));
  }
  if (!m_vfr && qAbs(m_fps - int(m_fps)) > 0) {
    this->addInfo(tr("Video doesn't use an even frame rate -> extracting time codes"));
    m_vfr = true;
  }
  if (m_vfr) {
    ui.infoLabel->setText(tr("extracting time codes with mkvextract,.."));
    this->extractTimeCodes();
    return;
  }
  if (m_ffindexCaller != nullptr) {
    ui.infoLabel->setText(tr("Indexing input file,.."));
    m_ffindexCaller->index(m_currentInput, m_indexFile);
    return;
  }
  this->startViewer();
}

void MkvCutter::startViewer()
{
  cout << "init viewer" << endl;
  /*
  if (m_viewer != nullptr) {
    cout << "  reset m_viewer" << endl;
    delete m_viewer;
    m_viewer = nullptr;
  }
  */
  QStringList keyframes;
  foreach(QString key, m_keyframes)
  {
    key = key.remove(key.indexOf(","), key.size());
    keyframes << key;
  }
  m_viewer = new AVSViewer(this, m_tempAvs, m_aspectRatio, true, keyframes, m_frameCount);
  this->myconnect(m_viewer, SIGNAL(finished(int)), this, SLOT(avsViewerFinished(int)));
  this->myconnect(m_viewer, SIGNAL(frameScale(int)), this, SLOT(setFrameScale(int)));
  this->myconnect(m_viewer, SIGNAL(cuts(QStringList)), this, SLOT(setCutList(QStringList)));
  this->myconnect(m_viewer, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_viewer, SIGNAL(setInterlacedMode(QString)), this, SLOT(setInterlacedMode(QString)));
  ui.avsViewerVerticalLayout->insertWidget(0, m_viewer);
  ui.mainStackedWidget->setCurrentIndex(1);
  ui.infoLabel->setText(tr("- Cut View -"));
  m_viewer->init();
  // init() kann fehlschlagen und ueber finished(<0) einen reset() ausloesen; initTools()
  // loescht den Viewer dann und setzt m_viewer auf nullptr.
  if (m_viewer == nullptr) {
    return;
  }
  if (!m_cliCutList.isEmpty()) {
    const QString path = m_cliCutList;
    m_cliCutList.clear();
    this->addInfo(QString("CLI: loadCutList(%1)").arg(path));
    if (!m_viewer->loadCutList(path)) {
      this->addInfo("CLI: cut list is empty or unreadable -> not committing");
      m_cliCommit = false;
      m_cliNext = false;
    }
  }
  if (m_cliCommit) {
    m_cliCommit = false;
    this->addInfo("CLI: commit()");
    m_viewer->commitCuts();
  }
}

void MkvCutter::ffIndexerFinished(int exitstate)
{
  if (exitstate < 0) {
    this->addInfo(tr("Resetting since ffindexer crashed,.."));
    this->reset();
    return;
  }
  if (!QFile::exists(m_indexFile)) {
    this->addInfo(tr("FFindexer output file %1 doesn't exist!").arg(m_indexFile));
    this->reset();
    return;
  }
  ui.infoLabel->setText(tr("Indexing input file finished,.."));
  this->startViewer();
}

// Welcher Teil wird kopiert und welcher neu codiert? handleSplitFiles() ordnet den
// Teildateien "<Ausgabe>-NNN.mkv" die Trim-Angabe "<Quelle>_cut_NNN.mkv" zu, und
// m_mkvVideoParts steht in derselben Reihenfolge. Bleibt am Ende nur ein Trim uebrig,
// hat buildTrimAndPartsList() ihn auf m_currentInput umgeschluesselt.
QString MkvCutter::trimForPart(const int index) const
{
  if (m_trimming.isEmpty()) {
    return "KEEP";
  }
  if (m_trimming.count() == 1) {
    return m_trimming.constBegin().value();
  }
  const QString name = Globals::getFileName(m_currentInput) + "_cut_"
      + numberToLength3String(index + 1) + ".mkv";
  return m_trimming.value(name, "KEEP");
}

QString MkvCutter::cutTimecodes()
{
  this->addInfo(tr("Cutting time codes,..."));
  // Zeile 0 ist der Header, Zeile u+1 gehoert zur Container-Einheit u. Abschliessende
  // Leerzeilen (der Zeilenumbruch am Dateiende) sind keine Zeitstempel. Die Datei ist
  // CRLF-codiert, die Eintraege enden also auf '\r' -- deshalb trimmed().
  const QStringList &timeCodeList = m_inputTimeCodes;
  int lastStamp = timeCodeList.size() - 1;
  while (lastStamp > 0 && timeCodeList.at(lastStamp).trimmed().isEmpty()) {
    --lastStamp;
  }
  auto stampOfUnit = [&timeCodeList, lastStamp](int unit) -> double {
    int index = unit + 1;
    if (index < 1) {
      index = 1;
    } else if (index > lastStamp) {
      index = lastStamp;
    }
    return timeCodeList.at(index).trimmed().toDouble();
  };

  // Die Ausgabe enthaelt nicht je Frame genau einen Block: kopierte Teile einer
  // feldcodierten Quelle behalten ihre Feldbloecke (zwei je Frame), neu codierte Teile
  // kommen als Frames aus x264 (einer je Frame). Eine Stempelliste je Container-Einheit
  // war deshalb bei PAFF zu lang -- mkvmerge nahm die ersten n Stempel und die Ausgabe
  // lief zu kurz (gemessen: 350 Bloecke bekamen 400 Stempel a 20 ms -> 7,0 s statt
  // 8,0 s). Deshalb wird hier je *Block* ein Stempel geschrieben; bei m_frameScale == 1,
  // also bei allen frame-codierten Quellen, ist das unveraendert einer je Frame.
  QStringList outputTimeCodes, range, pieces;
  double timestamp = 0.0;
  for (int i = 0, c = m_mkvVideoParts.count(); i < c; ++i) {
    range = m_mkvVideoParts.at(i).split("-");
    if (range.count() != 2) {
      continue;
    }
    const int partStart = range.at(0).toInt();
    const int partEnd = range.at(1).toInt(); // exklusiv
    const QString trim = this->trimForPart(i);
    std::cerr
        << qPrintable(
            tr("adding time codes for part %1 (%2): %3").arg(i + 1).arg(
                m_mkvVideoParts.at(i)).arg(trim)) << std::endl;
    if (trim == "KEEP" || trim.isEmpty()) {
      for (int unit = partStart; unit < partEnd; ++unit) {
        outputTimeCodes << QString::number(qRound(timestamp));
        timestamp += stampOfUnit(unit + 1) - stampOfUnit(unit);
      }
      continue;
    }
    // Trim(a,length=n)[+Trim(b,length=m)...]; Offsets und Laengen zaehlen Clip-Frames.
    pieces = trim.split("+");
    foreach(QString piece, pieces)
    {
      const int open = piece.indexOf("(");
      const int comma = piece.indexOf(",", open);
      const int equal = piece.indexOf("=", comma);
      const int close = piece.lastIndexOf(")");
      if (open < 0 || comma < 0 || equal < 0 || close < 0) {
        continue;
      }
      const int offset = piece.mid(open + 1, comma - open - 1).toInt();
      const int length = piece.mid(equal + 1, close - equal - 1).toInt();
      for (int frame = 0; frame < length; ++frame) {
        const int unit = partStart + (offset + frame) * m_frameScale;
        outputTimeCodes << QString::number(qRound(timestamp));
        // Ungerundet aufsummieren, sonst summiert sich der Abschneidefehler ueber den
        // ganzen Schnitt auf.
        timestamp += stampOfUnit(unit + m_frameScale) - stampOfUnit(unit);
      }
    }
  }
  // Ein Stempel mehr als Bloecke: mkvmerge leitet die Dauer des letzten Blocks aus der
  // Differenz zum naechsten Stempel ab und rateet sie sonst. Ohne diese Zeile bekam ein
  // abschliessendes neu codiertes Frame 20 statt 40 ms -- die Ausgabe war 7,980 s statt
  // 8,000 s lang. Ein zusaetzlicher Block entsteht dadurch nicht.
  if (!outputTimeCodes.isEmpty()) {
    outputTimeCodes << QString::number(qRound(timestamp));
  }
  std::cerr << " output time code count " << (outputTimeCodes.count() - 1) << std::endl;
  this->addInfo(
      tr("Finished cutting time codes, count: %1").arg(outputTimeCodes.count() - 1));
  outputTimeCodes.insert(0, "# timecode format v2");
  return outputTimeCodes.join("\r\n");
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
  // Die Zeitstempel der Quelle braucht schon buildCutList(), um die Audio- und
  // Untertitelschnitte auszurechnen.
  if (m_timecodes.isEmpty()) {
    m_inputTimeCodes.clear();
  } else {
    m_inputTimeCodes = Globals::readAll(m_timecodes, "auto").split("\n");
  }
  this->buildCutList();
  this->buildTrimAndPartsList();
  // Erst jetzt steht fest, welche Teile kopiert und welche neu codiert werden -- und damit,
  // wie viele Bloecke die Ausgabe je Frame bekommt. Genau das braucht cutTimecodes() (B14).
  if (!m_timecodes.isEmpty()) {
    const QString text = this->cutTimecodes();
    if (!ui.keepIntermediateCheckBox->isChecked()) {
      QFile::remove(m_timecodes);
    }
    m_timecodes = m_timecodes.insert(m_timecodes.lastIndexOf("."), "_cut");
    if (Globals::saveTextTo(text, m_timecodes) == 0) {
      this->addInfo(tr("Successfully cut and saved timecodes, to: %1").arg(m_timecodes));
    }
  }
  ui.infoLabel->setText(tr("Set output base file and temp folder,.."));
  ui.mainStackedWidget->setCurrentIndex(2);
  if (m_cliNext) {
    m_cliNext = false;
    this->addInfo("CLI: next() (deferred)");
    this->on_nextPushButton_clicked();
  }
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
    QMessageBox::information(this, tr("Notice"), tr("You need to specify the output file and the temp folder!"));
    return;
  }
  ui.mainStackedWidget->setCurrentIndex(3);
  int listCount = m_mkvVideoParts.size();
  this->addInfo(tr("mkvParts count: %1").arg(listCount));
  this->addInfo(m_mkvVideoParts.join("\n  "));
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
  QString output = m_currentOutput;
  if (!m_subtitles.isEmpty() && m_keyframeonly) {
    output = output.insert(output.lastIndexOf("."), "_withoutSubs");
  }
  m_mkvVideoSplitCaller->start(m_currentInput, output, m_mkvVideoParts, m_tempFolder, false,
      m_keyframeonly);
}

void MkvCutter::cutAudio()
{
  if (!m_hasAudio) {
    if (!m_subtitles.isEmpty()) {
      m_mkvSubtitleExtractor->startExtraction(m_currentInput, m_subtitles, m_tempFolder);
      return;
    }
    this->cleanUpAndMerge();
    return;
  }
  this->addInfo(tr("Calling audio cutter,.."));
  m_audioFile = m_tempFolder + QDir::separator() + Globals::getWholeFileName(m_currentOutput);
  m_audioFile = m_audioFile.insert(m_audioFile.lastIndexOf("."), "_AudioCut");
  m_audioFile = QDir::toNativeSeparators(m_audioFile);
  m_mkvAudioCutCaller->setKeepIntermediate(ui.keepIntermediateCheckBox->isChecked());
  m_mkvAudioCutCaller->start(m_currentInput, m_currentOutput, m_mkvAudioAndSubtitleParts,
      m_tempFolder, true);
}

/**
 * Bestimmt, um wie viel jedes Tonstueck beim Muxen verschoben werden muss.
 *
 * mkvmerge kann verlustfrei nur auf Frame-Grenzen des Tonformats schneiden -- bei AC-3
 * sind das 32 ms. Ein Stueck faellt dadurch bis zu ein Frame kuerzer oder laenger aus als
 * angefordert. Werden die Stuecke einfach aneinandergehaengt, summieren sich diese Fehler
 * auf und der Ton laeuft dem Bild davon (B16; gemessen: nach drei Schnitten 55 ms).
 *
 * Soll jedes Stueck dort beginnen, wo sein Videostueck beginnt, dann ist der noetige
 * Versatz genau die Differenz aus Soll- und Ist-Laenge des *Vorgaengers* -- die Fehler
 * addieren sich dann nicht mehr, sondern bleiben je Stueck unter einem Tonframe.
 */
void MkvCutter::computeAudioSyncOffsets()
{
  m_audioSyncOffsets.clear();
  m_audioSplitFiles.sort(); // mkvmerge nummeriert die Stuecke, die Reihenfolge zaehlt
  const int count = m_audioSplitFiles.count();
  if (count < 2) {
    return; // ein einzelnes Stueck kann nicht auseinanderlaufen
  }
  if (m_mkvAudioAndSubtitleParts.count() != count) {
    this->addInfo(
        " " + tr("%1 audio parts but %2 requested ranges -> no sync offsets").arg(count).arg(
            m_mkvAudioAndSubtitleParts.count()));
    return;
  }
  this->addInfo(tr("measuring the audio parts,.."));
  m_audioSyncOffsets << "0"; // das erste Stueck beginnt bei null
  QStringList elems;
  for (int i = 1; i < count; ++i) {
    elems = m_mkvAudioAndSubtitleParts.at(i - 1).split("-");
    const double got = Globals::mkvDurationInMs(m_audioSplitFiles.at(i - 1));
    if (elems.count() != 2 || got < 0) {
      this->addInfo(
          "  " + tr("couldn't measure %1 -> no offset").arg(m_audioSplitFiles.at(i - 1)));
      m_audioSyncOffsets << "0";
      continue;
    }
    const double wanted = (Globals::timeToSeconds(elems.at(1))
        - Globals::timeToSeconds(elems.at(0))) * 1000.0;
    const int offset = qRound(wanted - got);
    this->addInfo(
        "  " + tr("part %1: %2 ms wanted, %3 ms delivered -> shifting the next part by %4 ms")
            .arg(i).arg(qRound(wanted)).arg(qRound(got)).arg(offset));
    m_audioSyncOffsets << QString::number(offset);
  }
}

/**
 * Rechnet die Kapitel der Quelle auf die geschnittene Zeitachse um.
 *
 * Saemtliche mkvmerge-Aufrufe schalten Kapitel ab, die Ausgabe hatte deshalb nie welche
 * (B18). Nach einem Schnitt stimmen die Zeiten der Quelle aber auch nicht mehr -- ein
 * Kapitel muss dorthin, wo sein Anfang in der Ausgabe landet.
 *
 * Regel: liegt der Kapitelanfang in einem behaltenen Bereich, wandert er an die
 * entsprechende Stelle der Ausgabe. Liegt er in einem weggeschnittenen Bereich, rutscht er
 * an den Anfang des naechsten behaltenen Bereichs -- und fallen dabei mehrere Kapitel auf
 * dieselbe Stelle, bleibt das letzte stehen, denn das ist das Kapitel, das an dieser Naht
 * gerade laeuft. Was hinter dem letzten behaltenen Bereich liegt, faellt weg.
 */
void MkvCutter::buildChapterFile()
{
  m_chapterFile = QString();
  if (m_mkvAudioAndSubtitleParts.isEmpty()) {
    return;
  }
  const QString chapters = Globals::mkvChaptersSimple(m_currentInput);
  if (chapters.trimmed().isEmpty()) {
    return;
  }
  // behaltene Bereiche samt ihrer Position in der Ausgabe
  QList<double> starts, ends, offsets;
  double offset = 0;
  QStringList elems;
  foreach(QString part, m_mkvAudioAndSubtitleParts)
  {
    elems = part.split("-");
    if (elems.count() != 2) {
      continue;
    }
    const double from = Globals::timeToSeconds(elems.at(0));
    const double to = Globals::timeToSeconds(elems.at(1));
    starts << from;
    ends << to;
    offsets << offset;
    offset += to - from;
  }
  if (starts.isEmpty()) {
    return;
  }
  // CHAPTERnn=HH:MM:SS.mmm / CHAPTERnnNAME=<Titel>
  QHash<QString, QString> times, names;
  foreach(QString line, chapters.split("\n"))
  {
    line = line.trimmed();
    const int equals = line.indexOf("=");
    if (!line.startsWith("CHAPTER", Qt::CaseInsensitive) || equals < 0) {
      continue;
    }
    QString key = line.left(equals);
    const QString value = line.mid(equals + 1);
    if (key.endsWith("NAME", Qt::CaseInsensitive)) {
      key = key.left(key.size() - 4);
      names.insert(key, value);
    } else {
      times.insert(key, value);
    }
  }
  QStringList keys = times.keys();
  keys.sort();
  QStringList outTimes, outNames;
  foreach(QString key, keys)
  {
    const double at = Globals::timeToSeconds(times.value(key));
    double mapped = -1;
    for (int i = 0, c = starts.count(); i < c; ++i) {
      if (at >= starts.at(i) && at < ends.at(i)) {
        mapped = offsets.at(i) + (at - starts.at(i));
        break;
      }
      if (at < starts.at(i)) { // im weggeschnittenen Bereich davor
        mapped = offsets.at(i);
        break;
      }
    }
    if (mapped < 0) {
      this->addInfo(
          "  " + tr("chapter %1 (%2) is behind the last kept part -> dropped").arg(
              names.value(key, key)).arg(times.value(key)));
      continue;
    }
    const QString stamp = Globals::secondsToHMSZZZ(mapped);
    if (!outTimes.isEmpty() && outTimes.last() == stamp) {
      outTimes.removeLast(); // an einer Naht gewinnt das zuletzt begonnene Kapitel
      outNames.removeLast();
    }
    outTimes << stamp;
    outNames << names.value(key, key);
  }
  if (outTimes.isEmpty()) {
    this->addInfo(" " + tr("no chapter survived the cut"));
    return;
  }
  QStringList lines;
  for (int i = 0, c = outTimes.count(); i < c; ++i) {
    const QString number = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0'));
    lines << "CHAPTER" + number + "=" + outTimes.at(i);
    lines << "CHAPTER" + number + "NAME=" + outNames.at(i);
  }
  QString file = m_tempFolder + QDir::separator()
      + Globals::getFileName(m_currentOutput) + "_chapters.txt";
  file = QDir::toNativeSeparators(file);
  if (Globals::saveTextTo(lines.join("\r\n"), file) != 0) {
    this->addInfo(" " + tr("couldn't save the chapters to %1").arg(file));
    return;
  }
  m_chapterFile = file;
  m_toDelete << file;
  this->addInfo(
      " " + tr("kept %1 of %2 chapters:").arg(outTimes.count()).arg(keys.count()));
  for (int i = 0, c = outTimes.count(); i < c; ++i) {
    this->addInfo("  " + outTimes.at(i) + "  " + outNames.at(i));
  }
}

void MkvCutter::setKeyFrames(QStringList list)
{
  ui.infoLabel->setText(tr("Got key frame list from mkvinfo analyzer."));
  int count = list.count();
  if (count == 0) {
    // Division durch Null vermeiden, wenn mkvinfo keine I-Frames fand
    this->addInfo(" " + tr("Warning: no keyframes detected in video stream!"));
    m_keyframes = list;
    m_averageKeyDistance = 0;
    return;
  }
  int dist = m_frameCount / count;
  m_keyframes = list;
  m_averageKeyDistance = dist;
  this->addInfo(
      " " + tr("video stream key frame count: %1, average distance: %2").arg(count).arg(dist));
}
void MkvCutter::setResolution(QString width, QString height)
{
  m_width = width.toInt();
  m_height = height.toInt();
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
  cerr << qPrintable(infos) << endl;
  ui.infoTextBrowser->append(infos);
  int bottom = ui.infoTextBrowser->verticalScrollBar()->maximum();
  ui.infoTextBrowser->verticalScrollBar()->setValue(bottom);
}

void MkvCutter::reset(bool andInit)
{
  this->addInfo("Reset");
  bool keepIntermediate = ui.keepIntermediateCheckBox->isChecked();
  if (!m_tempAvs.isEmpty() && !keepIntermediate) {
    this->addInfo(tr("Deleting %1,..").arg(m_tempAvs));
    //this->addInfo(Globals::readAll(m_tempAvs, "auto"));
    QFile::remove(m_tempAvs);
  }
  m_tempAvs = QString();
  foreach(QString file, m_tempReencodeAvs)
  {
    if (!keepIntermediate) {
      QFile::remove(file);
      file.chop(4);
      file = file + ".mkv.ffindex";
      QFile::remove(file);
    }
  }
  if (!m_indexFile.isEmpty()) {
    if (!keepIntermediate) {
      QFile::remove(m_indexFile);
    }
    m_indexFile = QString();
  }
  m_width = -1;
  m_height = -1;
  m_tempReencodeAvs.clear();
  m_currentInput = QString();
  m_currentOutput = QString();
  m_tempFolder = QString();
  m_hasAudio = false;
  m_avcProfileLevel = QString();
  m_audioFormat = QString();
  m_avcCabac = true;
  m_avcRefFrames = 1;
  m_enabled = 0;
  m_frameCount = -1;
  m_frameScale = 1;
  m_keyframes.clear();
  m_cuts.clear();
  m_splitFiles.clear();
  m_videoEncodingCalls.clear();
  m_reencodedVideoFiles.clear();
  m_fps = -1;
  m_trimming.clear();
  m_cutList.clear();
  m_sps = -1;
  m_mkvmergeIntSplitList.clear();
  m_mkvVideoParts.clear();
  m_mkvAudioAndSubtitleParts.clear();
  m_audioFile = QString();
  m_averageBitrate = -1;
  m_audioSplitFiles.clear();
  m_audioSyncOffsets.clear();
  m_chapterFile = QString();
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
  m_scanType = QString();
  m_chroma = "4:2:0";
  m_bitDepth = 8;
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
  m_subtitles.clear();
  if (andInit) {
    this->initTools();
  }
}

void MkvCutter::setFrameScale(int scale)
{
  m_frameScale = (scale < 1) ? 1 : scale;
  if (m_frameScale != 1) {
    this->addInfo(
        " " + tr("source is field coded: %1 container unit(s) per frame").arg(m_frameScale));
  }
}

void MkvCutter::setCutList(QStringList cuts)
{
  // Der Viewer zaehlt AviSynth-Frames, alles danach -- Keyframeliste, m_frameCount,
  // 'mkvmerge --split parts-frames:', die Zeitstempeldatei und m_fps -- zaehlt
  // Container-Einheiten. Bei feldcodierten Quellen sind das zwei je Frame, deshalb wird
  // die Schnittliste hier einmal umgerechnet; nur die Trim()-Werte im AviSynth-Script
  // muessen in buildTrimAndPartsList() wieder zurueck (siehe B14).
  if (m_frameScale == 1) {
    m_cuts = cuts;
    return;
  }
  m_cuts.clear();
  QStringList elems;
  foreach(QString cut, cuts)
  {
    elems = cut.split("-");
    if (elems.count() != 2) {
      m_cuts << cut;
      continue;
    }
    m_cuts
        << QString::number(elems.at(0).toInt() * m_frameScale) + "-"
            + QString::number(elems.at(1).toInt() * m_frameScale);
  }
  this->addInfo(
      " " + tr("cut list in container units: %1").arg(m_cuts.join(", ")));
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
    QMessageBox::critical(this, tr("Error"), tr("Couldn't connect %1 '%2' to %3 '%4'").arg(sender->objectName()).arg(signal).arg(
            receiver->objectName()).arg(type));
  }
}

void MkvCutter::setSps(int sps)
{
  m_sps = sps;
}

// ---------------------------------------------------------------------------
// CLI control methods (called from main.cpp via --clinput)
// ---------------------------------------------------------------------------

void MkvCutter::cliOpen(const QString &path)
{
  this->addInfo(QString("CLI: open(%1)").arg(path));
  this->setInput(path);
}

void MkvCutter::cliSetOutput(const QString &path)
{
  this->addInfo(QString("CLI: setOutput(%1)").arg(path));
  QFileInfo fi(path);
  QString out = QDir::toNativeSeparators(path);
  ui.outputLabel->setText(out);
  m_currentOutput = out;
  // If output is inside a temp folder, auto-set it
  if (m_tempFolder.isEmpty() && fi.isAbsolute()) {
    QString dir = fi.absolutePath();
    if (QDir(dir).exists()) {
      m_tempFolder = dir;
      ui.tempFolderLabel->setText(dir);
    }
  }
}

void MkvCutter::cliSetTemp(const QString &path)
{
  this->addInfo(QString("CLI: setTemp(%1)").arg(path));
  QString dir = QDir::toNativeSeparators(path);
  if (!QDir(dir).exists()) {
    this->addInfo(QString("CLI: temp folder does not exist, ignoring: %1").arg(dir));
    return;
  }
  ui.tempFolderLabel->setText(dir);
  m_tempFolder = dir;
}

void MkvCutter::cliSetKeepIntermediate(bool keep)
{
  this->addInfo(QString("CLI: setKeepIntermediate(%1)").arg(keep ? "true" : "false"));
  ui.keepIntermediateCheckBox->setChecked(keep);
}

void MkvCutter::cliNext()
{
  // Auf Seite 2 (Ausgabe/Temp gesetzt, Schnittliste steht) sofort ausloesen, sonst bis
  // nach dem Commit des Viewers aufheben -- beim Abarbeiten von --clinput laeuft die
  // Analyse noch und es gibt weder Schnittliste noch mkvmerge-Parts.
  if (ui.mainStackedWidget->currentIndex() == 2) {
    this->addInfo("CLI: next()");
    this->on_nextPushButton_clicked();
    return;
  }
  this->addInfo("CLI: next() deferred until the cut view has been committed");
  m_cliNext = true;
}

void MkvCutter::cliLoadCutList(const QString &path)
{
  if (!QFile::exists(path)) {
    this->addInfo(QString("CLI: cut list does not exist, ignoring: %1").arg(path));
    return;
  }
  const QString file = QDir::toNativeSeparators(path);
  if (m_viewer != nullptr && ui.mainStackedWidget->currentIndex() == 1) {
    this->addInfo(QString("CLI: loadCutList(%1)").arg(file));
    m_viewer->loadCutList(file);
    return;
  }
  this->addInfo(QString("CLI: loadCutList(%1) deferred until the cut view is up").arg(file));
  m_cliCutList = file;
}

void MkvCutter::cliCommit()
{
  if (m_viewer != nullptr && ui.mainStackedWidget->currentIndex() == 1) {
    this->addInfo("CLI: commit()");
    m_viewer->commitCuts();
    return;
  }
  this->addInfo("CLI: commit() deferred until the cut view is up");
  m_cliCommit = true;
}

void MkvCutter::cliSetScanOrder(const QString &mode)
{
  this->addInfo(QString("CLI: setScanOrder(%1)").arg(mode));
  this->setInterlacedMode(mode);
}
