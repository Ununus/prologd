# Prolog D
Prolog D - Пролог детский, интерпретатор языка Пролог на русском языке для обучения школьников и студетов.

## Requirements
`C++17`, `CMake 3.16`

## Build
```
cmake -S . -B build
cmake --build build
```

The default build is the console interpreter and nothing else: it has no
third-party dependencies and does not download anything.

### Options

| Option | Default | What it does |
| --- | --- | --- |
| `PROLOGD_BUILD_FOR_GUI` | `OFF` | Build the JSON session (`prologd --protocol json`) that a front-end speaks. Needs the third-party JSON header, which is downloaded at configure time. |
| `PROLOGD_BUILD_TESTS` | `OFF` | Build and enable the test suite of `src/tests`. Needs Catch2, which is fetched at configure time; run it with `ctest --test-dir build -C Release --output-on-failure`. |
| `PROLOGD_FETCH_DEPS` | `ON` | Fetch third-party dependencies at configure time. Turn it off to use the ones already on the machine (`find_package(nlohmann_json)`). |
| `PROLOGD_THIRDPARTY_DIR` | `src/3rdparty` | Where the dependencies are fetched. The directory is not a part of the repository. |

Neither library is a part of the repository: each is taken from its own source
when it is asked for. A build that asks for none of them downloads nothing.

```
cmake -S . -B build -DPROLOGD_BUILD_FOR_GUI=ON
cmake -S . -B build -DPROLOGD_BUILD_TESTS=ON
```

## Run
```
prologd --source=FILE
```

Use `prologd --help` to see all available options.

`prologd --list-keywords` prints the reserved names of the engine
(`СЛОЖЕНИЕ/3`, `ВЫВОД/+`, `ЛИНИЯ/5`, …), one per line. A front-end reads that
command — and the `keywords` field of the `ready` event — to offer hints and
colour the names the engine actually accepts.
