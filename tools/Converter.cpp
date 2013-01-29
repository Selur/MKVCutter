/*
 * Converter.cpp
 *
 *  Created on: 29.01.2013
 *      Author: Selur
 */

#include "Converter.h"
#include <QStringList>
#include <QMessageBox>

void clearStart(QString &line)
{
  line = line.trimmed();
  line = line.remove(0, line.indexOf("=") + 1);
}

bool handleRest(QString line, QStringList &x264)
{
  if (line.startsWith("analyse=")) {
    clearStart(line);
    QStringList list = line.split(":");
    QString parameter = list.at(0);
    //bool adapt = parameter != "0";
    bool i4x4 = false;
    bool i8x8 = false;
    bool p4x4 = false;
    bool p8x8 = false;
    bool b8x8 = false;
    parameter = list.at(1);
    if (parameter == "0") {
      //none
    } else if (parameter == "0x1") {
      i4x4 = true;
    } else if (parameter == "0x3") {
      i4x4 = true;
      i8x8 = true;
    } else if (parameter == "0x10") {
      p8x8 = true;
    } else if (parameter == "0x11") {
      i4x4 = true;
      p8x8 = true;
    } else if (parameter == "0x13") {
      i4x4 = true;
      i8x8 = true;
      p8x8 = true;
    } else if (parameter == "0x30") {
      p4x4 = true;
      p8x8 = true;
    } else if (parameter == "0x31") {
      i4x4 = true;
      p4x4 = true;
      p8x8 = true;
    } else if (parameter == "0x33") {
      i4x4 = true;
      i8x8 = true;
      p4x4 = true;
      p8x8 = true;
    } else if (parameter == "0x110") {
      p8x8 = true;
      b8x8 = true;
    } else if (parameter == "0x111") {
      i4x4 = true;
      p8x8 = true;
      b8x8 = true;
    } else if (parameter == "0x113") {
      i4x4 = true;
      i8x8 = true;
      p8x8 = true;
      b8x8 = true;
    } else if (parameter == "0x130") {
      p4x4 = true;
      p8x8 = true;
      b8x8 = true;
    } else if (parameter == "0x131") {
      i4x4 = true;
      p4x4 = true;
      p8x8 = true;
      b8x8 = true;
    } else if (parameter == "0x133") {
      i4x4 = true;
      i8x8 = true;
      p4x4 = true;
      p8x8 = true;
      b8x8 = true;
    }
    x264 << "--partitions";
    parameter = QString();
    bool first = true;
    if (i4x4 && i8x8 && p8x8 && p4x4 && b8x8) {
      x264 << "all";
      return true;
    } else if (!i4x4 && !i8x8 && !p8x8 && !p4x4 && !b8x8) {
      x264 << "none";
      return true;
    }
    if (i4x4) {
      if (!first) {
        parameter += ",";
        first = false;
      }
      parameter += "i4x4";
    }
    if (i8x8) {
      if (!first) {
        parameter += ",";
        first = false;
      }
      parameter += "i8x8";
    }
    if (p8x8) {
      if (!first) {
        parameter += ",";
        first = false;
      }
      parameter += "p8x8";
    }
    if (p4x4) {
      if (!first) {
        parameter += ",";
        first = false;
      }
      parameter += "p4x4";
    }
    if (b8x8) {
      if (!first) {
        parameter += ",";
        first = false;
      }
      parameter += "b8x8";
    }
    x264 << parameter;
    return true;
  }
  if (line.startsWith("deadzone=")) {
    clearStart(line);
    QStringList elems = line.split(",");
    x264 << "--deadzone-inter" << elems.at(0);
    x264 << "--deadzone-intra" << elems.at(1);
    return true;
  }
  if (line.startsWith("fast_pskip=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--no-fast-pskip";
    }
    return true;
  }
  if (line.startsWith("sliced_threads=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--sliced-threads";
    }
    return true;
  }
  if (line.startsWith("decimate=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--no-dct-decimate";
    }
    return true;
  }
  if (line.startsWith("constrained_intra=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--constrained-intra";
    }
    return true;
  }
  if (line.startsWith("scenecut=")) {
    clearStart(line);
    x264 << "--scenecut" << line;
    return true;
  }
  if (line.startsWith("intra_refresh=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--intra-refresh";
    }
    return true;
  }
  if (line.startsWith("rc_lookahead=")) {
    clearStart(line);
    x264 << "--rc-lookahead" << line;
    return true;
  }
  if (line.startsWith("mbtree=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--no-mbtree" << line;
    }
    return true;
  }
  if (line.startsWith("nr=")) {
    clearStart(line);
    if (line != "0") {
      x264 << "--nr" << line;
    }
    return true;
  }
  if (line.startsWith("vbv_maxrate")) {
      clearStart(line);
      x264 << "--vbv-maxrate" << line;
      return true;
  }
  if (line.startsWith("vbv_bufsize")) {
      clearStart(line);
      x264 << "--vbv-bufsize" << line;
      return true;
  }
  if (line.startsWith("nal_hrd")) {
      clearStart(line);
      x264 << "--nal-hrd" << line;
      return true;
  }
  return false;
}

