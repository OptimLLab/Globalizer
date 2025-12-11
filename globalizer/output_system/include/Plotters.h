#pragma once


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include "ProblemInterface.h"
#include "SolutionResult.h"


#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#endif // _GLOBALIZER_BENCHMARKS

#ifndef WIN32
#include <unistd.h>
#endif


/**
\file Plotters.h

\authors Усова М.А.
\date 2025
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление функций, организующих работу стороннего рисовальщика

*/

#ifdef USE_PYTHON
namespace Plotter
{
  enum FigureTypes { LevelLayers, Surface, None };
  enum CalcsTypes { ObjectiveFunction, Approximation, Interpolation, ByPoints, OnlyPoints };

  /**
    Запуск отрисовки (требует обязательного совместного использования с ключом -sip)
    \param[in] problem - указатель на задачу,
    \param[in] result - указатель на полученное решение,
    \param[in] params - индексы вещественных параметров задачи, для которых следует произвести визуализацию
    (допускается 2 или 1 (TBD) параметра для визуализации)
    \param[in] output_file_name - имя файла для сохранения изображения с указанием формата,
    \param[in] figure_type - тип визуализации целевой функции
    (доступные режимы: LevelLayers, Surface)
    \param[in] calcs_type - тип вычислений значений для визуализации целевой функции
    (доступные режимы: ObjectiveFunction, Approximation, Interpolation, ByPoints, OnlyPoints)
    \param[in] show_figure - флаг о необходимости открыть полученный рисунок в интерактивном окне на экране
    (в формате булевого значения - true, false),
    \param[in] displacement_of_points - флаг о необходимости сместить точки испытаний под график
    (в формате булевого значения - true, false)
    */
  void draw_plot(IProblem* problem, SolutionResult* result, std::initializer_list<int> params,
    wchar_t* output_file_name = L"globalizer_output.png", FigureTypes figure_type = LevelLayers,
    CalcsTypes calcs_type = ObjectiveFunction, bool show_figure = false,
    bool move_points_under_graph = false);
};
#endif
