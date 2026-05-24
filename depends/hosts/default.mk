# Default host settings — overridden by host-specific files.

CFLAGS := -O2 -g
CXXFLAGS := -std=c++11 -O2 -g
LDFLAGS :=

host_toolchain :=

ifeq ($(host),$(build))
  # Native build — no toolchain prefix
  CC  := gcc
  CXX := g++
  AR  := ar
  RANLIB := ranlib
  NM  := nm
  STRIP := strip
else
  # Cross-compile
  CC  := $(host)-gcc
  CXX := $(host)-g++
  AR  := $(host)-ar
  RANLIB := $(host)-ranlib
  NM  := $(host)-nm
  STRIP := $(host)-strip
  host_toolchain := $(host)-
endif

JOBS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
