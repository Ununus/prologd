# The test suite.  It is built only with -DPROLOGD_BUILD_TESTS=ON (the root
# CMakeLists.txt has already fetched Catch2 by then).
#
# The engine is driven the way prologdc.cpp drives it: the tests define the hooks
# the engine prints and reads through (test_host.cpp) and feed it one statement
# per line, so a test reads what a console user would see.  That is why the test
# binary links prlib directly instead of running the interpreter as a process;
# the interpreter is covered by the two command line tests at the end, and by the
# protocol tests of a front-end build.

add_executable(prlib_tests
  "test_host.cpp"
  "test_program.cpp"
  "test_preprocessor.cpp"
  "test_source_encoding.cpp"
  "test_scanner.cpp"
  "test_arith.cpp"
  "test_control.cpp"
  "test_findall.cpp"
  "test_forall.cpp"
  "test_sort.cpp"
  "test_strings.cpp"
  "test_types.cpp"
  "test_input.cpp"
  "test_graphics.cpp"
  "test_keywords.cpp"
  "test_helpers.hpp"
)

target_link_libraries(prlib_tests prlib Catch2::Catch2WithMain)

# The sources are checked for their own encoding (test_source_encoding.cpp): the
# engine is compiled without /utf-8, so it needs to know which files are CP1251.
target_compile_definitions(prlib_tests PRIVATE PROLOGD_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Разбор `?ИМПОРТ` — часть предпроцессора интерпретатора, а не движка: он
# лежит рядом с prologdc.cpp, и тесты берут тот же файл, ничем его не
# оборачивая.  Пути эти файлы переводят в системные сами (path_utf8.cpp).
target_sources(prlib_tests PRIVATE
  "${CMAKE_SOURCE_DIR}/src/preprocessor.cpp"
  "${CMAKE_SOURCE_DIR}/src/path_utf8.cpp")
target_include_directories(prlib_tests PRIVATE "${CMAKE_SOURCE_DIR}/src")

if(MSVC)
  # A test is written in UTF-8 (it carries the words of the dialect), while the
  # engine reads CP1251; test_host.cpp converts with prlib's own conversions.  The
  # headers of the engine are still CP1251, which /utf-8 cannot read - hence the
  # suppressed warning (C4828: a character that is not valid in codepage 65001).
  target_compile_options(prlib_tests PRIVATE "/utf-8" "/wd4828" "/wd4876")
endif()

catch_discover_tests(prlib_tests)

# The interpreter itself: it is a process, so it is tested as one.
add_test(NAME cli_help COMMAND prologd --help)
set_tests_properties(cli_help PROPERTIES PASS_REGULAR_EXPRESSION "--source")

add_test(NAME cli_factorial
  COMMAND prologd "--source=${CMAKE_SOURCE_DIR}/src/examples/factorial.pld")
set_tests_properties(cli_factorial PROPERTIES PASS_REGULAR_EXPRESSION "=720")

# ИМПОРТ в самом интерпретаторе: файл программы называет библиотеку рядом с
# собой; запуск идёт из `src`, поэтому виден и поиск от текущего каталога.
add_test(NAME cli_imports
  COMMAND prologd "--source=${CMAKE_SOURCE_DIR}/src/examples/import.pld"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")
set_tests_properties(cli_imports PROPERTIES PASS_REGULAR_EXPRESSION "=42")

# ДЛЯВСЕХ в примере рядом с остальными: последний вопрос выводит оценки по разу на
# каждое решение условия, и `534` — это те три вывода в порядке предложений.
add_test(NAME cli_forall
  COMMAND prologd "--source=${CMAKE_SOURCE_DIR}/src/examples/forall.pld")
set_tests_properties(cli_forall PROPERTIES PASS_REGULAR_EXPRESSION "534")

# Кириллица в пути к файлу: имя приходит в UTF-8, а узкий `argv` собрала бы CRT в
# ANSI-кодировке — интерпретатор читает командную строку сам (src/prologdc.cpp).
set(prologd_cyrillic_dir "${CMAKE_CURRENT_BINARY_DIR}/кириллица")
file(MAKE_DIRECTORY "${prologd_cyrillic_dir}")
file(WRITE "${prologd_cyrillic_dir}/программа.pld" "?СЛОЖЕНИЕ(2, 3, С).\n")
add_test(NAME cli_cyrillic_path
  COMMAND prologd "--source=${prologd_cyrillic_dir}/программа.pld")
set_tests_properties(cli_cyrillic_path PROPERTIES PASS_REGULAR_EXPRESSION "=5")

# СОРТИРОВКА в настоящем интерпретаторе: список выстраивается, повтор остаётся
# одним значением, а несравнимые элементы — ошибка с именем предиката.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sort.pld" "?СОРТИРОВКА([3,1,2,1], С).\n")
add_test(NAME cli_sort
  COMMAND prologd "--source=${CMAKE_CURRENT_BINARY_DIR}/sort.pld")
set_tests_properties(cli_sort PROPERTIES PASS_REGULAR_EXPRESSION "1,2,3")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sort_error.pld" "?СОРТИРОВКА([1, аня], С).\n")
add_test(NAME cli_sort_error
  COMMAND prologd "--source=${CMAKE_CURRENT_BINARY_DIR}/sort_error.pld")
set_tests_properties(cli_sort_error PROPERTIES
  PASS_REGULAR_EXPRESSION "СОРТИРОВКА: Недопустимый тип аргументов")
