#ifndef MKVCUTTER_H
#define MKVCUTTER_H

#include <QWidget>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QHash>

#include "tools/analyzer/MkvInfoSourceAnalyser.h"
#include "tools/viewer/AVSViewer.h"
#include "tools/FFindexCaller.h"
#include "tools/MkvSplitCaller.h"
#include "ui_mkvcutter.h"

struct cutTyp1;

class MkvCutter : public QWidget
{
  Q_OBJECT
  public:
    MkvCutter(QWidget *parent = 0);
    ~MkvCutter();

  private:
    Ui::MkvCutterClass ui;
    QString m_currentInput, m_tempAvs, m_indexFile, m_currentOutput, m_tempFolder;
    int m_enabled, m_frameCount;
    QStringList m_keyframes, m_cuts;
    double m_fps;
    QHash <QString, QString> m_trimming;
    QList<cutTyp1> m_cutList;
    QSet<int> m_mkvmergeIntSplitList;
    MkvInfoSourceAnalyser *m_mkvinfoAnalyser;
    AVSViewer *m_viewer;
    FFIndexCaller *m_ffindexCaller;
    MkvSplitCaller *m_mkvSplitCaller;
    void myconnect(const QObject * sender, const char * signal, const QObject * receiver,
                   const char * method, Qt::ConnectionType type = Qt::AutoConnection);
    void reset();
    bool createAVS();
    void buildCutList();
    void buildAndCallMkvMerge();

  private slots:
    void on_openSourcePushButton_clicked();
    void on_outputPushButton_clicked();
    void on_tempPushButton_clicked();
    void on_nextPushButton_clicked();
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
    void setFPS(double framerate);
    void mkvSplitFinished(int exitstate);
    void mkvsplitProgress(int percent);
};

#endif // MKVCUTTER_H
