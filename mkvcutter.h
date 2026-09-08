#ifndef MKVCUTTER_H
#define MKVCUTTER_H

#include <QWidget>
#include <QString>
#include <QHash>
#include <QStringList>

#include "tools/analyzer/MkvInfoSourceAnalyser.h"
#include "tools/analyzer/MediaInfoAnalyser.h"
#include "tools/analyzer/H264Parser.h"
#include "tools/viewer/AVSViewer.h"
#include "tools/FFIndexCaller.h"
#include "tools/MkvSplitCaller.h"
#include "tools/X264Caller.h"
#include "tools/MkvMerger.h"
#include "tools/MkvVideoExtractor.h"
#include "tools/MkvTimeExtractor.h"
#include "tools/MkvSubtitleExtractor.h"
#include "tools/SubtitleCutter.h"
#include "tools/FFmpegVideoExtractor.h"

#include "ui_mkvcutter.h"

struct cutTyp1;

class MkvCutter : public QWidget
{
  Q_OBJECT
  public:
    MkvCutter(QWidget *parent = 0);
    ~MkvCutter();

    // CLI control slots (called from main.cpp via --clinput)
    void cliOpen(const QString &path);
    void cliSetOutput(const QString &path);
    void cliSetTemp(const QString &path);
    void cliSetKeepIntermediate(bool keep);
    void cliNext();
    void cliSetScanOrder(const QString &mode);
    void cliLoadCutList(const QString &path);
    void cliCommit();


  private:
    Ui::MkvCutterClass ui;
    QString m_currentInput, m_tempAvs, m_indexFile, m_currentOutput, m_tempFolder;
    QString m_avcProfileLevel, m_audioFormat;
    bool m_avcCabac;
    int m_avcRefFrames, m_enabled, m_frameCount;
    QStringList m_keyframes, m_cuts, m_splitFiles, m_tempReencodeAvs, m_videoEncodingCalls;
    QStringList m_reencodedVideoFiles;
    double m_fps;
    QHash<QString, QString> m_trimming;
    QList<cutTyp1> m_cutList;
    QSet<int> m_mkvmergeIntSplitList;
    MkvInfoSourceAnalyser *m_mkvinfoAnalyser;
    MediaInfoAnalyser *m_mediaInfoAnalyser;
    AVSViewer *m_viewer;
    FFIndexCaller *m_ffindexCaller;
    MkvSplitCaller *m_mkvVideoSplitCaller, *m_mkvAudioCutCaller;
    MkvMerger *m_mkvMerger;
    X264Caller *m_x264;
    QStringList m_mkvVideoParts, m_mkvAudioAndSubtitleParts;
    QString m_audioFile;
    int m_averageBitrate;
    QStringList m_audioSplitFiles, m_extractionFiles, m_toDelete;
    int m_videoTrackID;
    //MkvVideoExtractor *m_extractor;
    FFmpegVideoExtractor *m_extractor;
    MkvTimeExtractor *m_timeextractor;
    double m_aspectRatio;
    QString m_interlaced, m_mediaInfoScanorder;
    bool m_vfr;
    QString m_timecodes, m_x264Settings;
    int m_averageKeyDistance;
    bool m_paff;
    QString m_minKey, m_maxKey;
    H264Parser *m_h264Parser;
    int m_weightedP, m_weightedB, m_bframes, m_qpMin;
    int m_chromaOffset;
    QString m_toAnalyse;
    QList<SubtitleTrack> m_subtitles;
    MkvSubtitleExtractor *m_mkvSubtitleExtractor;
    SubtitleCutter *m_subtitleCutter;
    QStringList m_cutSubtitles;
    QStringList m_subtitleToCut;
    bool m_keyframeonly;
    bool m_hasAudio;
    int m_sps;
    int m_width, m_height;
    QHash<QString,QString> m_audioDelays;
    QStringList m_inputTimeCodes;
    // --clinput wird komplett vor dem Start der Pipeline abgearbeitet; zu dem Zeitpunkt
    // gibt es weder Viewer noch Schnittliste. Diese Wuensche werden deshalb gemerkt und
    // am passenden Meilenstein eingeloest (jeweils genau einmal).
    QString m_cliCutList;
    bool m_cliCommit, m_cliNext;

