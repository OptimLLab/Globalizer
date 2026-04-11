#include <iostream>
#include <Python.h>
#include <pybind11\pybind11.h>

#include "Globalizer.h"
#include "PYProblem.h"

namespace py = pybind11;

void solve(py::object& data) {
	GlobalizerInitialization(0, nullptr, false, true);
	IProblem* problem = new PYProblem(data);

	problem->Initialize();
	parameters.Dimension = problem->GetDimension();
	//problem-> переопределить get-методы
	std::cout << problem->GetDimension() << std::endl;
	Solver solver(problem);
	//std::cout << "Solver created" << std::endl;
	if (solver.Solve() != SYSTEM_OK)
		throw EXCEPTION("Error: solver.Solve crash!!!");

	//std::cout << "Solver has been executed" << std::endl;
}

PYBIND11_MODULE(PYGlobalizer, m) {
	m.doc() = "Python interface for Globalizer";
	m.def("solve", &solve, "Solve method");
}