cmake_minimum_required(VERSION 3.10)

project(
  curvemanagertests
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(GNUInstallDirs)

file(GLOB SOURCES "*.cpp")

file(GLOB INCLUDES "*.hpp")

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(curvemanagertests ${INCLUDES} ${SOURCES})
target_link_libraries(curvemanagertests PUBLIC gtest_main)

find_package(Boost REQUIRED)
find_package(QuantLib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)

# internal libs
find_package(QuantLibParser REQUIRED)
target_link_libraries(curvemanagertests PUBLIC QuantLibParser::QuantLibParser)

#parent folder
message(STATUS "Using this folder for test building: ${CMAKE_SOURCE_DIR}")
target_include_directories(curvemanagertests PUBLIC "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src")
<<<<<<< HEAD

if(MSVC)
  target_link_libraries(curvemanagertests PUBLIC "${CMAKE_SOURCE_DIR}/build/Debug/CurveManager.lib")
else()
  target_link_libraries(curvemanagertests PUBLIC "${CMAKE_SOURCE_DIR}/build/libcurvemanager.dylib")
endif(MSVC)

=======
target_link_libraries(curvemanagertests PUBLIC "${CMAKE_SOURCE_DIR}/build/libCurveManager.dylib")
>>>>>>> cefc82b (changed jsonobject file for schema's makeObj)


# external libs
include("../cmake/FindQuantExt.cmake")
target_link_libraries(curvemanagertests PUBLIC ${QLE_LIBRARY})
target_include_directories(curvemanagertests PUBLIC ${QLE_INCLUDE_DIR})

target_link_libraries(curvemanagertests PUBLIC QuantLib::QuantLib)
target_link_libraries(curvemanagertests PUBLIC Boost::boost)
target_link_libraries(curvemanagertests PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(curvemanagertests PUBLIC nlohmann_json_schema_validator)
