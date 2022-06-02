#define _CRT_SECURE_NO_WARNINGS

#include "prlib/scaner.h"
#include "prlib/pstructs.h"
#include "prlib/control.h"
#include "prlib/functions.h"

#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string.h>

constexpr char input_signal_char = '$';

static const char* source_filename{ nullptr };
static const char* input_filename{ nullptr };
static const char* output_filename{ nullptr };
static const char* error_filename{ nullptr };

static std::unique_ptr<std::ifstream> source_file;
static std::unique_ptr<std::ifstream> input_file;
static std::unique_ptr<std::ofstream> output_file;
static std::unique_ptr<std::ofstream> error_file;

static std::istream* source_stream{ nullptr };
static std::istream* input_stream{ nullptr };
static std::ostream* output_stream{ nullptr };
static std::ostream* error_stream{ nullptr };

static bool out_questions{ false };
static bool dialog{ false };
static int parsed_str_number{ 0 };

static std::list<std::string> inputed_strs;

void print_cp1251_decoded_to_utf8(const char* str, std::ostream& out) {
    for (const char* c = str; *c; ++c) {
        if (*c >= 'А' && *c <= 'я') {
            auto v = static_cast<unsigned char>(*c - 'А') + 1040u;
            unsigned char c1 = 192u + (v >> 6u);
            unsigned char c2 = (v & 63u) + 128u;
            out << c1 << c2;
        }
        else {
            out << static_cast<unsigned char>(*c & 127u);
        }
    }
    out << '\n';
}

void decode_utf8_to_cp1251(std::string& str) {
    if (str.empty()) return;
    size_t si = 0;
    for (size_t i = 0; i < str.size();) {
        if ((str[i] & 128u) == 0) {
            str[si++] = str[i++];
        }
        else if ((str[i] & 224u) == 192u && i + 1 != str.size()) {
            str[si++] = ((str[i] ^ 192u) << 6u) + (str[i + 1] & 63u) - 1040u - 64u;
            i += 2;
        }
        else {
            errout("Decoding error");
            throw 1;
        }
    }
    str.resize(si);
}

void out(const char* str)
{
    //*output_stream << str << '\n';
    print_cp1251_decoded_to_utf8(str, *output_stream);
}

void errout(const char* str)
{
    //*error_stream << parsed_str_number << ' ' << str << std::endl;
    *error_stream << parsed_str_number << ' ';
    print_cp1251_decoded_to_utf8(str, *error_stream);
}

int runProlog()
{
    int serr = 0, cerr = 0;
    std::unique_ptr<array> heap;
    std::unique_ptr<TScVar> ScVar;
    std::unique_ptr<TClVar> ClVar;
    try
    { // инициализация переменных, необходимых для работы интерпретатора
        heap = std::make_unique<array>(_maxarray_);
        ScVar = std::make_unique<TScVar>();
        ClVar = std::make_unique<TClVar>();
        ClVar->PrSetting = std::make_unique<TPrSetting>();
    }
    catch (const std::bad_alloc& er) {
        errout(er.what());
        return 1;
    }
    catch (const std::runtime_error& er) {
        errout(er.what());
        return 2;
    }
    catch (...) {
        errout("Prolog initialization failure");
        return 3;
    }
    bool EnableRunning = true;
    serr = buildin(ScVar.get(), heap.get());
    parsed_str_number = 0;
    while (!serr && !cerr && EnableRunning)
    { // трансляция построчно 
        std::string line;
        if (!std::getline(*source_stream, line))
        {
            break;
        }
        decode_utf8_to_cp1251(line);
        if (line[0] == input_signal_char)
        {
            inputed_strs.push_back(line);
            continue;
        }
        ++parsed_str_number;
        if (line.empty())
        {
            continue;
        }
        char* cur_str = const_cast<char*>(line.c_str());
        try
        {
            serr = scaner(cur_str, ScVar.get(), heap.get());

            if (!serr && ScVar->Query && ScVar->EndOfClause) //если конец предложения и вопрос то
            {
                if (out_questions)
                {
                    out(cur_str);
                }
                cerr = control(ScVar.get(), ClVar.get(), heap.get(), &EnableRunning);
                ScVar->Query = ScVar->EndOfClause = false;     //на выполнение
            }
        }
        catch (...) {
            errout("Runtime error");
            return 1;
        }
    }
    return 0;
}

int InputStringFromDialog(char* buf, size_t size, char* caption)
{
    std::string line;
    if (!inputed_strs.empty())
    {
        line.swap(*inputed_strs.begin());
        inputed_strs.pop_front();
    }
    else {
        if (dialog)
        {
            *output_stream << "!dialog ";
            print_cp1251_decoded_to_utf8(caption, *output_stream);
            output_stream->flush();
        }
        if (!std::getline(*input_stream, line))
        {
            //errout("Expected input, but got end of stream");
            errout("Недостаточно входных данных");
            return 1;
        }
    }
    if (line.empty() || size == 0)
    {
        return 1;
    }
    if (line[0] == input_signal_char)
    {
        line.erase(line.begin());
    }
    if (out_questions)
    {
        out(caption);
    }
    size_t to = line.size() < size - 1 ? line.size() : size - 1;
    for (size_t i = 0; i < to; ++i)
    {
        buf[i] = line[i];
    }
    buf[to] = 0;
    return 0;
}

