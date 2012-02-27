#include "AVSViewer.h"
#include "windows.h"
#include "avisynth.h"
#include <QFile>
#include <QImage>
#include <QString>
#include <conio.h>
#include <iostream>
#include <QLibrary>
#include <QMessageBox>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCodec>
using namespace std;

AVSViewer::AVSViewer(QWidget *parent, QString path, double mult, bool cutSupport) :
    QWidget(parent), m_env(0), m_inf(), m_clip(), m_frameCount(100), m_current(-1),
        m_currentInput(path), m_version(QString()), m_avsModified(QString()), m_res(0),
        m_mult(mult), m_currentImage(), m_cutSupport(cutSupport)
{
  ui.setupUi(this);
  if (!m_cutSupport) {
    delete ui.cutStackedWidget;
    delete ui.commitPushButton;
  }
  if (m_currentInput.isEmpty()) {
    return;
  }
  ui.showLabel->setText(tr("Preparing environment for %1").arg(m_currentInput));
  QString tmp = tr("AVSViewer, current input: %1").arg(m_currentInput);
  this->send(tmp);

  delete ui.openAvsPushButton;
  this->init();
}

AVSViewer::~AVSViewer()
{
  if (!m_avsModified.isEmpty()) {
    QFile::remove(m_avsModified);
  }
}

void AVSViewer::send(QString message)
{
  cout << qPrintable(message) << endl;
  emit sendInfos(message);
}

int AVSViewer::invokeImportInternal(const char *inputFile)
{
  try {
    m_res = m_env->Invoke("Import", inputFile); //import current input to environment
  } catch (AvisynthError &err) { //catch AvisynthErrors
    this->send(tr("Avisynth error: %1").arg(err.msg));
    return -1;
  } catch (...) { //catch the rest
    this->send(tr("Unknown C++ exception,.."));
    return -1;
  }
  return 0;
}

int AVSViewer::import(const char *inputFile)
{
  __try {
    if (invokeImportInternal(inputFile) != 0) {
      return -1;
    }
  }
  __except(1)
  {
    cerr << "-> Win32 exception" << endl;
    return -1;
  }
  return 0;
}

int AVSViewer::invokeInternal(const char *function)
{
  try {
    QString tmp = function;
    emit
    sendInfos(" " + tr("invoking %1").arg(tmp));
    m_res = m_env->Invoke(function, AVSValue(&m_res, 1)); //import current input to environment
  } catch (AvisynthError &err) { //catch AvisynthErrors
    this->send(tr("Avisynth error: %1").arg(err.msg));
    return -1;
  } catch (...) { //catch the rest
    this->send(tr("Unknown C++ exception"));
    return -1;
  }
  return 0;
}

int AVSViewer::invoke(const char *function)
{
  __try {
    if (invokeInternal(function) != 0) {
      return -1;
    }
  }
  __except(1)
  {
    cerr << "-> Win32 exception" << endl;
    return -1;
  }
  return 0;
}

//TODO: add cut-edit option
//TODO: add preview-trimms, add reset view

void AVSViewer::on_setCutStartPushButton_clicked()
{
  if (!m_cutSupport) {
    return;
  }emit
  sendInfos(tr("set cut-start to: %1").arg(m_current));
  ui.frameHorizontalSlider->setStart(m_current);
}
void AVSViewer::on_setCutEndPushButton_clicked()
{
  if (!m_cutSupport) {
    return;
  }emit
  sendInfos(tr("set cut-end to: %1").arg(m_current));
  ui.frameHorizontalSlider->setEnd(m_current);
}

bool AVSViewer::isValidCut(int start, int end)
{
  if (start == end) {
    this->send(tr("Ignored start and end need to differ!").arg(start).arg(end));
    return false;
  }
  int pos;
  QString elem;
  QStringList cutElems;
  for (int i = 0, c = ui.cutListWidget->count(); i < c; ++i) {
    elem = ui.cutListWidget->item(i)->text();
    if (elem.isEmpty()) {
      continue;
    }
    cutElems = elem.split("-");
    pos = cutElems.at(0).toInt(); //start
    if (start > pos && pos < end) {
      this->send(tr("Ignored %1-%2 since it overlaps with %3.").arg(start).arg(end).arg(elem));
      return false;
    }
    pos = cutElems.at(1).toInt(); //end
    if (start > pos && pos < end) {
      this->send(tr("Ignored %1-%2 since it overlaps with %3.").arg(start).arg(end).arg(elem));
      return false;
    }
  }
  return true;
}

