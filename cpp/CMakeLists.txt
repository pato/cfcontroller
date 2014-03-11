cmake_minimum_required(VERSION 2.8)

#include_directories(${DIR}/headers)
#link_directories (${DIR}/src/utils)

ADD_EXECUTABLE(capture capture.cpp)
TARGET_LINK_LIBRARIES(capture opencv_core opencv_imgproc ${EXTERNAL_LIBS})
#INSTALL(TARGETS sample DESTINATION "./src")
