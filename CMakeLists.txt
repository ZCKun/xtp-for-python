cmake_minimum_required(VERSION 3.8)
project(py_xtp_api)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set_target_properties(PROPERTIES COMPILE_FLAGS "-save-temps")

option(PYTHON_LIBRARY_PATH "The python library path." "")
option(PYTHON_INCLUDE_PATH "The python include files path." "")
option(LINUX "" ON)
option(OSX "" ON)
option(WIN "" ON)

#find_package(pybind11 CONFIG REQUIRED)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS 
    # -I/usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/include/python3.9 
    "${CMAKE_CXX_FLAGS} -O3 -Wall -shared -std=c++11")

set(XTPAPI_PATH xtpapi)
include_directories(${XTPAPI_PATH})

set(XTP_LIB_PATH)
if (LINUX)
    set(XTP_LIB_PATH "${XTPAPI_PATH}/linux")
elseif (OSX)
    set(XTP_LIB_PATH "${XTPAPI_PATH}/osx")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -undefined dynamic_lookup")
else()
    set(XTP_LIB_PATH "${XTPAPI_PATH}/win")
endif()

set(XTPAPI_MD_LIBRARY)
find_library(XTPAPI_MD_LIBRARY
    NAMES xtpquoteapi
    PATHS ${XTP_LIB_PATH}
)
find_package(pybind11 CONFIG REQUIRED)

#set(PYTHON_LIBRARY /usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/lib/)
#set(PYTHON_INCLUDE_PATH /usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/include/python3.9/)
include_directories(${PYTHON_INCLUDE_PATH})

# add_definitions(-BUILD_XTP_MD)
set(XTP_MD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/py_xtp_quote/include/)
include_directories(${XTP_MD_PATH})
set(PY_XTP_MD_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/py_xtp_quote/src/py_xtp_quote.cc)
add_library(py_xtp_quote SHARED ${PY_XTP_MD_SOURCE} ${XTP_MD_PATH})

set_target_properties(py_xtp_quote PROPERTIES PREFIX "")
target_link_libraries(py_xtp_quote ${PYTHON_LIBRARY_PATH} ${XTPAPI_MD_LIBRARY} pybind11::lto pybind11::embed pybind11::module)

#target_link_libraries(py_xtp_api PRIVATE pybind11::lto pybind11::embed pybind11::module)
