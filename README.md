# MKVCutter

A small GUI for frame-accurate, mostly lossless cutting of H.264 Matroska files.

Only the GOPs that a cut actually lands inside are re-encoded; everything between them is
copied through untouched. Audio and subtitles are cut alongside the video and put back where
the picture is.

> Still an alpha. It gets looked at when something is badly broken, and it is Windows-only.

---

## How it works

1. **`mkvinfo`** — key frame positions, frame/field count, frame rate, track layout.
2. **`MediaInfo`** — scan type and field order, bit depth, chroma subsampling, audio delays,
   and the original x264 settings if the source carries them.
3. **`ffmsindex`** — index the source so the preview can seek.
4. **Preview** — an AviSynth script calling `FFVideoSource`, shown in the built-in viewer.
   The clip length is compared with the container's count to work out whether the source is
   field-coded (see [Interlaced sources](#interlaced-sources)).
5. **You pick the parts you want to keep.**
6. **Cut lists** — the selection is turned into `mkvmerge --split parts-frames:` ranges
   snapped outwards to key frames, plus `Trim()` calls for the GOPs that have to be
   re-encoded. For variable frame rate sources the timestamp file is cut to match.
7. **You set the output file and the temp folder.**
8. **`mkvmerge`** splits the video into parts on key frame boundaries.
9. **`ffmpeg`** extracts the parts to H.264 elementary streams.
10. **`h264_parse`** analyses one of the parts that is *not* being re-encoded, so the new
    GOPs can be encoded with matching SPS settings.
11. **AviSynth scripts** (`FFVideoSource` + `Trim`) feed the GOPs that need re-encoding.
12. **`x264`** re-encodes them — profile, level, reference frames, B-frames, CABAC, key frame
    interval and field order are taken from the source; quality is a fixed `--crf 19`. If the
    source carries its original x264 settings, those are reused verbatim instead.
13. **`mkvmerge`** cuts the audio into one file per part, and subtitles are extracted and cut.
14. **`mkvmerge`** muxes everything back together, giving each audio part the offset that puts
    it back at the start of its video part.

### Interlaced sources

Field-coded (PAFF) H.264 puts one Matroska block per *field*, so the container counts roughly
twice as many units as the AviSynth clip has frames, and the key frame list is in fields.
MBAFF is frame-coded and counts the same either way.

MKVCutter measures the ratio instead of trusting MediaInfo's scan type: the viewer knows the
real clip length, `mkvinfo` the container length. Everything downstream works in container
units, and only the `Trim()` values are converted back.

### Audio placement

Compressed audio can only be cut losslessly on frame boundaries — 32 ms for AC-3 — so every
part comes out slightly shorter or longer than asked for. The parts are therefore cut into
separate files and joined while muxing, each with the offset that puts it back at the start of
its video part. Without that, the rounding errors add up and the sound runs ahead of the
picture.

---

## Using it

1. Start MKVCutter.
2. Press **Open Source** and pick the `.mkv` you want to cut.
3. Wait for `mkvinfo`, `MediaInfo` and the indexer to finish — the cut view opens by itself.
4. In the cut view you select the parts you want to **keep**:
   - go to the start of a section, press **Cut-Start**
   - go to its end, press **Cut-End** — the section is highlighted on the slider
   - press **Add to Cut-List**
   - repeat for every section you want to keep
   - **Save** writes the list to a `.cut` file, **Load** reads one back
5. Press **Commit Cut-List**.
6. Set **Output** (file) and **Temp folder**.
7. Press **Next** and wait for the *Finished!* pop-up.

### Cut list format

One line per kept section, `start#end`, frame numbers, end exclusive:

```
11683#11919
12912#13124
```

### Driving it from the command line

`--clinput=<command>` is repeatable and processed in order. `open` resets the GUI, so it has
to come first:

```
MkvCutter.exe --clinput=open:in.mkv --clinput=output:out.mkv --clinput=temp:C:\tmp ^
              --clinput=cutlist:cuts.cut --clinput=commit --clinput=next
```

| Command | Effect |
| --- | --- |
| `open:<path>` | load a `.mkv` |
| `output:<path>` | set the output file |
| `temp:<path>` | set the temp folder |
| `cutlist:<path>` | load a `.cut` file into the cut view |
| `commit` | commit the cut list |
| `next` | start the processing |
| `scanorder:<auto\|bff\|tff>` | override the detected field order |
| `keepIntermediate:<on\|off>` | keep the temporary files |
| `quit` | close the application |

---

## What it handles

| | |
| --- | --- |
| Video | H.264 in Matroska, 8 and 10 bit, 4:2:0 / 4:2:2 / 4:4:4, progressive, MBAFF and PAFF |
| Frame rate | constant and variable (the timestamp file is cut along) |
| Audio | copied through, cut on frame boundaries, several tracks per file |
| Subtitles | SRT, ASS/SSA and VobSub are cut |
| Fast path | if every cut already sits on a key frame, nothing is re-encoded at all |

## Limitations

- **H.264 only.** Other codecs may go through, but the cuts will not be frame accurate.
- The **user interface is ugly** and has no bells and whistles.
- **x264 is driven crudely** — the settings follow the source, but quality is always
  `--crf 19` and there is no way to choose.
- **PGS subtitles are not cut** and are dropped from the output, as is any other subtitle
  format the cutter does not know. The log says which tracks were dropped.
- **Chapters, attachments and global tags are dropped.**
- **Audio is not re-encoded**, so a cut can only land on an audio frame boundary. The offset
  correction keeps that below one frame per part instead of letting it accumulate.
- **Windows only**, mainly because AviSynth does the decoding and the preview. Using ffmpeg
  instead should make a port possible.
