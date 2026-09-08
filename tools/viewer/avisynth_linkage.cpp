// avisynth_linkage.cpp
//
// avisynth.h v11 expects a runtime data symbol `AVS_linkage` (a pointer
// to AVS_Linkage) that the host populates by calling
// IScriptEnvironment::GetAVSLinkage(). All inline method bodies of
// VideoInfo / PClip / PVideoFrame / AVSValue look up their implementation
// through this pointer.
//
// The symbol is exported and defined here. AVSViewer::init() picks up
// the value via IScriptEnvironment::GetAVSLinkage() right after the
// script environment is created.

#include "avisynth.h"

extern "C" const AVS_Linkage* AVS_linkage = 0;