void AVSViewer::on_addCutPushButton_clicked()
{
  if (!m_cutSupport) {
    return;
  }
  int start = ui.frameHorizontalSlider->getStart();
  int end = ui.frameHorizontalSlider->getEnd();
  if ((start == 0 && end == 0) || start == end) {
    return;
  }
  if (start != ui.frameHorizontalSlider->minimum() || end != ui.frameHorizontalSlider->maximum()) {
    if (!isValidCut(start, end)) {
      return;
    }
    QString cut = QString::number(start) + "-" + QString::number(end);
    emit
    sendInfos(tr("add cut item: %1").arg(cut));
    ui.cutListWidget->addItem(cut);
    ui.cutListWidget->sortItems();
    ui.frameHorizontalSlider->resetMarks();
  }
}

void AVSViewer::on_removeCutPushButton_clicked()
{
  if (!m_cutSupport) {
    return;
  }
  int row = ui.cutListWidget->currentRow();
  if (row == -1) {
    return;
  }
  QListWidgetItem *item = ui.cutListWidget->takeItem(row);
  emit
  sendInfos(tr("removing %1 from cut-list").arg(item->text()));
}

void AVSViewer::on_saveImagePushButton_clicked()
{
  ui.showLabel->setText(tr("Set output png file,.."));
  QString name = tr("Select input file");
  QString select = tr("Output (*.png)");
  QString inputPath = QApplication::applicationDirPath();
  QString input = QFileDialog::getSaveFileName(this, name, inputPath, select);
  if (input.isEmpty()) {
    return;
  }
  if (!m_currentImage.save(input, "PNG")) {
    QMessageBox::warning(this, "Error", tr("Couldn't save %1").arg(input));
  }
}

void AVSViewer::on_commitPushButton_clicked()
{
  if (m_cutSupport) {
    QStringList cutList;
    QString elem;
    for (int i = 0, c = ui.cutListWidget->count(); i < c; ++i) {
      elem = ui.cutListWidget->item(i)->text();
      if (elem.isEmpty()) {
        continue;
      }
      cutList << elem;
    }
    emit cuts(cutList);
  }
  emit finished(0);
}

void AVSViewer::on_ffinfoCheckBox_toggled()
{
  if (m_env != 0) {
    this->send(tr("Clean up old script environment,.."));
    m_res = 0;
    m_clip = 0;
    m_env->DeleteScriptEnvironment(); //delete the old script environment
    m_env = 0; // ensure new environment created next time
  }
  this->init(m_current);
}

QString removeQuotes(QString input)
{
  QString ret = input.trimmed();
  if (ret.startsWith("\"")) {
    ret = ret.remove(0, 1);
  }
  if (ret.endsWith("\"")) {
    ret = ret.remove(ret.size() - 1, 1);
  }
  return ret;
}

QString getWholeFileName(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString output = QDir::toNativeSeparators(input);
  int index = output.lastIndexOf(QDir::separator());
  if (output.endsWith(QDir::separator())) {
    return QString();
  } else if (index != -1) {
    output = output.remove(0, index + 1);
  }
  output = removeQuotes(output);
  return QDir::toNativeSeparators(output);
}

QString getFileName(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString output = getWholeFileName(input);
  int index = output.lastIndexOf(".");
  if (index != -1) {
    output = output.remove(index, output.size());
  }
  return output;
}
QString removeLastSeparatorFromPath(QString input)
{
  input = input.trimmed();
  if (input.isEmpty()) {
    return input;
  }
  input = QDir::toNativeSeparators(input);
  int size = input.size();
  if (!input.endsWith(QDir::separator())) {
    return input;
  } else if (size == 1) { //input only consists of the separator
    return QString();
  }
  return input.remove(size - 1, 1);
}

QString getDirectory(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString path = input;
  QFileInfo info(path);
  if (info.isDir()) {
    return removeLastSeparatorFromPath(path);
  }
  QString output = path;
  output = output.replace("\\", "/");
  int index = output.lastIndexOf("/");
  if (index == -1) {
    return QString();
  }
  output = output.remove(index, output.size());
  return QDir::toNativeSeparators(output);
}

int saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
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
  return -1;
}

/**
 * initilazing an avisynth environment for the current input file
 **/
void AVSViewer::init(int start)
{
  this->send(tr("init %1").arg(start));
  if (start < 0) {
    start = 0;
  }
  m_current = -1; //seletzt den aktuellen FrameIndex zurück
  emit
  sendInfos(tr("initializing the avisynth script environment,.."));
  if (m_currentInput.isEmpty()) {
    this->send(tr("Current input is empty,.."));
    emit
    finished(-1);
    return;
  }
  if (m_env != 0) { //if I do not abort here application will crash on 'm_res.AsClip()' later
    this->send(tr("init called on existing environment,.."));
    return;
  }

  try {
    QString inputPath = QApplication::applicationDirPath();
    QLibrary avsDLL("\""+inputPath+QDir::separator()+"avisynth.dll\"");
    if (!avsDLL.isLoaded() && !avsDLL.load()) { //load avisynth.dll if it's not already loaded and abort if it couldn't be loaded
      QString error = avsDLL.errorString();
      if (!error.isEmpty()) {
        this->send(tr("Could not load avisynth.dll! %1").arg(error));
        emit
        finished(-2);
        return;
      }emit
      sendInfos(tr("Could not load avisynth.dll!"));
      emit finished(-3);
    }

    emit
    sendInfos(tr("loaded avisynth dll,.."));
    IScriptEnvironment* (*CreateScriptEnvironment)(
        int version) = (IScriptEnvironment*(*)(int)) avsDLL.resolve("CreateScriptEnvironment"); //resolve CreateScriptEnvironment from the dll
    emit
    sendInfos(tr("loaded CreateScriptEnvironment definition from dll,.."));
    m_env = CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION); //create a new IScriptEnvironment
    if (!m_env) { //abort if IScriptEnvironment couldn't be created
      this->send(tr("Could not create IScriptenvironment,..."));
      emit
      finished(-4);
      return;
    }

    emit
    sendInfos(tr("created an IScriptEnvironment,.."));
    try {
      this->send(tr("looking for avisynth version,.."));
      AVSValue as_version;
      as_version = m_env->Invoke("VersionString", AVSValue(&as_version, 0)); //get current version info
      m_version = as_version.AsString(); //save current version for later use
      this->send(tr("current avisynth version: %1").arg(m_version));
    } catch (...) {
      this->send(tr("Could get the current version,.."));
      emit
      finished(-5);
      return;
    }
    QString newContent, input = m_currentInput;
    bool invokeFFInfo = false;
    QFile file(input);
    bool ffmpegSource = false;
    if (file.open(QIODevice::ReadOnly)) {
      QString content = file.readAll(), ffms2Line;
      file.close();
      if (!content.contains("FFInfo()")) {
        bool ffms2Avs = false;
        foreach(QString line, content.split("\n"))
        {
          if (line.contains("FFMpegSource2(", Qt::CaseInsensitive)
              || line.contains("FFVideoSource(", Qt::CaseInsensitive)) {
            ffmpegSource = true;
          }
          if (line.contains("ffms2.dll", Qt::CaseInsensitive)) {
            ffms2Line = line;
            ffms2Line = ffms2Line.remove(0, ffms2Line.indexOf("\"") + 1);
            ffms2Line = ffms2Line.remove(ffms2Line.indexOf("\""), ffms2Line.size());
            ffms2Line = getDirectory(ffms2Line);
            ffms2Line += QDir::separator();
            ffms2Line += "FFMS2.avsi";
            ffms2Line = QDir::toNativeSeparators(ffms2Line);
          }
          if (line.contains("FFMS2.avs", Qt::CaseInsensitive)) {
            ffms2Avs = true;
          }
        }
        ui.ffinfoCheckBox->setEnabled(ffmpegSource);
        ffmpegSource = ui.ffinfoCheckBox->isChecked();
        if (ffmpegSource) {
          int index = content.indexOf("distributor()", Qt::CaseInsensitive);
          if (index != -1) {
            newContent = content;
            newContent = newContent.remove(index, newContent.size()).trimmed();
            if (!ffms2Avs && !ffms2Line.isEmpty()) {
              newContent += "\n";
              newContent += "Import(\"" + ffms2Line + "\")";
            }
            newContent += "\n";
            newContent += "SetMTMode(5)";
            newContent += "\n";
            newContent += "FFInfo()";
            newContent += "\n";
            newContent += "distributor()";
            newContent += "\n";
            newContent += "return last";
          } else if (!ffms2Avs && !ffms2Line.isEmpty()) {
            newContent = content;
            newContent = newContent.remove(index, newContent.size()).trimmed();
            if (content.contains("SetModeMT(")) {
              newContent += "\n";
              newContent += "SeMTMode(5)";
            }
            newContent += "\n";
            newContent += "Import(\"" + ffms2Line + "\")";
            newContent += "\n";
            newContent += "FFInfo()";
          } else {
            invokeFFInfo = true;
          }
        }
      }
    }
    if (!newContent.isEmpty()) {
      QString directory = getDirectory(m_currentInput);
      QString name = getFileName(m_currentInput);
      m_avsModified = QDir::toNativeSeparators(directory + QDir::separator() + name + "_tmp.avs");
      if (saveTextTo(newContent, m_avsModified) == 0) {
        input = m_avsModified;
      }
    }

    const char* infile = input.toUtf8(); //convert input name to char*
    emit
    sendInfos(tr("Importing %1 into environment,..").arg(infile));
    if (import(infile) != 0) {
      emit finished(-6);
      return;
    }
    if (!m_res.Defined()) {
      QString error = tr("Couldn't import:") + " " + input;
      error += "\r\n";
      error += tr("Script seems not to be a valid avisynth script.");
      emit
      sendInfos(error);
      emit
      finished(-7);
      return;
    }

    emit
    sendInfos(tr("Script seems be a valid,.."));
    emit
    sendInfos(" " + tr("initializating a clip,.."));
    m_clip = m_res.AsClip(); //get clip
    emit
    sendInfos(" " + tr("grabbing clip infos,.."));
    m_inf = m_clip->GetVideoInfo(); //get clip infos
    if (!m_inf.HasVideo()) { //abort if clip has no video
      sendInfos(tr("Input has no video stream -> aborting"));
      emit
      finished(-8);
      return;
    }

    emit
    sendInfos(" " + tr("checking colorspace,.."));
    bool reload = false;
    if (m_inf.IsRGB()) {
      this->send(" " + tr("current color space is RGB"));
    } else {
      if (m_inf.IsYV12()) {
        this->send(" " + tr("current color space is Yv12"));
      } else if (m_inf.IsRGB24()) {
        this->send(" " + tr("current color space is RGB24"));
      } else if (m_inf.IsRGB32()) {
        this->send(" " + tr("current color space is RGB32"));
      } else if (m_inf.IsYUY2()) {
        this->send(" " + tr("current color space is YUY2"));
      } else if (m_inf.IsYUV()) {
        this->send(" " + tr("current color space is YUV"));
      } else {
        this->send(" " + tr("current color space is unknown"));
      }
      if (this->invoke("ConvertToRGB") != 0) {
        this->killEnv();
        emit
        finished(-9);
        return;
      }
      reload = true;
    }
    if (invokeFFInfo) {
      if (this->invoke("FFInfo") != 0) {
        this->killEnv();
        emit
        finished(-10);
        return;
      }
      reload = true;
    }

    if (reload) {
      this->send(" " + tr("initializating the clip anew,.."));
      m_clip = m_res.AsClip(); // update clip
      emit
      sendInfos(" " + tr("grabbing clip infos,.."));
      m_inf = m_clip->GetVideoInfo(); // update clip info
    }

    emit
    sendInfos(" " + tr("grabbing clip length,.."));
    m_frameCount = m_inf.num_frames; //get frame count
    emit
    sendInfos("  -> " + tr("clip contains %1 frames,..").arg(m_frameCount));
    emit
    sendInfos(" " + tr("adjusting slider to frame count,.."));
    ui.frameHorizontalSlider->setMaximum(m_frameCount);
    ui.frameHorizontalSlider->resetMarks();
    ui.showLabel->setFixedSize(m_inf.width, m_inf.height);
    emit
    sendInfos(" " + tr("showing first frame,.."));
    this->showFrame(start); //show first frame
    ui.showLabel->setMaximumSize(32767, 32767);
  } catch (AvisynthError &err) { //catch AvisynthErrors
    this->send("-> " + tr("Avisynth error: %1").arg(err.msg));
  } catch (...) { //catch everything else
    this->send("-> " + tr("Unknown error"));
  }
}
/**
 * adjusts frame-index and frame to slider position
 **/
