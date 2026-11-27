# Globalizer

**Globalizer** is a global optimization system developed at Lobachevsky State
University of Nizhny Novgorod (UNN). This package provides its Python
interfaces, compatible with `scipy.optimize` and with Optuna.

**Globalizer** — программная система для решения задач многомерной
многоэкстремальной (глобальной) оптимизации, разработанная в ННГУ
им. Н. И. Лобачевского. Пакет содержит Python-интерфейсы Globalizer,
совместимые с `scipy.optimize` и с Optuna.

## Установка

```bash
pip install globalizer
```

Готовые колёса собраны для Windows (x64) и Linux (x86_64, manylinux),
Python 3.11–3.14. Зависимость одна — `numpy`.

## Интерфейс в стиле scipy.optimize

Достаточно заменить `scipy` на `globalizer`:

```python
import numpy as np
from globalizer.optimize import differential_evolution

def rastrigin(x):
    return 10 * len(x) + np.sum(x ** 2 - 10 * np.cos(2 * np.pi * x))

res = differential_evolution(rastrigin, [(-5.12, 5.12)] * 2)
print(res.fun, res.x)
```

Доступны `differential_evolution`, `dual_annealing`, `shgo`, `direct`,
`basinhopping`, `minimize`, а также `Bounds`, `NonlinearConstraint`,
`LinearConstraint` и `OptimizeResult` — с теми же сигнатурами, что в scipy.

## Интерфейс в стиле Optuna

```python
import globalizer.tuning as optuna

def objective(trial):
    x = trial.suggest_float("x", -10, 10)
    y = trial.suggest_float("y", -10, 10)
    return (x - 2) ** 2 + (y + 1) ** 2

study = optuna.create_study(direction="minimize")
study.optimize(objective, n_trials=100)
print(study.best_params, study.best_value)
```

## Ограничения сборки из PyPI

* Без MPI, без расширенной точности (MPFR) и без CUDA — параллельность
  только через OpenMP. Для MPI-режима соберите Globalizer из исходников.
* Размерность задачи — не более 130, число функций (критерии + ограничения) —
  не более 70.
* Встроенная отрисовка графиков (`lib/plotters`) в пакет не входит.

## Ссылки

* Исходный код: https://github.com/OptimLLab/Globalizer
* Документация: https://globalizer-documentation.readthedocs.io/en/latest/

Лицензия — BSD 3-Clause (см. файл LICENSE).

## Совместимость и товарные знаки

Globalizer — независимый проект ННГУ, не связанный с разработчиками SciPy и
Optuna и не одобренный ими. Пакет не содержит кода SciPy и Optuna и не зависит
от них: с ними совпадают только имена функций, классов и параметров, нужные
для совместимости интерфейсов. Названия SciPy и Optuna упоминаются лишь для
описания этой совместимости и принадлежат их правообладателям.

Globalizer is an independent project of UNN. It is not affiliated with or
endorsed by the SciPy or Optuna projects and contains no code from them; only
the function, class and parameter names needed for API compatibility are the
same. SciPy and Optuna are trademarks of their respective owners.
