# Публикация Globalizer на PyPI (`pip install globalizer`)

Здесь описано, как собрать Python-пакет `globalizer`, проверить его и
опубликовать на PyPI. Репозиторий к этому уже подготовлен (что именно
изменено, см. в разделе 10). Остаются разовая настройка аккаунтов
(разделы 1 и 4) и сама публикация.

Основной способ публикации — через GitHub Actions. Workflow
`.github/workflows/wheels.yml` сам собирает колёса для Windows и Linux и
выкладывает их на PyPI без паролей и токенов (Trusted Publishing). Ручная
загрузка через `twine` описана в разделе 7 как запасной вариант.

---

## 0. Что получает пользователь

```bash
pip install globalizer
```

```python
from globalizer.optimize import differential_evolution   # как scipy.optimize
import globalizer.optuna as optuna                       # как Optuna
```

Внутри колеса:

```
globalizer/
  __init__.py, optimize/, optuna/        <- PYGlobalizer/globalizer/ (Python-код)
  PYDGlobalizer.cp312-win_amd64.pyd      <- C++-модуль (на Linux: *.so)
  _pyproblem.py                          <- копия PYGlobalizer/PYProblem.py
```

| Параметр колеса | Значение |
|---|---|
| Платформы | Windows x64, Linux x86_64 (manylinux_2_28) |
| Python | 3.11, 3.12, 3.13, 3.14 |
| MPI / MPFR / CUDA | выключены (MPI — заглушка `lib/mpi_stub`), параллельность только OpenMP |
| `GLOBALIZER_MAX_DIMENSION` | 130 (как в `gen/Start*-no_mp-No_MPI_PYGlobalizer.bat`) |
| `GLOBALIZER_MAX_Number_Of_Function` | 70 |
| Зависимости | `numpy` |

---

## 1. Перед первой публикацией (один раз)

1. **Лицензия.** Проект распространяется по BSD 3-Clause (файл `LICENSE` в
   корне). В метаданных пакета указано `license = "BSD-3-Clause AND MIT"`:
   в модуль также компилируется библиотека `lib/nlohmann_json` под MIT, и её
   файл лицензии тоже кладётся в пакет. Строка «All Rights Reserved» в шапках
   исходников BSD-лицензии не противоречит.
2. **Имя пакета.** Откройте https://pypi.org/project/globalizer/.
   * Если открывается «404 Not Found», имя свободно.
   * Если имя занято, выберите другое (например, `globalizer-opt` или
     `unn-globalizer`) и замените `name = "globalizer"` в `pyproject.toml`.
     Импорт в коде (`import globalizer`) при этом не меняется: меняется только
     команда `pip install <имя>`. Новое имя нужно также подставить в адреса
     `https://pypi.org/p/globalizer` и `https://test.pypi.org/p/globalizer`
     в `.github/workflows/wheels.yml` и в раздел 4.
3. **Аккаунты.** Зарегистрируйтесь на https://pypi.org и на
   https://test.pypi.org. Это два независимых сайта с отдельными
   аккаунтами. Включите двухфакторную аутентификацию: без неё PyPI не
   разрешает публиковать.
4. **Права на GitHub.** Для настройки окружений (раздел 4) и создания
   Release нужны права администратора (или maintainer) репозитория
   `OptimLLab/Globalizer`.

---

## 2. Локальная сборка и проверка (Windows)

Нужны:

* Python 3.11 или новее (`python --version`);
* Visual Studio 2022 или Build Tools for Visual Studio с компонентом
  «Разработка классических приложений на C++». Он уже установлен, если
  работает `gen\StartVS-no_mp-No_MPI_PYGlobalizer.bat`.

CMake ставить не нужно: если системный CMake старше 3.31, scikit-build-core
сам скачает подходящий из PyPI.

Команды выполняются в корне репозитория (`C:\Users\Li\Documents\Project\Globalizer`).

```bat
:: 1. Отдельное окружение для упаковки
python -m venv .venv-pkg
.venv-pkg\Scripts\activate
python -m pip install --upgrade pip build twine

:: 2. Сборка sdist и колеса для текущего Python (5-10 минут)
python -m build

:: 3. Проверка метаданных (README, лицензия, классификаторы)
twine check dist\*
```

