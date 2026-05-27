2026-05-27T01:53:03Z | FIRE START | model=claude-sonnet-4-6
2026-05-27T01:57:05Z | FIX .github/workflows/windows-build-depends.yml:88 | protoc not found; Qt GUI disabled | add DEPENDS_PREFIX/native/bin to PATH in configure step
2026-05-27T11:57:43Z | FIX rcc/uic Qt host tools missing | symlink from native/bin into expected qtbase/bin path before make
2026-05-27T12:23:50Z | FIX move Qt host tools symlink BEFORE autogen+configure (qmake runs in configure)
2026-05-27T13:22:19Z | FIX explicit --with-qt-incdir/libdir/plugindir/bindir to configure (QtCore headers missing warning)
2026-05-27T15:11:05Z | FIX add libpng to depends (Qt detects yes but PNG check fails; bundled qt-libpng not exposed as pkg-config)
2026-05-27T16:09:25Z | FIX add Qt plugin -L paths to LDFLAGS (platforms/accessible/imageformats/styles); libqwindows.a et al exist but were not on search path
2026-05-27T16:32:57Z | FIX skip qtaccessiblewidgets static plugin check for Qt5 (plugin removed in Qt5)
