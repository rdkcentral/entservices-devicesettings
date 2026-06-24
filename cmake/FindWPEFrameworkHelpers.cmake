# - Try to find WPEFrameworkHelpers
# Once done this will define
#  WPEFrameworkHelpers_FOUND        - System has WPEFrameworkHelpers
#  WPEFrameworkHelpers_INCLUDE_DIRS - The WPEFrameworkHelpers include directories
#
# Also creates an imported target:
#  WPEFrameworkHelpers::WPEFrameworkHelpers

find_path(WPEFrameworkHelpers_INCLUDE_DIRS
    NAMES DeviceSettingsConfig.h UtilsLogging.h
    PATH_SUFFIXES wpeframework/helpers wpeframework/helpers)

set(WPEFrameworkHelpers_INCLUDE_DIRS ${WPEFrameworkHelpers_INCLUDE_DIRS} CACHE PATH "Path to WPEFrameworkHelpers includes")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WPEFrameworkHelpers DEFAULT_MSG
    WPEFrameworkHelpers_INCLUDE_DIRS)

if(WPEFrameworkHelpers_FOUND AND NOT TARGET WPEFrameworkHelpers::WPEFrameworkHelpers)
    add_library(WPEFrameworkHelpers::WPEFrameworkHelpers INTERFACE IMPORTED)
    set_target_properties(WPEFrameworkHelpers::WPEFrameworkHelpers PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${WPEFrameworkHelpers_INCLUDE_DIRS}")
endif()

mark_as_advanced(
    WPEFrameworkHelpers_FOUND
    WPEFrameworkHelpers_INCLUDE_DIRS)