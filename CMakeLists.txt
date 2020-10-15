cmake_minimum_required(VERSION 3.0.0)
project(TestNcurses VERSION 0.1.0)

#include(CTest)
#enable_testing()

file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/source/*.cc)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/libs)

add_executable(testNcurses ${SOURCE})
add_executable(testSource ${CMAKE_SOURCE_DIR}/source/testSouce.cpp)
target_link_libraries(testSource -ljson11)
target_link_libraries(testNcurses -lncurses++ -lform -lmenu -lpanel -lncurses -lutil  -ldl -ljson11 -pthread)



#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)