bool handleMotion(QString line, QStringList &x264)
{
  if (line.startsWith("subme=")) {
    clearStart(line);
    x264 << "--subme " + line;
    return true;
  }
  if (line.startsWith("me_range=")) {
    clearStart(line);
    x264 << "--merange " + line;
    return true;
  }
  if (line.startsWith("mv_range=")) {
    clearStart(line);
    x264 << "--mvrange " + line;
    return true;
  }
  if (line.startsWith("chroma_me=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--no-chroma-me";
    }
    return true;
  }
  if (line.startsWith("ratetol=")) {
    clearStart(line);
    x264 << "--ratetol " + line;
    return true;
  }
  if (line.startsWith("me=")) {
    clearStart(line);
    x264 << "--me " + line;
    return true;
  }
  if (line.startsWith("8x8dct=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--8x8dct";
    } else {
      x264 << "--no-8x8dct";
    }
    return true;
  }
  return false;
}

bool handleFrame(QString line, QStringList &x264)
{
  if (line.startsWith("cabac=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--cabac";
    } else {
      x264 << "--no-cabac";
    }
    return true;
  }
  if (line.startsWith("trellis=")) {
    clearStart(line);
    x264 << "--trellis " + line;
    return true;
  }
  if (line.startsWith("aq=")) {
    clearStart(line);
    QStringList elems = line.split(":");
    QString mode = elems.at(0);
    x264 << "--aq-mode " + mode;
    if (mode != "0") {
      x264 << "--aq-strength " + elems.at(1);
    }
    return true;
  }
  if (line.startsWith("ip_ratio=")) {
    clearStart(line);
    x264 << "--ipratio " + line;
    return true;
  }
  if (line.startsWith("pb_ratio=")) {
    clearStart(line);
    x264 << "--pbratio " + line;
    return true;
  }
  if (line.startsWith("qpmin=")) {
    clearStart(line);
    x264 << "--qpmin " + line;
    return true;
  }
  if (line.startsWith("qpmax=")) {
    clearStart(line);
    x264 << "--qpmax " + line;
    return true;
  }
  if (line.startsWith("qpstep=")) {
    clearStart(line);
    x264 << "--qpstep " + line;
    return true;
  }
  if (line.startsWith("interlaced")) {
    clearStart(line);
    if (line == "fake") {
      x264 << "--fake-interlaced";
    } else if (line != "0") {
      x264 << "--" + line;
    }
    return true;
  }
  if (line.startsWith("bluray_compat=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--bluray-compat";
    }
    return true;
  }
  if (line.startsWith("qcomp=")) {
    clearStart(line);
    x264 << "--qcomp " + line;
    return true;
  }
  if (line.startsWith("cplxblur=")) {
    clearStart(line);
    x264 << "--cplxblur " + line;
    return true;
  }
  if (line.startsWith("qblur=")) {
    clearStart(line);
    x264 << "--qblur " + line;
    return true;
  }
  if (line.startsWith("chroma_qp_offset=")) {
    clearStart(line);
    x264 << "--qblur " + line;
    return true;
  }
  if (line.startsWith("psy=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--no-psy " + line;
    }
    return true;
  }
  if (line.startsWith("psy_rd=")) {
    clearStart(line);
    x264 << "--psy-rd " + line;
    return true;
  }
  if (line.startsWith("deblock=")) {
    clearStart(line);
    QStringList elems = line.split(":");
    if (elems.at(0) == "0") {
      x264 << "--no-deblock";
    } else {
      x264 << "--deblock " + elems.at(1) + ":" + elems.at(2);
    }
    return true;
  }

  return false;
}

