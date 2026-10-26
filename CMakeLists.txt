cmake_minimum_required(VERSION 3.16)

project("prologd" VERSION 1.27 LANGUAGES CXX)
set(PROLOGD_VERSION "${PROJECT_VERSION}")

set(PROLOGD_RELEASE_DATE "26.10")

# The engine and the interpreter are C++17 (std::filesystem, [[fallthrough]]).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The console interpreter is the whole of the default build.  The JSON protocol of
# a front-end (`prologd --protocol json`, src/protocol.cpp) and the test suite are
# options, and each of them drags in a third-party library - which is why neither
# is built unless it is asked for.
option(PROLOGD_BUILD_FOR_GUI "Build the JSON session a front-end speaks" OFF)
option(PROLOGD_BUILD_TESTS "Build the test suite" OFF)
option(PROLOGD_FETCH_DEPS "Fetch third-party dependencies with FetchContent" ON)

# Third-party sources live in the build tree, never in the repository: the folder
# is populated on demand and is ignored by git.  Point it somewhere else with
# -DPROLOGD_THIRDPARTY_DIR=... to share one download between checkouts.
set(PROLOGD_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/src/3rdparty"
    CACHE PATH "Directory used as FetchContent base")

if(PROLOGD_BUILD_TESTS)
  include(FetchContent)
  set(FETCHCONTENT_BASE_DIR "${PROLOGD_THIRDPARTY_DIR}")
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(Catch2)
  enable_testing()
  list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
  include(Catch)
endif()

# ---------------------------------------------------------------------------
# JSON for the front-end protocol (optional)
# ---------------------------------------------------------------------------
# Only the JSON session needs it: it carries the protocol a front-end speaks
# (src/protocol.cpp).  The interpreter itself stays dependency-free.
#
# The library is taken as its single-header release rather than cloned: one
# 900 KB file instead of a full repository checkout.  The download is pinned by
# hash, and a stale or truncated file is therefore impossible to keep.
add_library(prologd_json INTERFACE)
if(PROLOGD_BUILD_FOR_GUI)
  if(PROLOGD_FETCH_DEPS)
    set(PROLOGD_JSON_VERSION 3.11.3)
    set(PROLOGD_JSON_DIR "${PROLOGD_THIRDPARTY_DIR}/json-${PROLOGD_JSON_VERSION}")
    set(PROLOGD_JSON_HEADER "${PROLOGD_JSON_DIR}/include/nlohmann/json.hpp")
    if(NOT EXISTS "${PROLOGD_JSON_HEADER}")
      file(MAKE_DIRECTORY "${PROLOGD_JSON_DIR}/include/nlohmann")
      file(DOWNLOAD
        "https://github.com/nlohmann/json/releases/download/v${PROLOGD_JSON_VERSION}/json.hpp"
        "${PROLOGD_JSON_HEADER}"
        EXPECTED_HASH SHA256=9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6
        TLS_VERIFY ON
        STATUS prologd_json_status)
      list(GET prologd_json_status 0 prologd_json_code)
      if(NOT prologd_json_code EQUAL 0)
        list(GET prologd_json_status 1 prologd_json_message)
        file(REMOVE "${PROLOGD_JSON_HEADER}")
        message(FATAL_ERROR
          "Cannot download nlohmann/json ${PROLOGD_JSON_VERSION}: ${prologd_json_message}")
      endif()
      message(STATUS "Downloaded nlohmann/json ${PROLOGD_JSON_VERSION} header")
    endif()
    target_include_directories(prologd_json INTERFACE "${PROLOGD_JSON_DIR}/include")
  else()
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(prologd_json INTERFACE nlohmann_json::nlohmann_json)
  endif()
endif()

add_subdirectory("src")

if(PROLOGD_BUILD_TESTS)
  add_subdirectory("src/tests")
endif()
