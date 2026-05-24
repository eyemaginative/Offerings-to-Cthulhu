# OFF wallet — depends package list
# Included by depends/Makefile.

# Core packages always built
packages := boost openssl bdb protobuf qrencode

# Qt GUI
packages += qt

# Optional: miniupnpc
ifeq ($(NO_UPNP),)
packages += miniupnpc
endif

# Packages that require a native (build-machine) build first
# (e.g. Qt's moc, protoc)
native_packages := native_protobuf

ifneq ($(filter qt,$(packages)),)
  native_packages += native_qt
endif
