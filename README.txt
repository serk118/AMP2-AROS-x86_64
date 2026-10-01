AMP2 for AROS x86_64
====================

AMP2 is a multimedia player for AmigaOS by Mathias Roslund (Aminet misc/emu/AMP2).
This is a port of the AROS-adapted AMP2 source to 64-bit AROS x86_64, plus a
small GUI launcher.

Status: WORKING. Video and audio play on hosted AROS x86_64 with no traps.


What is here
------------

  bin/       Prebuilt AROS x86_64 binaries:
               AMP          - the player
               AMP2-GUI     - small launcher (Play File... / Quit)
               *.AMP        - the codec plugins
             Install as:

               Extras/Audio/AMP2/AMP
               Extras/Audio/AMP2/AMP2-GUI
               Extras/Audio/AMP2/Plugin/*.AMP

             (The Plugin/ folder sits next to AMP.)

  source/    The player source tree (see source/Makefile.aros).
  gui/       GUI launcher source (amp2-gui.c, Makefile.aros).
  tests/     Sample media covering the supported formats (see below).


Building
--------

  You need an x86_64 AROS cross toolchain (GCC) with headers/libs.

  Core player:
    cd source
    make -f Makefile.aros -j4           -> source/AMP

  Plugins:
    cd source/plugin
    make -f Makefile.aros -j4           -> source/dist/Plugin/*.AMP

  GUI launcher:
    cd gui
    make -f Makefile.aros               -> gui/AMP2-GUI

  The Makefiles use AROS_SYSROOT / CROSS_PREFIX; adjust them for your toolchain.


Running
-------

  Command line:
    AMP <file> WINDOW

  GUI:
    AMP2-GUI
  (Play File... opens a file requester starting in Extras/Audio/AMP2/tests.)


Supported formats (verified on hosted AROS x86_64, 0 traps)
----------------------------------------------------------

  AVI        Cinepak (cvid), MS Video-1 (MSVC), Motion-JPEG, MPEG-4 (DIVX/XviD),
             MS-MPEG4 (MP43), Sorenson 1 (SVQ1); audio PCM, AC3, MP2
  MOV/QT     Sorenson 1, MJPEG, MJPEG-A (mjpa/mjpb), MPEG-4 (mp4v); audio PCM
             (raw/sowt/twos), IMA4, MACE3/MACE6
  RealMedia  RV10
  MPEG       MPEG-1 / MPEG-2 video and system streams
  Raw audio  MP3, MP2, AC3


Not supported (no such decoder/parser in the bundled code)
----------------------------------------------------------

  H.264, SVQ3, RealVideo RV20/RV30, FLIC/FLI/FLC and IFF ANIM (no parser),
  XAnim (needs external codec modules), DVD/VCD (untested, need a device).


Test files
----------

  avi_cvid_pcm.avi     AVI  Cinepak + PCM
  avi_msvc_pcm.avi     AVI  MS Video-1 + PCM
  avi_mjpeg_pcm.avi    AVI  MJPEG + PCM
  avi_mp43_pcm.avi     AVI  MS-MPEG4 + PCM
  avi_svq1_pcm.avi     AVI  Sorenson 1 + PCM
  avi_divx_pcm.avi     AVI  MPEG-4 + PCM
  mov_svq1_ima4.mov    QT   Sorenson 1 + IMA4
  mov_mjpeg_pcm.mov    QT   MJPEG + PCM
  real_qt_audio.qt     QT   MJPEG + PCM (real QuickTime sample)
  mpeg1_sys.mpg        MPEG-1 system stream
  raw.mp3 / raw.mp2 / raw.ac3   raw audio


Credits
-------

  Original AMP2 (c) Mathias Roslund.
  AROS adaptation by the AROS/AMP2 contributors; x86_64 64-bit fixes and GUI by
  serk118.
