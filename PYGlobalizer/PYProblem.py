from typing import List, Callable, Union, Optional
import numpy as np


class PYProblem:
    """
    Инициализация объекта-задачи
    """

    def __init__(self, dimension: Optional[int] = None):
        self._functions: List[Callable] = []
        self._lower_bounds: List[float] = []
        self._upper_bounds: List[float] = []
        self._dimension: Optional[int] = dimension

    def add_function(self, func: Callable, name: Optional[str] = None) -> None:
        """
        Добавление одной функции оптимизации

        Args:
            func: функция, принимающая dimension аргументов (или один аргумент-массив)
            name: имя функции (опционально)
        """
        self._functions.append(func)

    def add_functions(self, functions: List[Callable]) -> None:
        """
        Добавление нескольких функций оптимизации
        
        Args:
            functions: список функций
        """
        for func in functions:
            self.add_function(func)

    def set_bounds(self, lower: List[float], upper: List[float]) -> None:
        """
        Установка границ для переменных
        
        Args:
            lower: список нижних границ
            upper: список верхних границ
            
        Raises:
            ValueError: если длины списков не совпадают
        """
        if len(lower) != len(upper):
            raise ValueError(f"Длины lower ({len(lower)}) и upper ({len(upper)}) не совпадают")
        
        if self._dimension is None:
            self._dimension = len(lower)
        elif self._dimension != len(lower):
            raise ValueError(f"Размерность задачи {self._dimension} не совпадает с длиной границ {len(lower)}")
            
        self._lower_bounds = lower.copy()
        self._upper_bounds = upper.copy()
        
    def set_lower_bounds(self, lower: List[float]) -> None:
        """Установка только нижних границ"""
        if self._upper_bounds and len(lower) != len(self._upper_bounds):
            raise ValueError(f"Длина lower ({len(lower)}) не совпадает с upper ({len(self._upper_bounds)})")
        self._lower_bounds = lower.copy()
        if self._dimension is None:
            self._dimension = len(lower)
            
    def set_upper_bounds(self, upper: List[float]) -> None:
        """Установка только верхних границ"""
        if self._lower_bounds and len(upper) != len(self._lower_bounds):
            raise ValueError(f"Длина upper ({len(upper)}) не совпадает с lower ({len(self._lower_bounds)})")
        self._upper_bounds = upper.copy()
        if self._dimension is None:
            self._dimension = len(upper)