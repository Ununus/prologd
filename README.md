# Пролог-Д
Исходный код компилятора "Пролог-Д"

### Инструкция по сборке на linux Debian
Версии пакетов для сборки:
- cmake 3.18.4
- gcc >= 10.2.1

```commandline
apt-get update
apt-get install cmake
echo 'deb http://deb.debian.org/debian testing main' > /etc/apt/sources.list.d/testing.list

cmake .
make         # compilation
make install # installation
```
