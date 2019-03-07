find_path(LIBNTL_INCLUDE_DIR
    NAMES NTL/ALL_FEATURES.h
)

find_library(LIBNTL_LIBRARY
    NAMES ntl libntl
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libNTL  DEFAULT_MSG
                                  LIBNTL_LIBRARY LIBNTL_INCLUDE_DIR)
mark_as_advanced(LIBNTL_INCLUDE_DIR LIBNTL_LIBRARY)

set(LIBNTL_LIBRARIES ${LIBNTL_LIBRARY} )
set(LIBNTL_INCLUDE_DIRS ${LIBNTL_INCLUDE_DIR} )