    void myconnect(const QObject * sender, const char * signal, const QObject * receiver,
                   const char * method, Qt::ConnectionType type = Qt::AutoConnection);
    void reset(bool andInit=true);
    void initTools();
    bool createLibAVSourceAVS();
    QStringList keyFrameTimes();
    void buildAndCallMkvMerge();
    void createVideoReencodeCall(QString avisynthFile);
    void createAvisynthSkript(QString filename, QString trim);
    void startVideoReencoding();
    void cleanUpAndMerge();
    void handleSplitFiles();
    void buildTrimAndPartsList();
    void buildCutList();
    void cutAudio();
    void startExtraction();
    QString getSmallest();
    cutTyp1 findCutForFrame(int frame, const bool start);
    void extractTimeCodes();
    QString cutTimecodes(QString timecodes);
    void startViewer();
    void addAudioAndSubtitleCuts(const int &start, const int &end);
    void addVideoCut(const int &start, const int &end, const bool &interlaced);
    void parseOriginal();
    void createReencodeCalls();
    bool createAVS();

  protected slots:
    void on_openSourcePushButton_clicked();
    void setInput(QString input);
    void on_outputPushButton_clicked();
    void on_tempPushButton_clicked();
    void on_nextPushButton_clicked();
    void enableGui(bool enable);
    void addInfo(QString infos);
    void setKeyFrames(QStringList list);
    void setAspectRatio(double aspect);
    void mkvAnalysefinished();
    void mkvAnalyseProgress(int linesRead);
    void setFrameCount(int count);
    void ffIndexerFinished(int state);
    void avsViewerFinished(int state);
    void setCutList(QStringList cuts);
    void ffindexProgress(int percent);
    void setFPS(double framerate);
    void mkvSplitFinished(int exitstate);
    void mkvAudioCutFinished(int exitstate);
    void mkvsplitProgress(int percent);
    void setSplitFiles(QStringList splitFiles);
    void setAvcProfileLevel(QString pl);
    void setAvcCabac(bool cabac);
    void setAvcRefFrames(int frames);
    void mediaInfoFinished(int exitstate);
    void setAudioFormat(QString format);
    void mkvMergerProgress(int percent);
    void mkvExtractorProgress(int percent);
    void mkvMergerFinished(int exitstate);
    void mkvExtractorFinished(int exitstate);
    void x264Progress(int percent);
    void x264Finished(int exitstate);
    void setAverageBitrate(int bitrate);
    void setAudioSplitFiles(QStringList splitFiles);
    void setVideoTrackID(int id);
    void setInterlaced(QString interlaced);
    void setInterlacedMode(QString interlaced);
    void setFrameRateMode(bool vfr);
    void finishedTimeCodeExtraction(int exitstate);
    void setTimecodes(QString timecodes);
    void setX264Settings(QString settings);
    void deleteFiles();
    void setMinKeyInt(QString min);
    void setMaxKeyInt(QString max);
    void h264ParseFinished();
    void setWeightedP(int value);
    void setWeightedB(int value);
    void setBFrames(int value);
    void setQPmin(int value);
    void setChromaOffset(int value);
    void subtitleTrack(SubtitleTrack track);
    void mkvSubtitleExtractorFinished(int state);
    void mkvSubtitleCutterFinished(int state);
    void setHasAudio(bool hasAudio);
    void setSps(int sps);
    void setResolution(QString width, QString height);
    void setTheAudioDelays(QHash<QString, QString> audioDelays);
};

#endif // MKVCUTTER_H
