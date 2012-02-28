#include "mkvcutter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QScrollbar>
#include <iostream>
using namespace std;

struct cutTyp
{
    int start, end;
};

struct cutTyp1
{
    int prevKey, nextKey;
    cutTyp cut;
};
struct cutTyp2
{
    int prevKey, nextKey;
    QList<cutTyp> cuts;
};

MkvCutter::MkvCutter(QWidget *parent) :
    QWidget(parent), m_currentInput(QString()), m_tempAvs(QString()), m_enabled(0), m_frameCount(0),
        m_keyframes(), m_cuts()
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
  m_ffindexCaller = new FFIndexCaller(this);
  this->myconnect(m_ffindexCaller, SIGNAL(enableGui(bool)), this, SLOT(enableGui(bool)));
  this->myconnect(m_ffindexCaller, SIGNAL(sendInfos(QString)), this, SLOT(addInfo(QString)));
  this->myconnect(m_ffindexCaller, SIGNAL(finished(int)), this, SLOT(ffIndexerFinished(int)));
  this->myconnect(m_mkvinfoAnalyser, SIGNAL(progress(int)), this, SLOT(ffindexProgress(int)));
  m_viewer = 0;
  ui.setupUi(this);
}

MkvCutter::~MkvCutter()
{
  this->reset();
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

QString cutTypToString(cutTyp cut)
{
    return QString::number(cut.start)+"-"+QString::number(cut.end);
}

QString cutTyp1ToString(cutTyp1 cut)
{
    return QString::number(cut.prevKey)+" "+cutTypToString(cut.cut)+" "+QString::number(cut.nextKey);
}

QString cutTyp1ListToString(QList<cutTyp> elems) {
    QString cutString;
    foreach (cutTyp tCut, elems) {
        cutString += QString::number(tCut.start)+"-"+QString::number(tCut.end)+",";
    }
    cutString.remove(cutString.size()-1, 1);
    return cutString;
}

QString cutTyp2ToString(cutTyp2 cut)
{
    return QString::number(cut.prevKey)+" "+cutTyp1ListToString(cut.cuts)+" "+QString::number(cut.nextKey);
}

struct mkvCut{
    int start;
    QString reencode;
};

QString mkvCutToString(mkvCut cut)
{
    return QString::number(cut.start)+" "+cut.reencode;
}

QString mkvCutListToString(QList<mkvCut> cuts)
{
    QString cutString;
    foreach (mkvCut tCut, cuts) {
        cutString += mkvCutToString(tCut)+",";
    }
    cutString.remove(cutString.size()-1, 1);
    return cutString;
}

void MkvCutter::buildCutList()
{
    this->addInfo("building cut list,..");
    //build cuts1 list
    QList<cutTyp1> cuts1;
    QStringList cuts;
    int prevKey = -1, nextKey = -1, keyIndex = 0, keyCount = m_keyframes.count();
    QString tmp;
    int currentKey;
    int start, end;
    cutTyp1 temp;
    for (int i = 0, c = m_cuts.count(); i < c; ++i) {
        cuts = m_cuts.at(i).split("-");
        start = cuts.at(0).toInt();
        end = cuts.at(1).toInt();
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
        temp.prevKey = prevKey;
        temp.cut.start = start;
        temp.cut.end = end;
        temp.nextKey = nextKey;
        //this->addInfo("adding to cuts1: "+cutTyp1ToString(temp));
        cuts1.append(temp);
    }
    this->addInfo("finished building cuts1 list,..");

    //building cuts2 list
    QList<cutTyp2> cuts2;
    cutTyp1 current;
    cutTyp2 toInsert;
    toInsert.prevKey = -1;
    toInsert.nextKey = -1;
    for (int i = 0, c = cuts1.count(); i < c; ++i) {
        current = cuts1.at(i);
        if (current.prevKey == toInsert.prevKey || toInsert.prevKey == -1) { //extend
            toInsert.prevKey = current.prevKey;
            toInsert.cuts.append(current.cut);
            toInsert.nextKey = current.nextKey;
            if (i+1 == c) {
                this->addInfo("adding to cuts2: "+cutTyp2ToString(toInsert));
                cuts2.append(toInsert);
            }
            continue;
        }
        if (current.prevKey > toInsert.nextKey) { //new
            this->addInfo("adding to cuts2: "+cutTyp2ToString(toInsert));
            cuts2.append(toInsert);

            toInsert.prevKey = current.prevKey;
            toInsert.cuts.clear();
            toInsert.cuts.append(current.cut);
            toInsert.nextKey = current.nextKey;
            if (i+1 == c) {
                this->addInfo("adding to cuts2: "+cutTyp2ToString(toInsert));
                cuts2.append(toInsert);
            }
            continue;
        }
    }
    this->addInfo("finished building cuts2 list,..");

    //build mkvCutList
    QList<mkvCut> mkvCuts;
    mkvCut mkvCurrent;
    mkvCurrent.start = 0;
    mkvCurrent.reencode = QString();
    cutTyp2 mkvTemp;
    for (int i = 0, c = cuts2.count(); i < c; ++i) {
        mkvTemp = cuts2.at(i);
        if (mkvTemp.prevKey == mkvCurrent.start) {

            mkvCurrent.reencode = cutTyp1ListToString(mkvTemp.cuts);
            this->addInfo("adding to mkvCutList: "+mkvCutToString(mkvCurrent));
            mkvCuts.append(mkvCurrent);
            mkvCurrent.reencode = "KEEP";
            mkvCurrent.start = mkvTemp.nextKey;
            if (i+1 == c) {
              this->addInfo("adding to mkvCutList: "+mkvCutToString(mkvCurrent));
              mkvCuts.append(mkvCurrent);
            }
            continue;
        }
        if (mkvTemp.prevKey > mkvCurrent.start) {
            mkvCurrent.start = mkvTemp.prevKey;
            this->addInfo("adding to mkvCutList: "+mkvCutToString(mkvCurrent));
            mkvCuts.append(mkvCurrent);
            mkvCurrent.reencode = cutTyp1ListToString(mkvTemp.cuts);
            mkvCurrent.start = mkvTemp.nextKey;
            if (i+1 == c) {
              this->addInfo("adding to mkvCutList: "+mkvCutToString(mkvCurrent));
              mkvCuts.append(mkvCurrent);
            }
            continue;
        }
    }
    this->addInfo("finished building mkvCutList list,..");
    this->addInfo("mkvCutList: "+mkvCutListToString(mkvCuts));
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

  //TODO: Check output handle cutlists:
  // generate mkvmerge calls to split content
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
