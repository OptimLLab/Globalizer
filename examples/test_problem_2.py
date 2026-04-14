import importlib.util
import sys
import os
from pathlib import Path
from math import cos, pi

# Определяем путь к текущему скрипту (он в examples)
current_dir = Path(__file__).parent.absolute()

# Поднимаемся на два уровня вверх: examples -> fork -> Globalizer
# Или на один уровень, если структура другая
root_dir = current_dir.parent.parent  # из examples в Globalizer

# Или можно подняться до папки Globalizer, ища её
def find_globalizer_root(start_path):
    """Ищем корневую директорию Globalizer"""
    current = start_path
    for _ in range(5):  # поднимаемся максимум на 5 уровней
        if (current / "PYGlobalizer").exists() and (current / "_bin").exists():
            return current
        current = current.parent
    return start_path.parent.parent  # fallback

root_dir = find_globalizer_root(current_dir)
print(f"Корневая директория: {root_dir}")

# Путь к PYProblem.py (Globalizer/PYGlobalizer/PYProblem.py)
pyproblem_path = root_dir / "PYGlobalizer" / "PYProblem.py"

# Проверяем существование файла
if not pyproblem_path.exists():
    raise FileNotFoundError(f"PYProblem.py не найден по пути: {pyproblem_path}")

# Загрузка модуля PYProblem
spec = importlib.util.spec_from_file_location(
    "PYProblem", 
    str(pyproblem_path)
)

module = importlib.util.module_from_spec(spec)
sys.modules["PYProblem"] = module
spec.loader.exec_module(module)
PYProblem = module.PYProblem

# Путь к PYGlobalizer (Globalizer/_bin)
module_dir = root_dir / "_bin"

# Добавляем директорию с PYGlobalizer в sys.path
sys.path.insert(0, str(module_dir))

# Импортируем PYGlobalizer
import PYGlobalizer

print(f"PYProblem загружен из: {pyproblem_path}")
print(f"PYGlobalizer загружен из: {module_dir}")
 
"""

Создание нескольких функций для тестирования решателя

"""

mDimension = 3
 
def f1(x):
    sum = 0.0
    for j in range(mDimension):
        sum += x[j] * x[j]
    return sum - 1.2**mDimension
 
def f2(x):
    sum = 0.0
    for j in range(mDimension):
        sum += x[j] * x[j]

    sum += -(1.25**mDimension)

    return sum
 
def f3(x):
    sum = 0.0

    for j in range(mDimension):

        sum += x[j] * x[j] - 10. * cos(2.0 * pi * x[j]) + 10.0

    return sum
 
"""

def f4(x, y):

    return x + y
 
def f5(x, y):

    return x - 4*y

"""
 
test_problem = PYProblem(dimension=mDimension)
test_problem.add_function(f1)
test_problem.add_function(f2)
test_problem.add_function(f3)
# test_problem.add_function(f4)  # Закомментировано, так как f4 и f5 не определены
# test_problem.add_function(f5) 
test_problem.set_bounds(lower=[-1.8, -1.0, -0.5], upper=[2.2]*3)
PYGlobalizer.solve(test_problem)
 