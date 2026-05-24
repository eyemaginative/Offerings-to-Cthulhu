# Windows (MinGW) cross-compile host settings

CFLAGS   += -DWIN32 -D_WINDOWS -D_MT
CXXFLAGS += -DWIN32 -D_WINDOWS -D_MT -DBOOST_THREAD_USE_LIB
LDFLAGS  += -static -static-libgcc -static-libstdc++

# Static link for Windows: no runtime .dll dependency
$(package)_config_opts_mingw32 += --host=$(host)
