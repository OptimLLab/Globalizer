/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      PYGlobalizer.cpp                                            //
//                                                                         //
//  Purpose:   Source code file with implementation of Python interface    //
//  module for Globalizer using pybind11                                   //
//                                                                         //
//  Author(s): Egorov K.                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file PYGlobalizer.cpp
\authors Егоров К.С.
\date 2026
\copyright ННГУ им. Н.И. Лобачевского
\brief Реализация модуля интерфейса для Python
\details Реализация модуля интерфейса для Python с использованием pybind11
*/

#include <iostream>
#include <Python.h>
#include <pybind11\pybind11.h>

#include "Globalizer.h"
#include "PYProblem.h"

namespace py = pybind11;

/// Функция, реализующая приём задачи из Python и вызов решателя Globalizer
void solve(py::object& data) {
	/// Инициализация Globalizer
	GlobalizerInitialization(0, nullptr, false, true);
	/// Создание экземпляра класса задач, получаемых из Python
	IProblem* problem = new PYProblem(data);

	/// инициализация задачи
	problem->Initialize();
	/// Установка размерности в параметры
	parameters.Dimension = problem->GetDimension();
	/// Создание решателя
	Solver solver(problem);
	/// Запуск решателя с проверкой
	if (solver.Solve() != SYSTEM_OK)
		throw EXCEPTION("Error: solver.Solve crash!!!");
}

/// Добавление модуля
PYBIND11_MODULE(PYGlobalizer, m) {
	/// Добавление справки
	m.doc() = "Python interface for Globalizer";
	/// Добавление вызываемой функции
	m.def("solve", &solve, "Solve method");
}