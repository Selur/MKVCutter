CONFIG += qt
# CONFIG += console
TEMPLATE = app
TARGET = MkvCutter
QT += core \
    gui \
    widgets
win32 {
    DEFINES += BUILDTIME=\\\"$$system('echo %time%')\\\"
    DEFINES += BUILDDATE=\\\"$$system('echo %date:~6,4%%date:~3,2%%date:~0,2%')\\\"
}
else {
    DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M.%s')\\\"
    DEFINES += BUILDDATE=\\\"$$system(date '+%y.%m.%d')\\\"
}
isEqual(QT_MAJOR_VERSION, 5):QT += widgets # for all widgets
win32-msvc* {
    message(Building for Windows using Qt $$QT_VERSION)
    !contains(QMAKE_HOST.arch, x86_64):QMAKE_LFLAGS += /LARGEADDRESSAWARE # allow the use more of than 2GB of RAM on 32bit Windows

    QMAKE_LFLAGS_CONSOLE += /SUBSYSTEM:CONSOLE,5.01
    CONFIG += c++17 # C++17 support
    QMAKE_CXXFLAGS += /std:c++17

    QMAKE_LFLAGS += /STACK:64000000
    QMAKE_CXXFLAGS += -bigobj


    # /Zi aus den Debug-Compiler-Flags entfernen
    QMAKE_CXXFLAGS_DEBUG -= /Zi
    QMAKE_CXXFLAGS_DEBUG += /Z7
    QMAKE_CFLAGS_DEBUG   -= /Zi
    QMAKE_CFLAGS_DEBUG   += /Z7
    QMAKE_LFLAGS_DEBUG += /DEBUG
    QMAKE_LFLAGS_DEBUG += /OPT:REF
    QMAKE_LFLAGS_DEBUG += /OPT:ICF

    QMAKE_LFLAGS += /entry:mainCRTStartup

    QMAKE_CFLAGS_RELEASE += -WX
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -WX
    QMAKE_CFLAGS_RELEASE += -link notelemetry.obj

    # some Windows headers violate strictStrings rules
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CFLAGS -= -Zc:strictStrings
    QMAKE_CXXFLAGS -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE += /Zc:__cplusplus
    QMAKE_CFLAGS_RELEASE += /Zc:__cplusplus
    QMAKE_CFLAGS += /Zc:__cplusplus
    QMAKE_CXXFLAGS += /Zc:__cplusplus

    DEFINES += NOMINMAX

    QMAKE_CXXFLAGS += -permissive-
}

HEADERS += tools/FFIndexCaller.h \
    tools/Subtitletrack.h \
    tools/FFmpegVideoExtractor.h \
    tools/subtitlecutters/IdxSubCutter.h \
    tools/MkvSubtitleExtractor.h \
    tools/SubtitleCutter.h \
    tools/subtitlecutters/AssCutter.h \
    tools/subtitlecutters/Cutter.h \
    tools/subtitlecutters/SrtCutter.h \
    tools/analyzer/H264Parser.h \
    mywindows.h \
    tools/Converter.h \
    tools/MkvSplitCaller.h \
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
SOURCES += tools/FFIndexCaller.cpp \
    tools/FFmpegVideoExtractor.cpp \
    tools/subtitlecutters/IdxSubCutter.cpp \
    tools/MkvSubtitleExtractor.cpp \
    tools/SubtitleCutter.cpp \
    tools/subtitlecutters/AssCutter.cpp \
    tools/subtitlecutters/Cutter.cpp \
    tools/subtitlecutters/SrtCutter.cpp \
    tools/analyzer/H264Parser.cpp \
    tools/Converter.cpp \
    Globals.cpp \
    tools/MkvSplitCaller.cpp \
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
