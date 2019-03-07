find_path(LIBGMP_INCLUDE_DIR
    NAMES gmp.h
)

find_library(LIBGMP_LIBRARY
    NAMES gmp libgmp
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libGMP  DEFAULT_MSG
                                  LIBGMP_LIBRARY LIBGMP_INCLUDE_DIR)
mark_as_advanced(LIBGMP_INCLUDE_DIR LIBGMP_LIBRARY)

set(LIBGMP_LIBRARIES ${LIBGMP_LIBRARY} )
set(LIBGMP_INCLUDE_DIRS ${LIBGMP_INCLUDE_DIR} )
