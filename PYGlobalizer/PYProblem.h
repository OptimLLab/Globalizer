#pragma once
#include <iostream>
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "Globalizer.h"
#include "Problem.h"

namespace py = pybind11;

class PYProblem :public Problem<PYProblem> 
{
#undef OWNER_NAME
#define OWNER_NAME PYProblem
private:
	std::vector<std::function<double(const double*)>> functionsOfProblem;
	std::vector<double> lowerBounds;
	std::vector<double> upperBounds;

	bool isSetOptimum;

	double optimumValue = 0;
	std::vector<double> optimumCoordinate;
public:
	PYProblem(py::object data);
	virtual void GetBounds(double* lower, double* upper);
	virtual double CalculateFunctionals(const double* y, int fNumber);
};