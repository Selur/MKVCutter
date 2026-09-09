/*
 * AVSViewer.h
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#ifndef AVSVIEWER_H_
#define AVSVIEWER_H_

// avisynth.h v11 expects an AVS_Linkage* AVS_linkage symbol that the host
// populates from env->GetAVSLinkage() at runtime. MkvCutter is a host
// (creates the IScriptEnvironment itself), so we provide the symbol in
// avisynth_linkage.cpp and init it the first time we have a script
// environment.
#include <QWidget>
#include "ui_AVSViewer.h"
#include "avisynth.h"
#include <QString>
#include <QStringList>
#include <QImage>
#include <QIcon>


class IScriptEnvironment;class QPushButton;

class AVSViewer : public QWidget
{
  Q_OBJECT
  public:
    // containerFrameCount ist die Laenge, die mkvinfo fuer die Quelle zaehlt. Bei
    // feldcodierten (PAFF) Quellen zaehlt der Container Felder, der AviSynth-Clip aber
    // Frames -- Keyframeliste und Schnittliste stehen dann in unterschiedlichen
    // Einheiten (siehe B14). Der Faktor wird aus dem Verhaeltnis beider Laengen
    // gemessen statt aus MediaInfo geraten.
    AVSViewer(QWidget *parent = nullptr, QString path = QString(), double mult = 0, bool cutSupport = false, QStringList keyframes = QStringList(), int containerFrameCount = -1);
    ~AVSViewer();
    void init(int start = 0);
    // Fuer die --clinput-Steuerung: Schnittliste aus einer .cut-Datei laden bzw. den
    // Commit ausloesen, ohne Datei-Dialog und ohne Button. Liefert true, wenn danach
    // mindestens ein gueltiger Schnitt in der Liste steht.
    bool loadCutList(const QString &path);
    void commitCuts();

  private:
    Ui::AVSViewerClass ui;
    IScriptEnvironment* m_env;
    VideoInfo m_inf;
    PClip m_clip;
    int m_frameCount, m_current;
    QString m_currentInput, m_version, m_avsModified;
    AVSValue m_res;
    double m_mult;
    QImage m_currentImage;
    bool m_cutSupport;
    QStringList m_keyFrames;
    QString m_scanOrder;
    int m_displayWidth, m_displayHeight;
    int m_containerFrameCount, m_frameScale;
    void showFrame(int frame);
    int import(const char *inputFile, AVSValue &res, IScriptEnvironment* env);
    int invokeImportInternal(const char *inputFile, AVSValue &res, IScriptEnvironment* env);
    int invoke(const char *function);
    int invokeInternal(const char *function);
    void killEnv();
    void send(QString message);
    int showtime(QString &input);
    void addCut(int start, int end);
    void setButtonImage(QPushButton *button, QIcon image, const int height);
    void setButtonImages();
    bool isValid(int position);
    void setDisplay();
    void measureFrameScale();

  private slots:
    void on_frameHorizontalSlider_valueChanged(int value);
    void on_scanOrderComboBox_currentTextChanged(const QString & text);
    void on_nextPushButton_clicked();
    void on_previousPushButton_clicked();
    void on_frameHorizontalSlider_sliderReleased();
    void on_saveImagePushButton_clicked();
    void on_setCutStartPushButton_clicked();
    void on_setCutEndPushButton_clicked();
    void on_addCutPushButton_clicked();
    void on_removeCutPushButton_clicked();
    void on_commitPushButton_clicked();
    bool isValidCut(int start, int end);
    void on_previousKeyPushButton_clicked();
    void on_nextKeyPushButton_clicked();
    void on_jumpToPushButton_clicked();
    void on_savePushButton_clicked();
    void on_loadPushButton_clicked();

  signals:
    void finished(int state);
    void sendInfos(QString info);
    void cuts(QStringList cuts);
    // Container-Einheiten je AviSynth-Frame: 1 bei frame-codierten, 2 bei feldcodierten Quellen.
    void frameScale(int scale);
    void setInterlacedMode(QString mode);
};

#endif /* AVSVIEWER_H_ */
