#ifndef MKVCUTTER_H
#define MKVCUTTER_H

#include <QWidget>
#include <QString>
#include <QStringList>

#include "tools/analyzer/MkvInfoSourceAnalyser.h"
#include "tools/viewer/AVSViewer.h"
#include "tools/FFindexCaller.h"
#include "ui_mkvcutter.h"

class MkvCutter : public QWidget
{
  Q_OBJECT
  public:
    MkvCutter(QWidget *parent = 0);
    ~MkvCutter();

  private:
    Ui::MkvCutterClass ui;
    QString m_currentInput, m_tempAvs, m_indexFile;
    int m_enabled, m_frameCount;
    QStringList m_keyframes, m_cuts;
    MkvInfoSourceAnalyser *m_mkvinfoAnalyser;
    AVSViewer *m_viewer;
    FFIndexCaller *m_ffindexCaller;
    void myconnect(const QObject * sender, const char * signal, const QObject * receiver,
                   const char * method, Qt::ConnectionType type = Qt::AutoConnection);
    void reset();
    bool createAVS();
    int saveTextTo(QString text, QString to);

  private slots:
    void on_openSourcePushButton_clicked();
    void enableGui(bool enable);
    void addInfo(QString infos);
    void setKeyFrames(QStringList list);
    void mkvAnalysefinished();
    void mkvAnalyseProgress(int linesRead);
    void setFrameCount(int count);
    void ffIndexerFinished(int state);
    void avsViewerFinished(int state);
    void setCutList(QStringList cuts);
    void ffindexProgress(int percent);
};

#endif // MKVCUTTER_H
