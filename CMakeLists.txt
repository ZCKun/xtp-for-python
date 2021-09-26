cmake_minimum_required(VERSION 3.8)
project(py_xtp_api)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set_target_properties(PROPERTIES COMPILE_FLAGS "-save-temps")

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup -I/usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/include/python3.9 -I/usr/local/include")

#option(USE_64BITS "compile 64bits" ON)
#if (USE_64BITS)
#    add_definitions(-DUSE_64BITS)
#endif()

set(XTPAPI_PATH xtpapi)
include_directories(${XTPAPI_PATH})

set(XTPAPI_MD_LIBRARY)
find_library(XTPAPI_MD_LIBRARY
    NAMES libxtpquoteapi.dylib
    PATHS ${XTPAPI_PATH}
)

set(PYTHON_LIBRARY /usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/lib/)
set(PYTHON_INCLUDE_PATH /usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/include/python3.9/)
include_directories(${PYTHON_INCLUDE_PATH})

# add_definitions(-BUILD_XTP_MD)
set(XTP_MD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/py_xtp_quote/include/)
include_directories(${XTP_MD_PATH})
set(PY_XTP_MD_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/py_xtp_quote/src/py_xtp_quote.cc)
add_library(py_xtp_quote SHARED ${PY_XTP_MD_SOURCE} ${XTP_MD_PATH})

set_target_properties(py_xtp_quote PROPERTIES PREFIX "")
target_link_libraries(py_xtp_quote ${PYTHON_LIBRARY} ${XTPAPI_MD_LIBRARY})


