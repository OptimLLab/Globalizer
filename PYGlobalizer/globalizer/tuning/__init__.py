"""globalizer.tuning - подбор параметров методом Globalizer с интерфейсом Optuna.

Чтобы перевести код с Optuna на Globalizer, достаточно заменить импорт:

    import optuna                         # было
    import globalizer.tuning as optuna    # стало

    def loss(trial):
        a = trial.suggest_float("a", -4, 4)
        b = trial.suggest_int("b", 0, 10)
        return (a - 1.5) ** 2 + abs(b - 3)

    study = optuna.create_study()
    study.optimize(loss, n_trials=80)
    print(study.best_params, study.best_value)

Точки испытаний выбирает индексный метод глобальной оптимизации Globalizer
(сэмплер :class:`samplers.GlobalizerSampler`); пространство поиска строится
по вызовам ``trial.suggest_*`` (define-by-run), как в Optuna. Поддерживаются
study.optimize / ask / tell / enqueue_trial / add_trial, ограничения
(``trial.set_constraint`` и ``constraints_func``), прунеры (MedianPruner,
PercentilePruner, NopPruner), направления minimize / maximize.
"""

from . import distributions, exceptions, logging, pruners, samplers, study, trial
from .exceptions import TrialPruned, TuningError
from .study import (Study, StudyDirection, copy_study, create_study, delete_study,
                    get_all_study_names, get_all_study_summaries, load_study)
from .trial import FixedTrial, FrozenTrial, Trial, TrialState, create_trial

__version__ = "0.1.0"

__all__ = [
    "Study", "StudyDirection", "Trial", "FrozenTrial", "FixedTrial", "TrialState",
    "TrialPruned", "TuningError", "create_study", "create_trial", "load_study",
    "delete_study", "copy_study",
    "get_all_study_names", "get_all_study_summaries", "distributions", "exceptions",
    "logging", "pruners", "samplers", "study", "trial",
]