unsigned int GetPixel(int, int)
{
    return 0;
}

void ClearView(unsigned int)
{

}

void Ellipse(int, int, int, int, unsigned int)
{

}

void FloodFill(int, int, unsigned int)
{

}

void HideGWindow(void)
{

}

void horisontal(int, int, int, int)
{

}

void MoveTo_LineTo(int, int, int, int, unsigned int)
{

}

void Rectangle(int, int, int, int, unsigned int)
{

}

void SetPixel(int, int, unsigned int)
{

}

void ShowGWindow(bool)
{

}

void vertical(int, int, int, int)
{

}

void printHelp()
{
    std::cout << "-s, --source=FILE\tinterpret a file.\n";
    std::cout << "-i, --input=FILE \tuse a file to input.\n";
    std::cout << "-o, --output=FILE\tuse a file to print the output.\n";
    std::cout << "-e, --error=FILE \tuse a file to print an error.\n";
    std::cout << "-q, --qustions   \tprint questions.\n";
    std::cout << "-d, --dialog     \tuse the input dialog.\n";
    std::cout << "-h, --help       \tshow this help and exit.\n";
    std::cout << "\n";
    std::cout << "If files are not specified, then standard streams will be used.\n";
    std::cout << "\n";
    std::cout << "If in the source there is a string starting with the $ symbol,\n"
        "  then it will be used for input.\n";
    std::cout << "If there are not enough such string for input, the program\n"
        "  will read strings from the input stream.\n";
    std::cout << "\n";
    std::cout << "If the dialog option is set, then before input from the input stream the program\n"
        "  prints a message starting with the '!dialog' and a caption after it.\n"
        "  Example: !dialog Input yes or no\n";
    std::cout << '\n';
    std::cout << "If the qustions option is set, then the program prints\n"
        "  questions that were in the source code.\n";
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printHelp();
            return 0;
        }
        if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--qustions") == 0)
        {
            out_questions = true;
            continue;
        }
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dialog") == 0)
        {
            dialog = true;
            continue;
        }
        char* eqptr = strchr(argv[i], '=');
        if (!eqptr || !*(eqptr + 1))
        {
            std::cerr << "Invalid argument " << argv[i] << std::endl;
            return 1;
        }
        if (strncmp(argv[i], "-s", sizeof("-s") - 1) == 0 ||
            strncmp(argv[i], "--source", sizeof("--source") - 1) == 0)
        {
            source_filename = eqptr + 1;
            continue;
        }
        if (strncmp(argv[i], "-i", sizeof("-i") - 1) == 0 ||
            strncmp(argv[i], "--input", sizeof("--input") - 1) == 0)
        {
            input_filename = eqptr + 1;
            continue;
        }
        if (strncmp(argv[i], "-o", sizeof("-o") - 1) == 0 ||
            strncmp(argv[i], "--output", sizeof("--output") - 1) == 0)
        {
            output_filename = eqptr + 1;
            continue;
        }
        if (strncmp(argv[i], "-e", sizeof("-e") - 1) == 0 ||
            strncmp(argv[i], "--error", sizeof("--error") - 1) == 0)
        {
            error_filename = eqptr + 1;
            continue;
        }
        std::cerr << "Invalid argument " << argv[i] << std::endl;
        return 1;
    }
    if (source_filename)
    {
        source_file = std::make_unique<std::ifstream>(source_filename);
        if (!source_file->is_open())
        {
            std::cerr << "Cannot open file " << source_filename << std::endl;
            return 1;
        }
        source_stream = source_file.get();
    }
    else
    {
        source_stream = &std::cin;
    }
    if (input_filename)
    {
        input_file = std::make_unique<std::ifstream>(input_filename);
        if (!input_file->is_open())
        {
            std::cerr << "Cannot open file " << input_filename << std::endl;
            return 1;
        }
        input_stream = input_file.get();
    }
    else
    {
        input_stream = &std::cin;
    }
    if (output_filename)
    {
        output_file = std::make_unique<std::ofstream>(output_filename);
        if (!output_file->is_open())
        {
            std::cerr << "Cannot open file " << output_filename << std::endl;
            return 1;
        }
        output_stream = output_file.get();
    }
    else
    {
        output_stream = &std::cout;
    }
    if (error_filename)
    {
        error_file = std::make_unique<std::ofstream>(error_filename);
        if (!error_file->is_open())
        {
            std::cerr << "Cannot open file " << error_filename << std::endl;
            return 1;
        }
        error_stream = error_file.get();
    }
    else
    {
        error_stream = &std::cerr;
    }

    return runProlog();
}