bool handleGop(QString line, QStringList &x264)
{
  if (line.startsWith("ref=")) {
    clearStart(line);
    x264 << "--ref " + line;
    return true;
  }
  if (line.startsWith("mixed_ref=")) {
    clearStart(line);
    int value = line.toInt();
    switch (value) {
      case 0 :
        x264 << "--no-mixed-refs";
        break;
      default :
        break;
    }
    return true;
  }
  if (line.startsWith("open_gop=")) {
    clearStart(line);
    if (line == "1") {
      x264 << "--open-gop";
    }
    return true;
  }
  if (line.startsWith("weightp=")) {
    clearStart(line);
    x264 << "--weightp " + line;
    return true;
  }
  if (line.startsWith("keyint=")) {
    clearStart(line);
    x264 << "--keyint " + line;
    return true;
  }
  if (line.startsWith("keyint_min=")) {
    clearStart(line);
    x264 << "--min-keyint " + line;
    return true;
  }
  return false;
}

bool handleBFrames(QString line, QStringList &x264)
{
  if (line.startsWith("bframes=")) {
    clearStart(line);
    x264 << "--bframes " + line;
    return true;
  }
  if (line.startsWith("b_pyramid=")) {
    clearStart(line);
    int value = line.toInt();
    switch (value) {
      case 1 :
        line = "normal";
        break;
      case 2 :
        line = "strict";
        break;
      default :
        line = "none";
        break;
    }
    x264 << "--b-pyramid " + line;
    return true;
  }
  if (line.startsWith("b_adapt=")) {
    clearStart(line);
    x264 << "--b-adapt " + line;
    return true;
  }
  if (line.startsWith("b_bias=")) {
    clearStart(line);
    x264 << "--b-bias " + line;
    return true;
  }
  if (line.startsWith("direct=")) {
    clearStart(line);
    int value = line.toInt();
    switch (value) {
      case 1 :
        line = "spatial";
        break;
      case 2 :
        line = "temporal";
        break;
      case 3 :
        line = "optimal";
        break;
      default :
        line = "auto";
        break;
    }
    x264 << "--direct " + line;
    return true;
  }
  if (line.startsWith("weightb=")) {
    clearStart(line);
    if (line == "0") {
      x264 << "--weightb";
    }
    return true;
  }
  return false;
}

QString Converter::encodingSettingsToX264(QString line)
{
  QStringList x264;
  QStringList lines = line.split("/");
  QStringList ignored;
  foreach(line, lines) {
    line = line.trimmed();
    if (handleBFrames(line, x264)) {
      continue;
    }
    if (handleGop(line, x264)) {
      continue;
    }
    if (handleFrame(line, x264)) {
      continue;
    }
    if (handleMotion(line, x264)) {
      continue;
    }
    if (handleRest(line, x264)) {
      continue;
    }
    ignored << line;
  }
  return x264.join(" ");
}
