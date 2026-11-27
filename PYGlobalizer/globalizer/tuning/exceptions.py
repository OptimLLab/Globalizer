"""Исключения globalizer.tuning.

Имена TrialPruned, DuplicatedStudyError, ExperimentalWarning и псевдоним
OptunaError совпадают с optuna.exceptions, чтобы код, перехватывающий эти
исключения, работал без изменений.
"""

__all__ = ["TuningError", "OptunaError", "TrialPruned", "DuplicatedStudyError",
           "ExperimentalWarning"]


class TuningError(Exception):
    """Базовое исключение globalizer.tuning."""


#: Имя базового исключения в Optuna - для совместимости ``except ...OptunaError``.
OptunaError = TuningError


class TrialPruned(TuningError):
    """Поднимается в целевой функции, чтобы прервать (обрезать) испытание."""


class DuplicatedStudyError(TuningError):
    """Исследование с таким именем уже существует."""


class ExperimentalWarning(Warning):
    """Предупреждение об экспериментальной функции."""
