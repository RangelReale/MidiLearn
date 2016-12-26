FIND_PATH(TSE3_ROOT_DIR NAMES src/tse3/Transport.h HINTS $ENV{TSE3_ROOT})

SET(TSE3_FOUND 0)

IF(TSE3_ROOT_DIR)
  IF(EXISTS "${TSE3_ROOT_DIR}/src/tse3/Transport.h")
        SET(TSE3_INCLUDE_DIR "${TSE3_ROOT_DIR}/src")
        SET(TSE3_FOUND 1)
  ELSE()
      MESSAGE(FATAL_ERROR "TSE3 was not found.")
  ENDIF()
  
  IF(TSE3_FOUND)
        FIND_LIBRARY(TSE3_LIBRARY_DEBUG NAMES tse3 HINTS ${TSE3_ROOT_DIR}/build/lib ${TSE3_ROOT_DIR}/build/lib/Debug)
        FIND_LIBRARY(TSE3_LIBRARY_RELEASE NAMES tse3 HINTS ${TSE3_ROOT_DIR}/build/lib ${TSE3_ROOT_DIR}/build/lib/Release ${TSE3_ROOT_DIR}/build/lib/RelWithDebInfo)
        
        SET(TSE3_LIBRARY
          debug ${TSE3_LIBRARY_DEBUG}
          optimized ${TSE3_LIBRARY_RELEASE}
        )        
        
        if (WIN32)
            LIST(APPEND TSE3_LIBRARY Winmm)
        endif()
        if ("${CMAKE_SYSTEM}" MATCHES "Linux")
            LIST(APPEND TSE3_LIBRARY alsa)
        endif()
  ENDIF()
ELSE()
MESSAGE(FATAL_ERROR "TSE3 was not found.")
ENDIF()
