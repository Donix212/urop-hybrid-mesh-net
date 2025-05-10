build_lib(
  LIBNAME manet-core
  SOURCE_FILES model/packetbb.cc
  HEADER_FILES model/packetbb.h
  LIBRARIES_TO_LINK ${libinternet}
  TEST_SOURCES test/packetbb-test-suite.cc
)
