#pragma once
#include <string>
#include <list>

std::stringstream& preprocessor_run(std::string);
std::stringstream& preprocessor_run_interactive(std::istream*);

std::list<std::string> preprocessor_run_on_source(std::list<std::string>);