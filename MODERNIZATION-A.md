2026-05-27T01:53:03Z | FIRE START | model=claude-sonnet-4-6
2026-05-27T01:57:05Z | FIX .github/workflows/windows-build-depends.yml:88 | protoc not found; Qt GUI disabled | add DEPENDS_PREFIX/native/bin to PATH in configure step
2026-05-27T11:57:43Z | FIX rcc/uic Qt host tools missing | symlink from native/bin into expected qtbase/bin path before make
2026-05-27T12:23:50Z | FIX move Qt host tools symlink BEFORE autogen+configure (qmake runs in configure)
