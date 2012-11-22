/*
 * AVSViewer.h
 *
 *  Created on: Feb 26, 2012
 *      Author: Selur
 */

#ifndef AVSVIEWER_H_
#define AVSVIEWER_H_

#include <QWidget>
#include "ui_AVSViewer.h"
#include "avisynth.h"
#include <QString>
#include <QStringList>
#include <QImage>

class IScriptEnvironment;

class AVSViewer : public QWidget
{
  Q_OBJECT
  public:
    AVSViewer(QWidget *parent = 0, QString path = QString(), double mult = 0, bool cutSupport =
                  false, QStringList keyframes = QStringList());
    ~AVSViewer();
    void init(int start = 0);

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
    void showFrame(int frame);
    int import(const char *inputFile, AVSValue &res, IScriptEnvironment* env);
    int invokeImportInternal(const char *inputFile, AVSValue &res, IScriptEnvironment* env);
    int invoke(const char *function);
    int invokeInternal(const char *function);
    void killEnv();
    void send(QString message);
    int handleFFInfo(QString &input, bool &invokeFFInfo);
    QString m_scanOrder;

  private slots:
    void on_frameHorizontalSlider_valueChanged(int value);
    void on_scanOrderComboBox_currentIndexChanged( const QString & text);
    void on_nextPushButton_clicked();
    void on_previousPushButton_clicked();
    void on_frameHorizontalSlider_sliderReleased();
    void on_openAvsPushButton_clicked();
    void on_ffinfoCheckBox_toggled();
    void on_saveImagePushButton_clicked();
    void on_setCutStartPushButton_clicked();
    void on_setCutEndPushButton_clicked();
    void on_addCutPushButton_clicked();
    void on_removeCutPushButton_clicked();
    void on_commitPushButton_clicked();
    bool isValidCut(int start, int end);
    void on_previousKeyPushButton_clicked();
    void on_nextKeyPushButton_clicked();

  signals:
    void finished(int state);
    void sendInfos(QString info);
    void cuts(QStringList cuts);
    void setInterlacedMode(QString mode);
};

#endif /* AVSVIEWER_H_ */
