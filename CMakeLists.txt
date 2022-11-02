cmake_minimum_required(VERSION 3.10)

# set the project name
project(
  CurveManager
  VERSION 1.0
  LANGUAGES CXX)
  
set(CMAKE_CXX_STANDARD 20)
set(BUILD_TESTS OFF)

include(GNUInstallDirs) # despues de definir el proyecto
# target is curvemanager, and use include a src folders

file(GLOB SOURCES "src/*.cpp" "src/utils/*.cpp")

file(GLOB INCLUDES "include/*.hpp" "include/curvemanager/*.hpp")

if(MSVC)
  add_library(${PROJECT_NAME} STATIC ${INCLUDES} ${SOURCES})
else()
  add_library(${PROJECT_NAME} SHARED ${INCLUDES} ${SOURCES})
endif(MSVC)

# bc using <> instead of ""
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

include("cmake/FindQuantExt.cmake")
find_package(Boost REQUIRED)
find_package(QuantLib REQUIRED)
find_package(QuantLibParser REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)

# link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${QLE_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC QuantLib::QuantLib)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json_schema_validator)
target_link_libraries(${PROJECT_NAME} PUBLIC QuantLibParser::QuantLibParser)

target_include_directories(${PROJECT_NAME} PUBLIC ${QLE_INCLUDE_DIR})

# install paths
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake)

set(CONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
    CACHE INTERNAL "")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")

# export targets to be used with find_package
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}_Targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# exports target config file to be used with find_package
install(
  EXPORT ${PROJECT_NAME}_Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CONFIG_INSTALL_DIR})

# exports config file to be used with find_package
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CONFIG_INSTALL_DIR})

# exports headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX})

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
