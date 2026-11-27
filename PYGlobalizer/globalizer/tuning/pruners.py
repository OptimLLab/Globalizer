"""Прунеры (досрочная остановка неперспективных испытаний) - как в optuna.pruners.

Прунер решает только, прерывать ли испытание по промежуточным значениям
(trial.report / trial.should_prune), и не зависит от сэмплера. Для метода
Globalizer значением прерванного испытания служит его последнее
промежуточное значение.
"""

import math

import numpy as np

__all__ = ["BasePruner", "NopPruner", "MedianPruner", "PercentilePruner"]


class BasePruner:
    """Базовый класс: prune(study, trial) -> bool."""

    def prune(self, study, trial):              # pragma: no cover - переопределяется
        raise NotImplementedError


class NopPruner(BasePruner):
    """Никогда не прерывает испытания."""

    def prune(self, study, trial):
        return False


class PercentilePruner(BasePruner):
    """Прерывает испытание, если его лучшее промежуточное значение хуже
    заданного перцентиля значений завершённых испытаний на том же шаге."""

    def __init__(self, percentile, n_startup_trials=5, n_warmup_steps=0, interval_steps=1,
                 *, n_min_trials=1):
        if not 0.0 <= percentile <= 100.0:
            raise ValueError(f"percentile must lie in [0, 100], got {percentile}.")
        if n_startup_trials < 0:
            raise ValueError(f"n_startup_trials must be >= 0, got {n_startup_trials}.")
        if n_warmup_steps < 0:
            raise ValueError(f"n_warmup_steps must be >= 0, got {n_warmup_steps}.")
        if interval_steps < 1:
            raise ValueError(f"interval_steps must be >= 1, got {interval_steps}.")
        if n_min_trials < 1:
            raise ValueError(f"n_min_trials must be >= 1, got {n_min_trials}.")
        self._percentile = float(percentile)
        self._startup = n_startup_trials           # сколько испытаний не прерывать
        self._warmup = n_warmup_steps              # первые шаги каждого испытания
        self._interval = interval_steps
        self._min_trials = n_min_trials            # сколько значений нужно на шаге

    def prune(self, study, trial):
        from .trial import TrialState
        from .study import StudyDirection

        completed = study.get_trials(deepcopy=False, states=(TrialState.COMPLETE,))
        if not completed or len(completed) < self._startup:
            return False
        step = trial.last_step
        if step is None or step < self._warmup:
            return False
        if (step - self._warmup) % self._interval != 0:
            return False

        maximize = study.direction == StudyDirection.MAXIMIZE
        values = [v for s, v in trial.intermediate_values.items() if s <= step]
        if not values or all(math.isnan(v) for v in values):
            return True
        best = np.nanmax(values) if maximize else np.nanmin(values)

        others = [t.intermediate_values[step] for t in completed if step in t.intermediate_values]
        if len(others) < self._min_trials:
            return False
        pct = 100.0 - self._percentile if maximize else self._percentile
        threshold = float(np.nanpercentile(np.asarray(others, dtype=float), pct))
        if math.isnan(threshold):
            return False
        return best < threshold if maximize else best > threshold


class MedianPruner(PercentilePruner):
    """Прерывает испытание, если оно хуже медианы завершённых (прунер Optuna по умолчанию)."""

    def __init__(self, n_startup_trials=5, n_warmup_steps=0, interval_steps=1, *, n_min_trials=1):
        super().__init__(50.0, n_startup_trials, n_warmup_steps, interval_steps,
                         n_min_trials=n_min_trials)