В `dist\` появятся два файла:

```
globalizer-0.2.0.tar.gz
globalizer-0.2.0-cp312-cp312-win_amd64.whl     (теги зависят от версии Python)
```

Посмотрите, что попало в колесо:

```bat
python -m zipfile -l dist\globalizer-0.2.0-cp312-cp312-win_amd64.whl
```

В списке должны быть `globalizer/PYDGlobalizer.cp312-win_amd64.pyd`,
`globalizer/_pyproblem.py`, `globalizer/optimize/...`,
`globalizer/optuna/...` и `LICENSE`. Заголовков `.h` и `Globalizer_l.lib`
там быть не должно.

Проверьте установку в чистом окружении. Запускайте не из корня
репозитория, иначе Python может взять исходники вместо установленного пакета:

```bat
deactivate
python -m venv %TEMP%\gtest
%TEMP%\gtest\Scripts\activate
pip install dist\globalizer-0.2.0-cp312-cp312-win_amd64.whl
cd %TEMP%
python C:\Users\Li\Documents\Project\Globalizer\integration_tests\test_pip_package.py
```

Ожидаемый вывод заканчивается строкой `OK`. Скрипт проверяет, что
C++-модуль загружается из пакета (`globalizer.PYDGlobalizer`), и решает
простую задачу.

> Быстрый вариант для разработки: `pip install .` в корне собирает и сразу
> ставит пакет в активное окружение.

Сборка через pip не трогает ваш рабочий `_bin` и `build_64*`: она идёт в
`build\<тег-колеса>\`. Старый способ (bat-скрипты + `PYTHONPATH`) продолжает
работать.

---

## 3. Отправка изменений в репозиторий

Сначала переместите файл workflow на место. Он лежит в `docs\wheels.yml`,
потому что папку `.github` нельзя было записать автоматически:

```bat
move docs\wheels.yml .github\workflows\wheels.yml
```

```bat
git checkout -b pypi-packaging
git status
git add pyproject.toml .gitignore README.md CMakeLists.txt ^
        cmake\globalizer_targets.cmake cmake\globalizer_dependencies.cmake ^
        PYGlobalizer\PYGlobalizer.cpp PYGlobalizer\README_PyPI.md ^
        PYGlobalizer\globalizer\README.md PYGlobalizer\globalizer\optimize\_native.py ^
        integration_tests\test_pip_package.py .github\workflows\wheels.yml ^
        docs\PUBLISHING_PYPI.md
