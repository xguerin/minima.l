# Find ragel executable and provides macros to generate custom build rules

find_program(RAGEL_EXECUTABLE
  NAMES ragel
  DOC "path to the ragel executable"
  PATHS
  /usr/local
  /usr
  PATH_SUFFIXES bin
  HINTS
  $ENV{RAGEL_ROOT}
  ${RAGEL_ROOT})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Ragel DEFAULT_MSG RAGEL_EXECUTABLE)
mark_as_advanced(RAGEL_EXECUTABLE)
