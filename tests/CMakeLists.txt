include_directories(${LANA_SOURCE_DIR})
link_directories(${LANA_BINARY_DIR}/lib)

find_package(CppUnit REQUIRED)

file(GLOB SOURCES "*.cpp")

add_executable(test ${SOURCES})
target_link_libraries(test lana ${CPPUNIT_LIBRARY})

add_custom_command(TARGET test
    POST_BUILD
    COMMAND test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
