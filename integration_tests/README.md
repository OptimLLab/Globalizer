# Интеграционные тесты Globalizer — запуск всех примеров из `examples/` (v3)

Этот пакет расширяет интеграционные тесты так, чтобы **каждый пример из папки
`examples/` запускался как тест** — и C++, и Python.

Всё расширение находится в папке `integration_tests/`. В ядре Globalizer нужна
ровно одна правка (см. раздел «Правка корневого CMakeLists»).

## Изменения по версиям

- **v3** — ИСПРАВЛЕН разбор вывода C++ примеров. Раннер не распознавал формат
  `Minimum value = ...`, из-за чего `SimpleMain`, `Main`, `Main_ackley` падали
  с «не удалось извлечь найденное значение из вывода», хотя значения в выводе
  были корректные. Добавлен шаблон `Minimum value = ...`. Теперь эти три
  примера проходят (значения попадают в допуск).
- **v2** — исправлен `LNK1104: python312.lib` (транзитивная зависимость от
  Python у таргета тестов) и предупреждение `C4005` в `src/main.cpp`.
- **v1** — первая версия пакета.

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
2. Пересоберите через `BuildTestAndOpenVS.bat` (или `cmake --build ...`).
   Для одного лишь раннера пересборка не нужна — изменился только Python-скрипт.

## Запуск раннера напрямую

```bash
python integration_tests/run_all_examples.py            # все примеры
python integration_tests/run_all_examples.py --only cpp
python integration_tests/run_all_examples.py --only python
python integration_tests/run_all_examples.py --filter ackley
python integration_tests/run_all_examples.py --list
```

Коды выхода: `0` — нет FAIL; `1` — есть FAIL; `2` — ошибка конфигурации.

## Ожидаемый результат на вашей машине

По вашему последнему логу: модульные тесты — 513/513 PASS. Интеграционные
после правки v3:

- PASS: SimpleMain, ConstraintsIterationMain, NoConstraintsIterationMain,
  IterationMain, Main, Main_ackley, SampleMain, HDOptimizationMain,
  DynamicProblemMain, GCGenMain, GCGenGKLSMain (все C++);
- PASS: example_legacy, example_manual_sphere, example_constrained,
  example_ackley, example_rastrigin_int (Python);
- SKIP: Example_simple, Example_SVC_2D, Example_SVC_3D — нет внешних пакетов
  `iOptProblemSimple` / `iOptProblem` (ожидаемо; поставьте пакеты, если нужны).

Итого ожидается: 16 PASS, 0 FAIL, 3 SKIP.

## Примечание про matplotlib

В логе `SimpleMain` виден трейсбек `ModuleNotFoundError: No module named
'matplotlib'` — это **не** относится к тестам. Так Globalizer пытается
нарисовать график (параметр `IsPlot=true` внутри примера) и падает уже ПОСЛЕ
того, как решение найдено и выведено. На результат интеграционного теста это
не влияет (значение уже напечатано). Чтобы убрать сообщение — установите
`matplotlib` в окружение: `pip install matplotlib`.
