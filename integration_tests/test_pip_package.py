"""Проверка установленного пакета globalizer (колесо из pip / cibuildwheel).

Запускается cibuildwheel после сборки каждого колеса (test-command в
pyproject.toml) и вручную после `pip install`:

    python integration_tests/test_pip_package.py

Скрипт НЕ добавляет папку PYGlobalizer в sys.path: проверяется именно
установленный пакет, а не исходники.
"""

import sys
from pathlib import Path

import numpy as np


def main():
    import globalizer
    import globalizer.optimize as gopt
    import globalizer.optuna  # noqa: F401  (проверяем, что подпакет импортируется)
    from globalizer.optimize import _native

    print("globalizer", globalizer.__version__, "from", Path(globalizer.__file__).parent)

    native, pyproblem_cls = _native.load()
    print("native module:", native.__name__, "->", getattr(native, "__file__", "?"))
    assert native.__name__ == "globalizer.PYDGlobalizer", (
        "C++-модуль загружен не из пакета: " + native.__name__)
    assert pyproblem_cls.__module__ == "globalizer._pyproblem", pyproblem_cls.__module__

    # Простая задача: минимум 0 в точке (0.3, -0.2).
    res = gopt.differential_evolution(
        lambda x: (x[0] - 0.3) ** 2 + (x[1] + 0.2) ** 2, [(-1.0, 1.0), (-1.0, 1.0)])
    print("differential_evolution:", "f* =", res.fun, "x* =", np.round(res.x, 4),
          "nfev =", res.nfev)
    assert res.fun < 1e-2, res
    assert np.allclose(res.x, [0.3, -0.2], atol=0.1), res

    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
