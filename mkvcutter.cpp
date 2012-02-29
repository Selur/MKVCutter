#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QScrollbar>
#include <iostream>
#include "Globals.h"
using namespace std;

MkvCutter::MkvCutter(QWidget *parent) :
    QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_indexFile(QString()),
    m_currentOutput(QString()), m_tempFolder(QString()), m_enabled(0), m_frameCount(0),
    m_keyframes(), m_cuts(), m_fps(-1), m_trimming(),m_cutList(), m_mkvmergeIntSplitList()
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
  m_ffindexCaller = new FFIndexCaller(this);
  this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  m_viewer = 0;
  ui.setupUi(this);
  ui.mainStackedWidget->setCurrentIndex(0);
}

MkvCutter::~MkvCutter()
{
  this->reset();
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

void MkvCutter::mkvAnalyseProgress(int linesRead)
{
  ui.infoLabel->setText(tr("MkvInfoAnalyser read %1 lines,..").arg(linesRead));
}

int MkvCutter::saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
    this->addInfo(QObject::tr("Failed: saveTextTo %1 called with empty text").arg(to));
    return -1;
  }
  QFile file(to);
  file.remove();
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForUtfText(text.toUtf8()));
    out << text;
    if (file.exists()) {
      file.close();
      return 0;
    }
  }
  this->addInfo(QObject::tr("Failed to saveTextTo %1").arg(to));
  return -1;
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
  return this->saveTextTo(script.join("\n"), m_tempAvs) == 0;
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
  ui.infoLabel->setText(tr("Indexing input file,.."));
  m_ffindexCaller->index(m_currentInput, m_indexFile);
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
    for (int i = 0, c = m_cuts.count(); i < c; ++i) {
        tCuts = m_cuts.at(i).split("-");
        start = tCuts.at(0).toInt();
        end = tCuts.at(1).toInt();
        for (; keyIndex < keyCount; ++keyIndex) {
            tmp = m_keyframes.at(keyIndex);
            tmp = tmp.remove(tmp.indexOf(","), tmp.size());
            currentKey = tmp.toInt();
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
        this->addInfo("adding to cuts: "+cutTyp1ToString(temp));
        m_cutList.append(temp);
    }
    this->addInfo("finished building cuts list,..");

    QStringList trimCalls;
    QString name, trim;
    bool matchStart, matchEnd, matchLast;
    for (int i = 0, c = m_cutList.count(); i < c; ++i) {
        temp = m_cutList.at(i);
        name = "cut_" + QString::number(temp.prevKey) + "-" + QString::number(temp.nextKey-1)+".mkv";
        trim = QString();
        matchStart = temp.prevKey == temp.cut.start;
        matchEnd = temp.nextKey-1 == temp.cut.end;
        matchLast = temp.nextKey == temp.cut.end && temp.nextKey == m_frameCount;
        if (matchStart && (matchEnd || matchLast)) {
            trim += "DELETE";
            //this->addInfo(tr("%1 delete: adding %2 to trimList").arg(name).arg(trim));
            m_trimming.insert(name,trim);
            continue;
        }
        if (matchStart) {
            trim += "Trim("+QString::number(temp.cut.end-temp.prevKey+1)+","+QString::number(temp.nextKey-temp.prevKey-1)+")";
            //this->addInfo(tr("%1 matchedStart: adding %2 to trimList").arg(name).arg(trim));
            m_trimming.insert(name,trim);
            continue;
        }
        if (matchLast) {
            trim += "Trim(0,"+QString::number(temp.cut.start-temp.prevKey-1)+")";
            //this->addInfo(tr("%1 matchLast: adding %2 to trimList").arg(name).arg(trim));
            m_trimming.insert(name,trim);
            continue;
        }
        if (matchEnd) {
            trim += "Trim(0,"+QString::number(temp.cut.start-temp.prevKey-1)+")";
            //this->addInfo(tr("%1 matchEnd: adding %2 to trimList").arg(name).arg(trim));
            m_trimming.insert(name,trim);
            continue;
        }
        trim += "Trim(0,"+QString::number(temp.cut.start-temp.prevKey-1)+")";
        trim+="+";
        trim += "Trim("+QString::number(temp.cut.end-temp.prevKey+1)+","+QString::number(temp.nextKey-temp.prevKey-1)+")";
        //this->addInfo(tr("%1 matchMiddle: adding %2 to trimList").arg(name).arg(trim));
        m_trimming.insert(name,trim);
        continue;
    }
    this->addInfo("finished building trimList,..");
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
  this->addInfo("split key frames: "+intSetToString(m_mkvmergeIntSplitList));
  this->addInfo("split times: "+intSetToTimes(m_mkvmergeIntSplitList, m_fps));
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
        QMessageBox::information(this, tr("Notice"), tr("You need to specify the output file and the temp folder!"));
        return;
    }
    ui.mainStackedWidget->setCurrentIndex(3);
    ui.infoLabel->setText(tr("Calling mkvmerge,.."));
    this->buildAndCallMkvMerge();
}

void MkvCutter::buildAndCallMkvMerge()
{
    QMessageBox::information(this,"DEBUG", "Output: "+m_currentOutput+", temp: "+m_tempFolder);
    //TODO:
    // 2. generate mkvmerge split call
    // 3. lösche die Dateien die gelsöcht werden sollen
    // 4. erstelle Avisynth skripte zum Reencoden des Videos mit der TrimListe
    // generate x264 call to reencode the first few frames that are needed video
    // delete unneeded parts
    // extract and cut audio with (delaycut)
    // extract and cut subtitle with (?)
    // generate mkvmerge calls to mux reencoded parts
    // generate mkvmerge calls to join all parts
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
