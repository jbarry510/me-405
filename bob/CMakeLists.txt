project(bob)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(bob_SRCS bob.cpp main.cpp)
qt4_automoc(${bob_SRCS})
add_executable(bob ${bob_SRCS})
target_link_libraries(bob ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS bob RUNTIME DESTINATION bin)
