CONFIG += qt
TEMPLATE = app
TARGET = MkvCutter
QT += core \
    gui
HEADERS += tools/Converter.h \
    tools/MkvSplitCaller.h \
    tools/FFIndexCaller.h \
    tools/viewer/AVSViewer.h \
    tools/viewer/ImageLabel.h \
    tools/viewer/MarkSlider.h \
    tools/viewer/avisynth.h \
    tools/viewer/stdafx.h \
    tools/analyzer/MkvInfoSourceAnalyser.h \
    mkvcutter.h \
    Globals.h \
    tools/analyzer/MediaInfoAnalyser.h \
    tools/MkvMerger.h \
    tools/X264Caller.h \
    tools/MkvVideoExtractor.h \
    DropPushButton.h \
    tools/MkvTimeExtractor.h
SOURCES += tools/Converter.cpp \
    Globals.cpp \
    tools/MkvSplitCaller.cpp \
    tools/FFIndexCaller.cpp \
    tools/viewer/AVSViewer.cpp \
    tools/viewer/ImageLabel.cpp \
    tools/viewer/MarkSlider.cpp \
    tools/viewer/interface.cpp \
    tools/analyzer/MkvInfoSourceAnalyser.cpp \
    main.cpp \
    mkvcutter.cpp \
    tools/analyzer/MediaInfoAnalyser.cpp \
    tools/MkvMerger.cpp \
    tools/X264Caller.cpp \
    tools/MkvVideoExtractor.cpp \
    DropPushButton.cpp \
    tools/MkvTimeExtractor.cpp
FORMS += tools/viewer/AVSViewer.ui \
    mkvcutter.ui
RESOURCES += resources.qrc
