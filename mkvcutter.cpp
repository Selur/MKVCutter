#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollbar>
#include <iostream>
#include "Globals.h"
using namespace std;

MkvCutter::MkvCutter(QWidget *parent) :
    QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_indexFile(QString()),
    m_currentOutput(QString()), m_tempFolder(QString()),m_avcProfileLevel(QString("High@L4.1")),
    m_audioFormat(QString()), m_avcCabac(true), m_avcRefFrames(1), m_enabled(0), m_frameCount(0),
    m_keyframes(), m_cuts(), m_splitFiles(), m_tempReencodeAvs(), m_videoEncodingCalls(),
    m_audioEncodingCalls(), m_fps(-1), m_trimming(), m_keyTimes(), m_cutList(),
    m_mkvmergeIntSplitList()
{
  this->setObjectName("MkvCutter-Main");
  m_mkvinfoAnalyser = new MkvInfoSourceAnalyser(this);
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(keyFrameInfos(QStringList)), this,
                  SLOT(setKeyFrames(QStringList)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(finished()), this, SLOT(mkvAnalysefinished()));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(progress(int)), this, SLOT(mkvAnalyseProgress(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(frameCount(int)), this, SLOT(setFrameCount(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(fps(double)), this, SLOT(setFPS(double)));
  m_mediaInfoAnalyser = new MediaInfoAnalyser(this);
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(finished(int)), this, SLOT(mediaInfoFinished(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(avcProfileLevel(QString)), this, SLOT(setAvcProfileLevel(QString)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(refframes(int)), this, SLOT(setAvcRefFrames(int)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(cabac(bool)), this, SLOT(setAvcCabac(bool)));
  this->myconnect(m_mediaInfoAnalyser, SIGNAL(audioFormat(QString)), this, SLOT(setAudioFormat(QString)));
  m_ffindexCaller = new FFIndexCaller(this);
  this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
  this->myconnect(m_ffindexCaller, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  m_mkvSplitCaller = new MkvSplitCaller(this);
  this->myconnect(m_mkvSplitCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_mkvSplitCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_mkvSplitCaller, SIGNAL(finished(int)), this, SLOT(mkvSplitFinished(int)));
  this->myconnect(m_mkvSplitCaller, SIGNAL(progress(int)), this, SLOT(mkvsplitProgress(int)));
  this->myconnect(m_mkvSplitCaller, SIGNAL(splitFiles(QStringList)), this, SLOT(setSplitFiles(QStringList)));

  m_viewer = 0;
  ui.setupUi(this);
  ui.mainStackedWidget->setCurrentIndex(0);
}

MkvCutter::~MkvCutter()
{
  this->reset();
}

void MkvCutter::setSplitFiles(QStringList splitFiles)
{
    m_splitFiles = splitFiles;
}

void MkvCutter::setFPS(double framerate)
{
  m_fps = framerate;
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

void MkvCutter::mkvsplitProgress(int percent)
{
  ui.infoLabel->setText(tr("MkvMerge splitting at %1%").arg(percent));
}

void MkvCutter::mkvAnalyseProgress(int linesRead)
{
  ui.infoLabel->setText(tr("MkvInfoAnalyser read %1 lines,..").arg(linesRead));
}

bool MkvCutter::createAVS()
{
  QString temp = m_currentInput;
  temp = temp.remove(temp.lastIndexOf("."), temp.size());
  m_indexFile = temp;
  m_indexFile += ".ffindex";
  m_indexFile = QDir::toNativeSeparators(m_indexFile);
  temp += ".avs";
  m_tempAvs = QDir::toNativeSeparators(temp);

  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  script << "LoadPlugin(\"" + QDir::toNativeSeparators(inputPath + "ffms2.dll") + "\")";
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
    ret = "0"+ret;
  }
  return ret;
}

void MkvCutter::buildCutList()
{
  this->addInfo("building cut list,..");
  //build cuts list
  QStringList tCuts;
  int prevKey = -1, nextKey = -1, keyIndex = 0, keyCount = m_keyframes.count();
  QString tmp;
  int currentKey;
  int start, end;
  cutTyp1 temp;
  QStringList elems;
  for (int i = 0, c = m_cuts.count(); i < c; ++i) {
    tCuts = m_cuts.at(i).split("-");
    start = tCuts.at(0).toInt();
    end = tCuts.at(1).toInt();
    for (; keyIndex < keyCount; ++keyIndex) {
      tmp = m_keyframes.at(keyIndex);
      elems = tmp.split(",");
      tmp = elems.at(0);
      currentKey = tmp.toInt();
      m_keyTimes.insert(currentKey, elems.at(1));
      if (currentKey <= start) { //
        prevKey = currentKey;
        continue;
      }
      if (currentKey >= end) {
        nextKey = currentKey;
        break;
      }
      if (nextKey < end) {
        nextKey = end;
      }
    }
    //add to list
    m_mkvmergeIntSplitList.insert(prevKey);
    m_mkvmergeIntSplitList.insert(nextKey);
    temp.prevKey = prevKey;
    temp.cut.start = start;
    temp.cut.end = end;
    temp.nextKey = nextKey;
    //this->addInfo("adding to cuts: " + Globals::cutTyp1ToString(temp));
    m_cutList.append(temp);
  }
  this->addInfo("finished building cuts list,..");

  QStringList trimCalls;
  QString name, trim;
  bool matchStart, matchEnd, matchLast;
  int fileIndex = 1;
  for (int i = 0, c = m_cutList.count(); i < c; ++i, ++fileIndex) {
    temp = m_cutList.at(i);
    name = "cut_" + numberToLength3String(fileIndex) + ".mkv";
    trim = QString();
    matchStart = temp.prevKey == temp.cut.start;
    matchEnd = temp.nextKey - 1 == temp.cut.end;
    matchLast = temp.nextKey == temp.cut.end && temp.nextKey == m_frameCount;
    if (i != 0 || temp.prevKey != 0) {
        trim += "KEEP";
        this->addInfo(tr("keep: adding %1 <> %2 to trimList").arg(name).arg(trim));
        m_trimming.insert(name, trim);
        fileIndex++;
        name = "cut_" + numberToLength3String(fileIndex) + ".mkv";
        trim = QString();
    }
    if (matchStart && (matchEnd || matchLast)) {
      trim += "DELETE";
      this->addInfo(tr("delete: adding %1 <> %2 to trimList").arg(name).arg(trim));
      m_trimming.insert(name, trim);
      continue;
    }
    if (matchStart) {
      trim += "Trim(" + QString::number(temp.cut.end - temp.prevKey + 1) + ","
          + QString::number(temp.nextKey - temp.prevKey - 1) + ")";
      this->addInfo(tr("matchedStart: adding %1 <> %2 to trimList").arg(name).arg(trim));
      m_trimming.insert(name, trim);
      continue;
    }
    if (matchLast) {
      trim += "Trim(0," + QString::number(temp.cut.start - temp.prevKey - 1) + ")";
      this->addInfo(tr("matchLast: adding %1 <> %2 to trimList").arg(name).arg(trim));
      m_trimming.insert(name, trim);
      continue;
    }
    if (matchEnd) {
      trim += "Trim(0," + QString::number(temp.cut.start - temp.prevKey - 1) + ")";
      this->addInfo(tr("matchEnd: adding %1 <> %2 to trimList").arg(name).arg(trim));
      m_trimming.insert(name, trim);
      continue;
    }
    trim += "Trim(0," + QString::number(temp.cut.start - temp.prevKey - 1) + ")";
    trim += "+";
    trim += "Trim(" + QString::number(temp.cut.end - temp.prevKey + 1) + ","
        + QString::number(temp.nextKey - temp.prevKey - 1) + ")";
    this->addInfo(tr("matchMiddle: adding %1 <> %2 to trimList").arg(name).arg(trim));
    m_trimming.insert(name, trim);
    continue;
  }
  this->addInfo("finished building trimList,..");
}

void MkvCutter::createAudioCutCall(QString filename, QString trim)
{
    if (m_audioFormat.isEmpty()) {
        return;
    }
    //TODO: Create audioCut calls,.. fill m_audioEncodingCalls;
}

void MkvCutter::createAvisynthSkript(QString filename, QString trim)
{
  this->addInfo(tr("createAvisynthSkript(%1, %2)").arg(filename).arg(trim));
  QString avisynthFileName = filename;
  avisynthFileName = avisynthFileName.remove(avisynthFileName.lastIndexOf((".")), avisynthFileName.size());
  avisynthFileName += ".avs";

  QStringList script;
  QString inputPath = QApplication::applicationDirPath() + QDir::separator();
  script << "LoadPlugin(\"" + QDir::toNativeSeparators(inputPath + "ffms2.dll") + "\")";
  script << "FFVideoSource(\"" + filename + "\", threads=1)";
  script << trim;
  if(Globals::saveTextTo(script.join("\n"), avisynthFileName) == 0) {
    trim = tr("Saved avisynth script:\r\n%1\r\nto: %2").arg(trim).arg(avisynthFileName);
    this->addInfo(trim);
    m_tempReencodeAvs << avisynthFileName;
    this->createReencodeCall(avisynthFileName);
  } else {
    this->addInfo(tr("Couldn't create(%1)").arg(avisynthFileName));
  }
}

void MkvCutter::createReencodeCall(QString avisynthFile)
{
  this->addInfo(tr("createReencodeCall(%1)").arg(avisynthFile));
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
    tmp+= "high";
  } else if (m_avcProfileLevel.contains("High")) {
    tmp += "baseline";
  } else {
    tmp += "main";
  }
  call << tmp;

  tmp = m_avcProfileLevel;
  if (!tmp.isEmpty()) {
    tmp = tmp.remove(0, tmp.indexOf("@") + 1);
    tmp = tmp.remove(".");
    tmp = "--profile "+tmp;
    call << tmp;
    //TODO: add vbv restrictions
  }
  if (!m_avcCabac) {
      call << "-no-cabac";
  }
  call << "--frames "+m_avcRefFrames;
  call << "--thread-input";
  //TODO: bluray check
  call << "--crf 19";
  call << "--demuxer avs";
  tmp = avisynthFile;
  tmp = tmp.remove(tmp.indexOf("."), tmp.size());
  tmp += "_reencode.mkv";
  tmp += "-o \""+tmp+"\"";
  call << tmp;
  tmp = "\""+avisynthFile+"\"";
  call << tmp;
  tmp = call.join(" ");
  this->addInfo(tr("x264 call: %1").arg(tmp));
  m_videoEncodingCalls << tmp;
}

void MkvCutter::startReencoding()
{
  // m_videoEncodingCalls abarbeiten
  // m_audioEncodingCalls abarbeiten
  // muxing calls erstellen
  // generate mkvmerge calls to join all parts
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
  //TODO: CHECK SPLIT FILES!!
  return;
  //handle splitFiles
  QString toDelete, trim;
  foreach (QString file, m_splitFiles) {
      toDelete = file;
      toDelete = Globals::getWholeFileName(file);
      toDelete = toDelete.remove(0, toDelete.indexOf("-")+1);
      toDelete = "cut_"+toDelete;
      trim = m_trimming.value(toDelete);
      if (trim == "KEEP" || trim.isEmpty()) {
          continue;
      }
      if (trim == "DELETE") {
          if (QFile::remove(file)) {
            this->addInfo(tr("Deleted %1,..").arg(file));
          } else {
            this->addInfo(tr("Couldn't delete %1,..").arg(file));
          }
          continue;
      }
      this->createAvisynthSkript(file, trim);
      this->createAudioCutCall(file, trim);
  }
  this->startReencoding();
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
  this->buildCutList();
  this->addInfo("split key frames: " + Globals::intSetToString(m_mkvmergeIntSplitList));
  this->addInfo("split times: " + Globals::intSetToTimes(m_mkvmergeIntSplitList, m_fps));
  ui.infoLabel->setText(tr("Set output base file and temp folder,.."));
  ui.mainStackedWidget->setCurrentIndex(2);
}

void MkvCutter::on_outputPushButton_clicked()
{
  QString name = tr("Select mkv output base file");
  QString select = tr("Output (*.mkv)");
  QString inputPath = QApplication::applicationDirPath();
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
  QString tempFolder = QFileDialog::getExistingDirectory(this, name, inputPath);
  if (!tempFolder.isEmpty()) {
    tempFolder = QDir::toNativeSeparators(tempFolder);
    ui.tempFolderLabel->setText(tempFolder);
    m_tempFolder = tempFolder;
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
  ui.infoLabel->setText(tr("Calling mkvmerge,.."));
  this->buildAndCallMkvMerge();
}

QString MkvCutter::keyFrameTimes()
{
   QString times;
   QString value;
   foreach (int key, m_mkvmergeIntSplitList) {
       if (key == 0 || key == m_frameCount) {
           continue;
       }
       value = m_keyTimes.value(key);
       if (value.isEmpty()) {
           this->addInfo("FOUND NO ENTRY FOR: "+QString::number(key));
           continue;
       }
       value = value.remove(0, value.indexOf("(")+1);
       value = value.remove(value.indexOf(")"), value.size());
       if (!value.isEmpty()) {
         times += value+",";
       }
   }
   if (!times.isEmpty()) {
     times.remove(times.size() - 1, 1);
   }
   this->addInfo("Times: "+times);
   return times;
}

void MkvCutter::buildAndCallMkvMerge()
{
  m_mkvSplitCaller->start(m_currentInput, m_currentOutput, keyFrameTimes(), m_tempFolder);
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
  m_currentInput = QString();
  m_keyframes.clear();
  m_frameCount = 0;
  ui.mainStackedWidget->setCurrentIndex(0);
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
  m_tempReencodeAvs.clear();
  m_cuts.clear();
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
