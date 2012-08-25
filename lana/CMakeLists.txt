include_directories(${LANA_SOURCE_DIR})
file(GLOB SOURCES "*.cpp")

find_package(BISON)
find_package(FLEX)

add_library(lana ${SOURCES})
    

