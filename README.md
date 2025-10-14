# 11. Outer join

Асинхронный TCP‑сервер `join_server` принимает текстовые команды, поддерживает две in-memory таблицы A и B и выполняет операции `INTERSECTION` и `SYMMETRIC_DIFFERENCE` в соответствии с условием задания.

## Сборка

Требуется установленный Conan и CMake 3.16+.

```bash
conan install . --output-folder=build --build=missing
cmake -S . -B build
cmake --build build
```

Для ускоренной конфигурации без тестов можно передать `-DJOIN_SERVER_BUILD_TESTS=OFF` на шаге `cmake -S . -B build`.

## Тесты

```bash
cmake --build build --target join_server_tests
ctest --test-dir build
```

## Запуск

```bash
./build/join_server <port>
```

- `port` — номер TCP‑порта, на котором сервер будет принимать соединения (можно указать `0`, чтобы выбрать порт автоматически).
- Соединение обслуживается в отдельном потоке, команды в рамках одного соединения обрабатываются последовательно.

## Протокол

Команды передаются в виде строк, завершающихся символом `\n`. Параметры в командах разделяются одним пробелом.

```
INSERT <table> <id> <name>
TRUNCATE <table>
INTERSECTION
SYMMETRIC_DIFFERENCE
```

- `<table>` — `A` или `B`.
- `<id>` — целое число, уникальное в пределах таблицы.
- `<name>` — строка без разделителей.

### Ответы

- Успешные команды завершаются строкой `OK`. Для выборок перед `OK` перечислены строки результата в формате `id,A_value,B_value`.
- При ошибке сервер отвечает строкой `ERR <описание>`.

Пример с тестовыми данными из условия:

```
INSERT A 0 lean
INSERT A 0 understand      -> ERR duplicate 0
INSERT A 1 sweater         -> OK
INSERT A 2 frank           -> OK
...
INSERT B 6 flour           -> OK
INSERT B 7 wonder          -> OK
INSERT B 8 selection       -> OK

INTERSECTION
3,violation,proposal
4,quality,example
5,precision,lake
OK

SYMMETRIC_DIFFERENCE
0,lean,
1,sweater,
2,frank,
6,,flour
7,,wonder
8,,selection
OK
```

## Реализация

- В `join_server::TablesStore` хранятся таблицы A и B (остаются отсортированными за счёт `std::map`) и предоставляются операции вставки, очистки и выборки.
- `join_server::CommandProcessor` разбирает строку команды, проверяет аргументы и вызывает соответствующие методы хранилища.
- `join_server::TcpServer` обслуживает соединения, разбивает поток байтов на строки команд, передаёт их процессору и отправляет ответы клиенту.
- Модульные тесты покрывают логику хранилища и процессора команд.
