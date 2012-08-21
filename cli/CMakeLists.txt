include_directories(${LANA_SOURCE_DIR})
link_directories(${LANA_BINARY_DIR}/lib)


find_package(Readline REQUIRED)

set(SOURCES main.cpp)

add_executable(lanacli ${SOURCES})
target_link_libraries(lanacli lana)

