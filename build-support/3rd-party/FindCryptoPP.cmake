# Copyright (c) 2025
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions, and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions, and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of the author nor the names of its contributors may be used
#   to endorse or promote products derived from this software without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.

#[=======================================================================[.rst:
FindCryptoPP
--------------

Find Crypto++ headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``CryptoPP::CryptoPP``
  The Crypto++ library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``CryptoPP_FOUND``
  True if Crypto++ is found.
``CryptoPP_VERSION``
  The version of Crypto++.
``CryptoPP_LIBRARIES``
  The libraries to link against to use Crypto++.
``CryptoPP_INCLUDE_DIRS``
  Where to find the Crypto++ headers.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_CRYPTOPP QUIET libcryptopp)  # Corrected module name
set(CryptoPP_COMPILE_OPTIONS ${PC_CRYPTOPP_CFLAGS_OTHER})
set(CryptoPP_VERSION ${PC_CRYPTOPP_VERSION})

find_path(
  CryptoPP_INCLUDE_DIR
  NAMES cryptlib.h
  HINTS ${PC_CRYPTOPP_INCLUDEDIR} ${PC_CRYPTOPP_INCLUDE_DIRS}
  PATH_SUFFIXES cryptopp
)

find_library(
  CryptoPP_LIBRARY 
  NAMES cryptopp libcryptopp
  HINTS ${PC_CRYPTOPP_LIBDIR} ${PC_CRYPTOPP_LIBRARY_DIRS}
)

# Extract Crypto++ version from cryptlib.h
if(CryptoPP_INCLUDE_DIR AND NOT CryptoPP_VERSION)
  if(EXISTS "${CryptoPP_INCLUDE_DIR}/cryptlib.h")
    file(READ "${CryptoPP_INCLUDE_DIR}/cryptlib.h" _cryptopp_version_content)
    string(REGEX MATCH "#define CRYPTOPP_VERSION ([0-9]+)" _dummy "${_cryptopp_version_content}")
    if(CMAKE_MATCH_1)
      math(EXPR CryptoPP_VERSION_MAJOR "${CMAKE_MATCH_1} / 100")
      math(EXPR CryptoPP_VERSION_MINOR "(${CMAKE_MATCH_1} % 100) / 10")
      math(EXPR CryptoPP_VERSION_PATCH "${CMAKE_MATCH_1} % 10")
      set(CryptoPP_VERSION "${CryptoPP_VERSION_MAJOR}.${CryptoPP_VERSION_MINOR}.${CryptoPP_VERSION_PATCH}")
    endif()
  endif()
endif()

if(CryptoPP_INCLUDE_DIR AND CryptoPP_LIBRARY)
  set(CryptoPP_FOUND TRUE)
else()
  set(CryptoPP_FOUND FALSE)
endif()

if(NOT CryptoPP_FIND_QUIETLY)
  if(CryptoPP_FOUND)
    message(STATUS "Found Crypto++: ${CryptoPP_LIBRARY} (version: ${CryptoPP_VERSION})")
  else()
    message(WARNING "Crypto++ not found!")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  CryptoPP
  FOUND_VAR CryptoPP_FOUND
  REQUIRED_VARS CryptoPP_INCLUDE_DIR CryptoPP_LIBRARY
  VERSION_VAR CryptoPP_VERSION
)

if(CryptoPP_FOUND AND NOT TARGET CryptoPP::CryptoPP)
  add_library(CryptoPP::CryptoPP UNKNOWN IMPORTED GLOBAL)
  set_target_properties(
    CryptoPP::CryptoPP
    PROPERTIES IMPORTED_LOCATION "${CryptoPP_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${CryptoPP_COMPILE_OPTIONS}"
               INTERFACE_INCLUDE_DIRECTORIES "${CryptoPP_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  CryptoPP_INCLUDE_DIR CryptoPP_LIBRARY
)

if(CryptoPP_FOUND)
  set(CryptoPP_LIBRARIES ${CryptoPP_LIBRARY})
  set(CryptoPP_INCLUDE_DIRS ${CryptoPP_INCLUDE_DIR})
endif()
