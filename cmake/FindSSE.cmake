# Check if SSE instructions are available on the machine where 
# the project is compiled.
#
# Inspiration: https://gist.github.com/hideo55/5642892

SET(SSE_FOUND false)
SET(SSE_VERSION "")

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
   EXEC_PROGRAM(cat ARGS "/proc/cpuinfo" OUTPUT_VARIABLE CPUINFO)

   STRING(REGEX REPLACE "^.*(sse4_1).*$" "\\1" SSE_THERE "${CPUINFO}")
   STRING(COMPARE EQUAL "sse4_1" "${SSE_THERE}" SSE41_TRUE)
   IF (SSE41_TRUE)
      SET(SSE_FOUND true CACHE BOOL "SSE4.1 available on host")
      SET(SSE_VERSION "4.1" CACHE BOOL "SSE4.1 available on host")
   ENDIF (SSE41_TRUE)

   STRING(REGEX REPLACE "^.*(sse4_2).*$" "\\1" SSE_THERE "${CPUINFO}")
   STRING(COMPARE EQUAL "sse4_2" "${SSE_THERE}" SSE42_TRUE)
   IF (SSE42_TRUE)
      SET(SSE_FOUND true CACHE BOOL "SSE4.2 available on host")
      SET(SSE_VERSION "4.2" CACHE BOOL "SSE4.2 available on host")
   ENDIF (SSE42_TRUE)

ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   EXEC_PROGRAM("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE CPUINFO)

   STRING(REGEX REPLACE "^.*(SSE4.1).*$" "\\1" SSE_THERE "${CPUINFO}")
   STRING(COMPARE EQUAL "SSE4.1" "${SSE_THERE}" SSE41_TRUE)
   IF (SSE41_TRUE)
      SET(SSE_FOUND true CACHE BOOL "SSE4.1 available on host")
      SET(SSE_VERSION "4.1" CACHE BOOL "SSE4.1 available on host")
   ENDIF (SSE41_TRUE)

   STRING(REGEX REPLACE "^.*(SSE4.2).*$" "\\1" SSE_THERE "${CPUINFO}")
   STRING(COMPARE EQUAL "SSE4.2" "${SSE_THERE}" SSE42_TRUE)
   IF (SSE42_TRUE)
      SET(SSE_FOUND true CACHE BOOL "SSE4.2 available on host")
      SET(SSE_VERSION "4.2" CACHE BOOL "SSE4.2 available on host")
   ENDIF (SSE42_TRUE)

ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

mark_as_advanced(SSE_FOUND SSE_VERSION)
