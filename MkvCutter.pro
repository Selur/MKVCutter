CONFIG += qt \
    console
TEMPLATE = app
TARGET = MkvCutter
QT += core \
    gui
isEqual(QT_MAJOR_VERSION, 5):QT += widgets # for all widgets

TEMPLATE = app
TARGET = MkvCutter
INCLUDEPATH += .

# Input
HEADERS += DropPushButton.h \
           Globals.h \
           mkvcutter.h \
           mywindows.h \
           tools/Converter.h \
           tools/MkvMerger.h \
           tools/MkvSplitCaller.h \
           tools/MkvSubtitleExtractor.h \
           tools/MkvTimeExtractor.h \
           tools/MkvVideoExtractor.h \
           tools/SubtitleCutter.h \
           tools/X264Caller.h \
           tools/analyzer/H264Parser.h \
           tools/analyzer/MediaInfoAnalyser.h \
           tools/analyzer/MkvInfoSourceAnalyser.h \
           tools/subtitlecutters/AssCutter.h \
           tools/subtitlecutters/Cutter.h \
           tools/subtitlecutters/SrtCutter.h \
           tools/viewer/avisynth.h \
           tools/viewer/AVSViewer.h \
           tools/viewer/ImageLabel.h \
           tools/viewer/MarkSlider.h \
           tools/viewer/stdafx.h
FORMS += mkvcutter.ui tools/viewer/AVSViewer.ui
SOURCES += DropPushButton.cpp \
           Globals.cpp \
           main.cpp \
           mkvcutter.cpp \
           mkvcutter_plugin_import.cpp \
           tools/Converter.cpp \
           tools/MkvMerger.cpp \
           tools/MkvSplitCaller.cpp \
           tools/MkvSubtitleExtractor.cpp \
           tools/MkvTimeExtractor.cpp \
           tools/MkvVideoExtractor.cpp \
           tools/SubtitleCutter.cpp \
           tools/X264Caller.cpp \
           tools/analyzer/H264Parser.cpp \
           tools/analyzer/MediaInfoAnalyser.cpp \
           tools/analyzer/MkvInfoSourceAnalyser.cpp \
           tools/subtitlecutters/AssCutter.cpp \
           tools/subtitlecutters/Cutter.cpp \
           tools/subtitlecutters/SrtCutter.cpp \
           tools/viewer/AVSViewer.cpp \
           tools/viewer/ImageLabel.cpp \
           tools/viewer/interface.cpp \
           tools/viewer/MarkSlider.cpp
RESOURCES += resources.qrc
