2026-05-27T02:06:28Z | FIRE START | model=claude-opus-4-7
2026-05-27T02:07:06Z | PHASE 1 | boost 1.55->1.74: version 1_74_0, download_path archives.boost.io/release/1.74.0/source, sha256 83bfc15..., removed stale darwin_boost_atomic patches from preprocess_cmds and deleted patch files | branch=modernize-depends
2026-05-27T11:44:55Z | PHASE 1+ | bump openssl 1.0.1k->1.0.2u (Boost 1.74 needs X509_check_ip_asc/X509_check_host added in OpenSSL 1.0.2) | branch=modernize-depends
2026-05-27T11:55:56Z | FIRE START | model=claude-opus-4-7
2026-05-27T11:59:00Z | PHASE 1+ | drop no-cms from openssl 1.0.2u config_opts (ec_ameth requires cms.h) | branch=modernize-depends
