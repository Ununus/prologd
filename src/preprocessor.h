#pragma once
#include <string>
#include <list>
#include <sstream>
#include <vector>

// Все пути этого файла — имена файлов и каталогов — в UTF-8: так их несут текст
// программы, протокол окна и командная строка, которую разбирает prologdc.cpp.
// В системный путь (UTF-16 на Windows) их переводит `path_utf8.hpp`, и делает это
// препроцессор: строка, отданная `std::ifstream`, прочиталась бы в ANSI-кодировке
// и кириллица в имени файла превратилась бы в `?`.

// Каталог запущенного exe: рядом с движком лежат `stdlib` и `examples` поставки,
// поэтому его тоже спрашивают, где искать файл `?ИМПОРТ`.
void preprocessor_set_executable_directory(std::string directory);

// Каталоги ИМПОРТ из командной строки (`-I`, `-d`) — в том порядке, в каком их
// спрашивают после каталога импортирующего файла.
void preprocessor_set_import_directories(std::vector<std::string> directories);

// Файл, который называет строка `?ИМПОРТ("имя")` (или `:- ИМПОРТ("имя")`):
// абсолютное имя берётся как есть, иначе имя ищется от каталога импортирующего
// файла, от каталогов `-I`, от каталога exe (и его `stdlib`) и, наконец, от
// текущего каталога.  Ответ — путь в UTF-8; пустая строка, если файла нет.
std::string preprocessor_find_import(std::string name, std::string from_directory);

// Разбор файла программы: имя — путь в UTF-8.
std::stringstream& preprocessor_run(std::string);
std::stringstream& preprocessor_run_interactive(std::istream*);

// Разбор текста программы: он может быть назван файлом (`source_path`), и тогда
// от его каталога ищутся `?ИМПОРТ`; без имени файла ищут их прочие каталоги.
std::list<std::string> preprocessor_run_on_source(std::list<std::string>, std::string source_path = "");
