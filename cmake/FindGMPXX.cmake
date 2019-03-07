find_path(LIBGMPXX_INCLUDE_DIR
    NAMES gmpxx.h
)

find_library(LIBGMPXX_LIBRARY
    NAMES gmpxx libgmpxx
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libGMPXX  DEFAULT_MSG
                                  LIBGMPXX_LIBRARY LIBGMPXX_INCLUDE_DIR)
mark_as_advanced(LIBGMPXX_INCLUDE_DIR LIBGMPXX_LIBRARY)

set(LIBGMPXX_LIBRARIES ${LIBGMPXX_LIBRARY} )
set(LIBGMPXX_INCLUDE_DIRS ${LIBGMPXX_INCLUDE_DIR} )
