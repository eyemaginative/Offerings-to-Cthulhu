2026-05-27T02:06:28Z | FIRE START | model=claude-opus-4-7
2026-05-27T02:07:06Z | PHASE 1 | boost 1.55->1.74: version 1_74_0, download_path archives.boost.io/release/1.74.0/source, sha256 83bfc15..., removed stale darwin_boost_atomic patches from preprocess_cmds and deleted patch files | branch=modernize-depends
2026-05-27T11:44:55Z | PHASE 1+ | bump openssl 1.0.1k->1.0.2u (Boost 1.74 needs X509_check_ip_asc/X509_check_host added in OpenSSL 1.0.2) | branch=modernize-depends
2026-05-27T11:55:56Z | FIRE START | model=claude-opus-4-7
2026-05-27T11:59:00Z | PHASE 1+ | drop no-cms from openssl 1.0.2u config_opts (ec_ameth requires cms.h) | branch=modernize-depends
2026-05-27T12:03:34Z | PHASE 1+ | WAITING for run 26509765680 (sha 040ce1d, no-cms fix). NOTE: concurrent firing on shared checkout already applied+pushed this fix & triggered CI; deferring to avoid duplicate work | branch=modernize-depends
2026-05-27T12:24:24Z | PHASE 1+ | drop no-idea from openssl 1.0.2u config_opts (e_idea.o requires idea.h) | branch=modernize-depends
2026-05-27T12:52:00Z | PHASE 1+ | drop ALL no-X feature flags from openssl 1.0.2u
2026-05-27T19:06:19Z | FIRE START | model=claude-opus-4-7
2026-05-27T19:09:26Z | PHASE 1 SUCCESS — next phase ready | Boost 1.74 builds; configure detects boostlib>=1.20 + links System/Filesystem/Program_Options/Thread/Chrono; openssl 1.0.2u + qrencode + qt5.12 all build & cache in depends; daemon+cli link OK. Remaining CI red is PHASE 2 (Qt): src configure reports "QtCore headers missing; bitcoin-qt frontend will not be built" -> src/qt/Offerings-qt.exe absent -> workflow strip step exits 1. NOTE: main branch ALSO failing identically. | branch=modernize-depends
2026-05-27T20:07:28Z | FIRE START | model=claude-opus-4-7
2026-05-27T20:09:47Z | PHASE 2 | qt.mk 5.12.11->5.15.16: version, download_path archive/qt/5.15, suffix everywhere-opensource-src, sha256 qtbase=b048150.../qttools=1cab118.../qttranslations=415dbbb... (downloaded from official mirror, xz -t verified) | branch=modernize-depends
2026-05-27T21:06:39Z | FIRE START | model=claude-opus-4-7
2026-05-27T21:08:14Z | PHASE 2 | qt preprocess fix: qbytearraymatcher.h moved corelib/tools->corelib/text in Qt5.15 (sed couldn't find file -> .stamp_preprocessed Error 2). Verified other sed targets (moc/generator.cpp, qendian.h, qfloat16.h, qttools generator.cpp) still exist via raw.githubusercontent v5.15.16-lts-lgpl | branch=modernize-depends
2026-05-27T22:07:02Z | FIRE START | model=claude-opus-4-7
2026-05-27T22:08:14Z | PHASE 2 | qt corelib build past preprocessing+tools; fix qlocale_win.cpp std::size error (GCC13 needs <iterator>) via preprocess sed. qrencode 404 is non-fatal (bitcoincore.org mirror fallback OK). | branch=modernize-depends
2026-05-27T23:06:52Z | FIRE START | model=claude-opus-4-7
2026-05-27T23:09:15Z | PHASE 2 | qt configure failed: 'openssl-linked' precondition fails (Qt5.15 needs OpenSSL>=1.1.1, depends still on 1.0.2u from Phase1). Dropped unconditional -openssl-linked + OPENSSL_LIBS; mingw32 now uses native -schannel SSL backend (Windows is the only CI target). Defers OpenSSL bump to Phase 3. | branch=modernize-depends
2026-05-28T00:09:18Z | FIRE START | model=claude-opus-4-7
2026-05-28T00:11:19Z | PHASE 2 | qlocale_win.cpp std::size error: <iterator> include alone insufficient (std::size is C++17-guarded, qtbase compiles this TU pre-C++17). Replaced int(std::size(buf)) -> int(sizeof(buf)/sizeof(buf[0])) via preprocess sed (buf is wchar_t[255]); header/standard-independent. | branch=modernize-depends
2026-05-28T01:06:43Z | FIRE START | model=claude-opus-4-7
2026-05-28T01:07:49Z | PHASE 2 | qtbase corelib std::less<>{} (qabstractitemmodel.cpp:218) failed as 'wrong number of template args' — transparent comparator needs C++14+ but qt.mk forced -c++std c++11. Bumped to c++17 (Qt5.15 supports/requires it). qlocale std::size fix from prior firing confirmed working. | branch=modernize-depends
2026-05-28T06:30:31Z | FIRE START | model=opus-4-7
2026-05-28T06:32:17Z | PHASE 2 | qtbase fails linking xdgdesktopportal platformtheme plugin (needs libQt5DBus.a but only -dbus-runtime set). Added -no-dbus + -no-feature-xdgdesktopportal to config_opts_mingw32 (xdgdesktopportal is Linux-XDG-only; Windows target shouldn't build it). | branch=modernize-depends
2026-05-28T10:20:17Z | PHASE 2 | drop invalid -no-feature-xdgdesktopportal (Qt5.15 unknown feature name); -no-dbus alone should skip the xdgdesktopportal platformtheme | branch=modernize-depends
2026-05-28T11:39:24Z | FIX add boost::placeholders to bitcoingui+clientmodel | branch=modernize-depends
2026-05-28T12:09:11Z | FIX boost::placeholders in splashscreen.cpp | branch=modernize-depends
