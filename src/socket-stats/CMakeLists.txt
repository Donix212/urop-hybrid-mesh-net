build_lib(
  LIBNAME socket-stats
  SOURCE_FILES
    helper/ss-helper.cc
    model/ss.cc
  HEADER_FILES
    helper/ss-helper.h
    model/ss.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libstats}
)
