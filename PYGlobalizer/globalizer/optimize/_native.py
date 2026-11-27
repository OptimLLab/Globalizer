"""Загрузка C++-модуля PYDGlobalizer и вспомогательного класса PYProblem.

Пакет, установленный через pip (pip install globalizer):
  модуль лежит внутри пакета - globalizer/PYDGlobalizer*.pyd / *.so,
  класс PYProblem - в globalizer/_pyproblem.py.

Запуск из исходников репозитория (PYGlobalizer добавлен в sys.path),
порядок поиска модуля PYDGlobalizer*.pyd / *.so:
  1) уже доступен через sys.path / PYTHONPATH;
  2) каталог из переменной окружения GLOBALIZER_BIN;
  3) каталог _bin репозитория Globalizer (рядом с папкой PYGlobalizer).
PYProblem.py берётся из папки PYGlobalizer (родительской для пакета).
"""

import importlib
import importlib.util
import os
import sys
from pathlib import Path

_PACKAGE = __name__.split(".")[0]                                  # "globalizer"
_HELPERS_DIR = Path(__file__).resolve().parent.parent.parent      # .../PYGlobalizer
_DEFAULT_BIN_DIR = _HELPERS_DIR.parent / "_bin"                    # .../_bin

_native = None
_pyproblem_cls = None


def _candidate_dirs():
    dirs = []
    env = os.environ.get("GLOBALIZER_BIN")
    if env:
        dirs.append(Path(env))
    dirs.append(_DEFAULT_BIN_DIR)
    return dirs


def _load_installed():
    """Модуль из установленного колеса или None, если пакет взят из исходников."""
    if importlib.util.find_spec(_PACKAGE + ".PYDGlobalizer") is None:
        return None
    # Модуль в пакете есть: ошибки загрузки (например, не найдена DLL)
    # не прячем за поиском в других местах.
    native = importlib.import_module(_PACKAGE + ".PYDGlobalizer")
    pyproblem = importlib.import_module(_PACKAGE + "._pyproblem")
    return native, pyproblem.PYProblem


def _load_from_sources():
    """Модуль, собранный bat-скриптами gen/ (каталог _bin или GLOBALIZER_BIN)."""
    # PYProblem импортируется C++-модулем по имени "PYProblem".
    if str(_HELPERS_DIR) not in sys.path:
        sys.path.append(str(_HELPERS_DIR))

    try:
        native = importlib.import_module("PYDGlobalizer")
    except ImportError:
        native = None
        for d in _candidate_dirs():
            if d.is_dir() and str(d) not in sys.path:
                sys.path.insert(0, str(d))
                try:
                    native = importlib.import_module("PYDGlobalizer")
                    break
                except ImportError:
                    sys.path.remove(str(d))
        if native is None:
            raise ImportError(
                "Cannot import the Globalizer extension module PYDGlobalizer. "
                "Install the package (pip install globalizer) or build the module "
                "(gen/StartGCC-no_mp-No_MPI_PYGlobalizer.bat or "
                "gen/StartVS-no_mp-No_MPI_PYGlobalizer.bat) and put its folder "
                "into PYTHONPATH or the GLOBALIZER_BIN environment variable.") from None

    pyproblem = importlib.import_module("PYProblem")
    return native, pyproblem.PYProblem


def load():
    """Возвращает (модуль PYDGlobalizer, класс PYProblem)."""
    global _native, _pyproblem_cls
    if _native is not None:
        return _native, _pyproblem_cls

    loaded = _load_installed()
    if loaded is None:
        loaded = _load_from_sources()
    _native, _pyproblem_cls = loaded
    return _native, _pyproblem_cls
