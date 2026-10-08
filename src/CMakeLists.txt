set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

add_subdirectory("prlib")

# The console interpreter.  The JSON session of a front-end is an option: it needs
# the third-party JSON header, and a build that only runs programs should not
# carry it.
set(PROLOGD_CLI_SOURCES "prologdc.cpp" "preprocessor.cpp")
set(PROLOGD_CLI_HEADERS "preprocessor.h")

if(PROLOGD_BUILD_FOR_GUI)
  list(APPEND PROLOGD_CLI_SOURCES "protocol.cpp")
  list(APPEND PROLOGD_CLI_HEADERS "host.hpp" "protocol.hpp")
endif()

add_executable(prologd ${PROLOGD_CLI_SOURCES} ${PROLOGD_CLI_HEADERS})
target_link_libraries(prologd prlib)

if(PROLOGD_BUILD_FOR_GUI)
  target_link_libraries(prologd prologd_json)
  target_compile_definitions(prologd PRIVATE PROLOGD_FOR_GUI)
  if(MSVC)
    # protocol.cpp is UTF-8 (its messages are Russian); every other source of the
    # interpreter is CP1251, so the switch must not be global.
    set_source_files_properties("protocol.cpp" PROPERTIES COMPILE_OPTIONS "/utf-8")
  endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(prologd -static-libgcc -static-libstdc++)
endif()