git commit -m "Python package for PyPI: pyproject.toml, wheels CI"
git push -u origin pypi-packaging
```

Затем откройте Pull Request в `main` и влейте его.

Workflow публикации запускается только на ветке, где лежит файл
`wheels.yml`. Чтобы запускать его из вкладки Actions, удобнее, чтобы файл
был в `main`.

---

## 4. Настройка Trusted Publishing (один раз)

Trusted Publishing позволяет GitHub Actions публиковать на PyPI без пароля
и API-токена: PyPI доверяет конкретному workflow конкретного репозитория.

### 4.1. На GitHub: окружения

Repository → **Settings → Environments → New environment**:

* `pypi` — для боевой публикации. Рекомендуется включить
  **Required reviewers** и указать себя: тогда перед загрузкой на PyPI
  workflow будет ждать подтверждения.
* `testpypi` — для пробной публикации.

### 4.2. На PyPI: «pending publisher»

Проекта на PyPI ещё нет, поэтому издатель регистрируется заранее:

1. https://pypi.org → **Your account → Publishing**
   (https://pypi.org/manage/account/publishing/).
2. В блоке **Add a new pending publisher** выберите **GitHub** и заполните:

   | Поле | Значение |
   |---|---|
   | PyPI Project Name | `globalizer` |
   | Owner | `OptimLLab` |
   | Repository name | `Globalizer` |
   | Workflow name | `wheels.yml` |
   | Environment name | `pypi` |

3. **Add**.

### 4.3. На TestPyPI

То же самое на https://test.pypi.org/manage/account/publishing/, но в поле
**Environment name** укажите `testpypi`.

После первой успешной загрузки pending publisher сам превращается в
обычного издателя проекта.

---

## 5. Пробная публикация на TestPyPI

1. GitHub → **Actions → Wheels / PyPI → Run workflow**.
2. Ветка `main`, `target` = `testpypi` → **Run workflow**.
3. Workflow выполнит:
   * `build_wheels`: колёса для Windows и Linux, каждое проверяется скриптом
     `integration_tests/test_pip_package.py` (около 15–30 минут);
   * `build_sdist`: исходный архив и `twine check`;
   * `publish_testpypi`: загрузка на test.pypi.org.
4. Проверьте установку с TestPyPI на любом компьютере:

   ```bash
   python -m venv t && t\Scripts\activate        # Linux: source t/bin/activate
   pip install -i https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ globalizer
   python -c "from globalizer.optimize import differential_evolution as de; print(de(lambda x: (x[0]-0.3)**2, [(-1, 1)]).x)"
   ```

   `--extra-index-url` нужен, чтобы `numpy` скачался с обычного PyPI.

> Если нужен только набор колёс без публикации, запустите workflow с
> `target = build` и скачайте артефакты `wheels-*` и `sdist` внизу страницы
> запуска.

**Повторная загрузка той же версии невозможна** ни на PyPI, ни на TestPyPI
(«File already exists»). Для повторных проб на TestPyPI повышайте версию,
например `0.2.0.dev1` → `0.2.0.dev2` (см. раздел 6).

---

## 6. Публикация на PyPI

1. **Версия.** Единственное место, где она задаётся, —
   `PYGlobalizer/globalizer/__init__.py`:

   ```python
   __version__ = "0.2.0"
   ```

   `pyproject.toml` читает её оттуда. Для каждой публикации версия должна
   быть новой (схема `MAJOR.MINOR.PATCH`; предварительные версии —
   `0.3.0rc1`, `0.3.0.dev1`).
2. Закоммитьте изменение версии в `main`.
3. GitHub → **Releases → Draft a new release**:
   * **Choose a tag**: введите `v0.2.0` → *Create new tag on publish*;
   * Target: `main`;
   * заголовок `Globalizer 0.2.0` и описание изменений;
   * **Publish release**.
4. Запустится workflow `Wheels / PyPI`. Если для окружения `pypi` включены
   Required reviewers, откройте запуск и нажмите **Review deployments →
   Approve**.
5. Через несколько минут после окончания пакет появится на
   https://pypi.org/project/globalizer/ и станет доступен всем:

   ```bash
   pip install globalizer
   pip install --upgrade globalizer      # обновление
   ```

---

## 7. Запасной вариант: ручная загрузка через twine

Этот способ подходит, если GitHub Actions недоступен. Минус: загрузится
только колесо для вашей платформы (Windows) и sdist. Пользователи Linux
будут собирать пакет из sdist сами, и им понадобятся компилятор и OpenMP.

1. На https://pypi.org → **Account settings → API tokens → Add API token**
   (scope: *Entire account* для первой загрузки, потом можно заменить
   токеном только на проект `globalizer`). Скопируйте токен `pypi-...`.
   Для TestPyPI токен создаётся отдельно на test.pypi.org.
2. Соберите пакет (раздел 2): `python -m build`.
3. Загрузите:

   ```bat
   :: пробно
   twine upload --repository testpypi dist\*
   :: боевой
   twine upload dist\*
   ```

   На запрос `username` введите `__token__`, на запрос `password` вставьте
   токен.

Колёса под несколько версий Python на одной машине собираются так:
`py -3.11 -m build --wheel`, `py -3.12 -m build --wheel` и т. д.

---

## 8. Чек-лист выпуска новой версии

1. Изменения влиты в `main`, CI `Globalizer Test` зелёный.
2. Поднята `__version__` в `PYGlobalizer/globalizer/__init__.py`.
3. (По желанию) пробный прогон: Actions → Wheels / PyPI → `testpypi`
   с версией `X.Y.Z.devN`.
4. Release `vX.Y.Z` → Publish → Approve deployment.
5. Проверка: `pip install --upgrade globalizer` в чистом окружении.

---

## 9. Частые проблемы

| Симптом | Причина и решение |
|---|---|
| `invalid-publisher` / `Trusted publishing exchange failure` при загрузке | Поля pending publisher (раздел 4) не совпадают с реальными: проверьте Owner, Repository, имя файла `wheels.yml` и имя окружения (`pypi` / `testpypi`). |
| `File already exists` | Эта версия уже загружена. Поднимите `__version__`. |
| `The name 'globalizer' is too similar to an existing project` | Имя занято или похоже на чужое. Смените `name` в `pyproject.toml` (раздел 1). |
| `CMake 3.31 or higher is required` при локальной сборке | Обновите pip (`python -m pip install -U pip`): scikit-build-core скачает свежий CMake. |
| `Could NOT find Python (missing: Development.Module)` | Python без заголовков (урезанный дистрибутив). Поставьте Python с python.org или conda. |
| `ImportError: DLL load failed while importing PYDGlobalizer` | Не нашлась DLL (обычно `vcomp140.dll`). В колёсах из CI её добавляет `delvewheel`. При локальной сборке установите Microsoft Visual C++ Redistributable или выполните `pip install delvewheel` и `delvewheel repair dist\*.whl -w dist\fixed`. |
| `Cannot import the Globalizer extension module PYDGlobalizer` после `pip install` | Python взял исходники `PYGlobalizer/globalizer` вместо установленного пакета (запуск из корня репозитория или `PYTHONPATH` указывает на `PYGlobalizer`). Запустите из другой папки или очистите `PYTHONPATH`. |
| Размерность больше 130 или функций больше 70 | Ограничение ABI колеса. Поднимите `GLOBALIZER_MAX_DIMENSION` / `GLOBALIZER_MAX_Number_Of_Function` в `[tool.scikit-build.cmake.define]` и выпустите новую версию, либо используйте сборку из исходников. |
| Нужен macOS | Колёса для macOS сейчас не собираются: для OpenMP на macOS нужен `libomp`. Добавьте `macos-latest` в матрицу `wheels.yml` и `before-all = "brew install libomp"` в `[tool.cibuildwheel.macos]`, затем проверьте сборку. |

---

## 10. Что изменено в репозитории

| Файл | Изменение |
|---|---|
| `pyproject.toml` | Новый файл: метаданные пакета, сборка через scikit-build-core, опции CMake для колеса, настройки cibuildwheel. |
| `PYGlobalizer/README_PyPI.md` | Новый файл: описание пакета для страницы на PyPI. |
| `.github/workflows/wheels.yml` (пока лежит в `docs/wheels.yml`, см. раздел 3) | Новый файл: сборка колёс и sdist, публикация на TestPyPI и PyPI. |
| `integration_tests/test_pip_package.py` | Новый файл: проверка установленного пакета (запускается в CI после каждого колеса). |
| `docs/PUBLISHING_PYPI.md` | Эта инструкция. |
| `CMakeLists.txt` | При сборке через pip (`SKBUILD`) вывод идёт в каталог сборки, а не в `_bin`. |
| `cmake/globalizer_dependencies.cmake` | При `SKBUILD` ищется `Python Development.Module` без `NumPy`: в manylinux нет libpython, а NumPy C API не используется. |
| `cmake/globalizer_targets.cmake` | То же для модуля; при `SKBUILD` `PYDGlobalizer` и `PYProblem.py` (как `_pyproblem.py`) ставятся в пакет `globalizer`, компонент `python`. Сборка bat-скриптами не изменилась. |
| `PYGlobalizer/PYGlobalizer.cpp` | Класс `PYProblem` ищется сначала в `globalizer._pyproblem`, затем в `PYProblem`, как раньше. |
| `PYGlobalizer/globalizer/optimize/_native.py` | Сначала загружается модуль из установленного пакета, затем, как раньше, из `PYTHONPATH`, `GLOBALIZER_BIN` или `_bin`. |
| `README.md` | Добавлены значок PyPI и раздел «Python: установка через pip». |
| `PYGlobalizer/globalizer/README.md` | Раздел «Установка» дополнен вариантом `pip install`. |
| `.gitignore` | Добавлены `dist/`, `wheelhouse/`, `*.egg-info/`. |

### Как убедиться, что правки на месте

Если откатываете другие изменения (например, через `git checkout -- .`),
файлы из таблицы выше не трогайте: без них колесо соберётся без C++-модуля
или не соберётся вовсе. Быстрая проверка:

```bat
findstr /n SKBUILD CMakeLists.txt cmake\globalizer_targets.cmake cmake\globalizer_dependencies.cmake
findstr /n pyproblem_class PYGlobalizer\PYGlobalizer.cpp
findstr /n _load_installed PYGlobalizer\globalizer\optimize\_native.py
```

Каждая команда должна найти хотя бы одну строку.

### Низкоуровневый интерфейс после `pip install`

Примеры, которые работают с модулем напрямую (`import PYDGlobalizer`,
`from PYProblem import PYProblem`), рассчитаны на запуск из репозитория.
С установленным пакетом те же объекты импортируются так:

```python
from globalizer import PYDGlobalizer
from globalizer._pyproblem import PYProblem
```

### Что не входит в колесо

* `lib/plotters` (встроенная отрисовка графиков при `IsPlot = true`): C++
  ищет скрипт по относительному пути `../lib/plotters/start_cpp.py`.
* Задачи `third_party/Problems`, генератор GCGen, MPI-режим, MPFR, CUDA.
  Для них остаётся сборка из исходников (`gen/*.bat`).