void AVSViewer::on_frameHorizontalSlider_valueChanged(int value)
{
  ui.frameNumberLabel->setText("(" + QString::number(value) + ")"); // update frame label
  if (!ui.frameHorizontalSlider->isSliderDown()) {
    this->showFrame(value); //show current frame
  }
}

/**
 * shows frame number i
 **/
void AVSViewer::showFrame(int i)
{
  if (m_env == 0 || i > m_frameCount || i == m_current) {
    return;
  }
  try {
    PVideoFrame f = m_clip->GetFrame(i, m_env); // get frame number i
    if (m_mult == 0) {
      m_mult = 1;
    }
    int width = m_inf.width;
    int height = m_inf.height;
    QImage image(f->GetReadPtr(), width, height, QImage::Format_RGB32); //create a QImage

    if (m_mult > 0 && m_mult != 1) {
      width = int(width * m_mult + 0.5);
      emit
      sendInfos(tr("Width: %1, Height: %2").arg(width).arg(height));
      image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    ui.showLabel->setText(QString());
    m_currentImage = image.mirrored();
    ui.showLabel->setPixmap(QPixmap::fromImage(m_currentImage)); //flip image, otherwise it is show heads down
    emit
    sendInfos(tr("showing frame: %1").arg(i));
    //ui.showLabel->show(); // show the frame
    m_current = i; //set m_current to i
    ui.frameHorizontalSlider->setSliderPosition(m_current); // adjust the slider position
    this->send(QObject::tr("showing frame number: %1 of %2").arg(m_current).arg(m_frameCount)); //adjust title bar
  } catch (...) {
    this->send(" " + tr("couldn't show frame,..."));
  }
}
void AVSViewer::killEnv()
{
  if (m_env != 0) {
    this->send(tr("Clean up old script environment,.."));
    m_res = 0;
    m_clip = 0;
    m_env->DeleteScriptEnvironment(); //delete the old script environment
    m_env = 0; // ensure new environment created next time
  }
  if (!m_avsModified.isEmpty()) {
    QFile::remove(m_avsModified);
    m_avsModified = QString();
  }
  ui.frameHorizontalSlider->resetMarks();
  if (m_cutSupport) {
    ui.cutListWidget->clear();
  }
}

/**
 * allows to select a .avs file, starts the initialization
 **/
void AVSViewer::on_openAvsPushButton_clicked()
{
  ui.showLabel->setText(tr("Opening new file,.."));
  QString name = tr("Select input file");
  QString select = tr("Input (*.avs)");
  QString inputPath = QApplication::applicationDirPath();
  QString input = QFileDialog::getOpenFileName(this, name, inputPath, select);
  if (!input.endsWith(".avs") || input.isEmpty()) { //abort if input does not end with .avs
    this->send("Current input is empty or not an .avs file,..");
    emit
    finished(-11);
    return;
  }
  this->killEnv();

  m_currentInput = input; //set current input
  ui.showLabel->setText(tr("Preparing environment for %1").arg(m_currentInput));
  emit
  sendInfos(tr("Current input: %1").arg(m_currentInput));
  this->init();
}

/**
 * shows the next frame
 **/
void AVSViewer::on_nextPushButton_clicked()
{
  if (m_current < 0) {
    m_current = 0;
  }
  this->showFrame(m_current + 1); // show next frame
}
/**
 * shows the previous frame
 **/
void AVSViewer::on_previousPushButton_clicked()
{
  if (m_current < 1) {
    m_current = 1;
  }
  this->showFrame(m_current - 1); // show previous frame
}
/**
 * shows the frame of the index where the slider was released
 **/
void AVSViewer::on_frameHorizontalSlider_sliderReleased()
{
  this->showFrame(ui.frameHorizontalSlider->sliderPosition()); // show frame for current slider position
}
