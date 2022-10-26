find_path(QLE_INCLUDE_DIR NAMES qle HINTS "${CMAKE_PREFIX_PATH}/QuantExt/include")

if(MSVC)
    find_library(QLE_LIBRARY NAMES "QuantExt-x64-mt.lib" HINTS "${CMAKE_PREFIX_PATH}/QuantExt/lib")
else()
    find_library(QLE_LIBRARY NAMES QuantExt HINTS "${CMAKE_PREFIX_PATH}/QuantExt")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QuantExt DEFAULT_MSG QLE_LIBRARY QLE_INCLUDE_DIR)