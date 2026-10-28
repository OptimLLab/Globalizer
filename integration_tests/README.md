# Интеграционные тесты Globalizer — запуск всех примеров из `examples/` (v2)

Этот пакет расширяет интеграционные тесты так, чтобы **каждый пример из папки
`examples/` запускался как тест** — и C++, и Python.

Всё расширение находится в папке `integration_tests/`. В ядре Globalizer нужна
ровно одна правка (см. раздел «Правка корневого CMakeLists»).

## Изменения в v2

- **Исправлен `LNK1104: python312.lib`** при сборке `Globalizer_IntegrationTests`.
  При `GLOBALIZER_PYTHON=ON` библиотека `Globalizer_l` транзитивно требует
  `python<ver>.lib`. Теперь `CMakeLists.txt` находит Python и подключает
  `Python::Python` + каталог `libs` к таргету тестов. Сам тест Python не
  использует — зависимость чисто транзитивная.
- **Убрано предупреждение `C4005 _GLOBALIZER_BENCHMARKS`** в `src/main.cpp`:
  удалён дублирующий `#define` (макрос уже приходит из CMake) и повторные
  `#include`.

## Состав пакета

```
integration_tests/
├── CMakeLists.txt              # сборка кейсов + регистрация всех примеров в CTest
├── run_all_examples.py         # универсальный раннер (C++ и Python)
├── README.md                   # этот файл
├── references/
│   ├── examples.json           # список примеров и эталонные минимумы
│   └── tests.json              # прежний набор кейсов
└── src/
    └── main.cpp                # бинарник кейсов (чистый, без C4005)
```

## Критерий успеха

Для каждого примера в `references/examples.json`:

- если задан `known_optimum` — пример пройден, когда `|f_found - known_optimum| <= tolerance`;
- если `known_optimum = null` — достаточно, что пример отработал **без краха**.

Статусы: `PASS`, `FAIL`, `SKIP`.

## Правка корневого `CMakeLists.txt`

В блоке интеграционных тестов добавьте `enable_testing()`:

```cmake
# add integration tests
if(${GLOBALIZER_BUILD_INTEGRATION_TESTS})
  enable_testing()                                    # <-- добавить эту строку
  add_subdirectory(${PROJECT_SOURCE_DIR}/integration_tests)
endif()
```

## Установка

1. Распакуйте архив в корень репозитория, заменив папку `integration_tests/`.
2. Пересоберите через `BuildTestAndOpenVS_v2.bat` (или `cmake --build ...`).

## Запуск раннера напрямую

```bash
python integration_tests/run_all_examples.py            # все примеры
python integration_tests/run_all_examples.py --only cpp
python integration_tests/run_all_examples.py --only python
python integration_tests/run_all_examples.py --filter ackley
python integration_tests/run_all_examples.py --list
```

Коды выхода: `0` — нет FAIL; `1` — есть FAIL; `2` — ошибка конфигурации.

## Если LNK1104 всё же повторится

Редкий случай, если CMake не определил каталог Python. Тогда в блоке
исправления в `CMakeLists.txt` путь можно задать жёстко, например:

```cmake
target_link_directories(Globalizer_IntegrationTests PRIVATE
  "${CMAKE_SOURCE_DIR}/build_64/Globalizer_env/libs")
```

Либо отключите Python при сборке тестов (`-DGLOBALIZER_PYTHON=OFF`), если
Python-интерфейс не нужен — тогда транзитивной зависимости не будет.
