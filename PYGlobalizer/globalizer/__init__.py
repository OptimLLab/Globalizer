"""Globalizer - система глобальной оптимизации (Python-интерфейс).

Модуль ``globalizer.optimize`` повторяет интерфейс ``scipy.optimize`` для
задач глобальной оптимизации: достаточно заменить ``scipy`` на ``globalizer``.

Модуль ``globalizer.tuning`` повторяет интерфейс Optuna (study / trial /
suggest_*): достаточно заменить ``import optuna`` на
``import globalizer.tuning as optuna``.

Globalizer - независимый проект ННГУ: он не связан с разработчиками SciPy и
Optuna и не содержит их кода; совпадают только имена функций, классов и
параметров, нужные для совместимости интерфейсов. SciPy и Optuna - товарные
знаки их правообладателей.
"""

from . import optimize
from . import tuning

__all__ = ["optimize", "tuning"]
__version__ = "0.2.0"
