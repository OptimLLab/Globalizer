#include "PYProblem.h"

PYProblem::PYProblem(py::object data) {
	this->mOwner = this;
	this->mMinDimension = 1;
	this->mMaxDimension = 50;
	this->mNumberOfConstraints = 0;	//from python
	this->mLeftBorder = -1.0;
	this->mRightBorder = 1.0;
	this->mNumberOfCriterions = 1;
	if (py::hasattr(data, "_dimension")) {
		SetDimension(data.attr("_dimension").cast<int>());
		//std::cout << "Dimension of problem: " << mDim << std::endl;
	}
	if (py::hasattr(data, "_lower_bounds")) {
		//lower bounds here
		py::list lower_list = data.attr("_lower_bounds");
		for (auto item : lower_list) {
			lowerBounds.push_back(item.cast<double>());
		}

		/*std::cout << "Lower bounds: " << std::endl;
		for (int i = 0; i < mDim; i++) {
			std::cout << i << ": " << lowerBounds[i] << std::endl;
		}*/
	}
	if (py::hasattr(data, "_upper_bounds")) {
		//upper bounds here
		py::list upper_list = data.attr("_upper_bounds");
		for (auto item : upper_list) {
			upperBounds.push_back(item.cast<double>());
		}

		/*std::cout << "Upper bounds: " << std::endl;
		for (int i = 0; i < mDim; i++) {
			std::cout << i << ": " << upperBounds[i] << std::endl;;
		}*/
	}
		// Получаем функции - переписать!!!
	if (py::hasattr(data, "_functions")) {
		py::list functions_list = data.attr("_functions");

		for (auto item : functions_list) {
			py::function py_func = py::reinterpret_borrow<py::function>(item);

			// Лямбда для вызова Python функции с одним аргументом-списком
			functionsOfProblem.push_back(
				[py_func, mDim = this->GetDimension()](const double* x) -> double {
				py::gil_scoped_acquire gil;

				// Создаём список Python из массива double
				py::list args;
				for (int i = 0; i < mDim; ++i) {
					args.append(x[i]);
				}

				// Вызываем Python функцию с одним аргументом-списком
				return py_func(args).cast<double>();
			}
			);
		}
	}

	if (py::hasattr(data, "_isSetOptimum")) {
		isSetOptimum = data.attr("_isSetOptimum").cast<bool>();
	}
	else {
		isSetOptimum = false;
	}

	this->mNumberOfConstraints = functionsOfProblem.size() - this->mNumberOfCriterions;
	std::cout << "Number of constraints: " << this->mNumberOfConstraints << std::endl;
	std::cout << "Number of criterions: " << this->mNumberOfCriterions << std::endl;

	if (isSetOptimum && py::hasattr(data,"_optimumValue"))
	{
		optimumValue = data.attr("_optimumValue").cast<double>();
		/*if (optimumCoordinate_.size() != 0)
		{
			optimumCoordinate.resize(mDim);
			for (int i = 0; i < mDim; i++)
				optimumCoordinate[i] = optimumCoordinate_[i];
		}*/
	}

	/*double* tempU = new double[GetDimension()];
	double* tempL = new double[GetDimension()];

	GetBounds(tempL, tempU);

	std::cout << "Check upper bounds: " << std::endl;
	for (int i = 0; i < GetDimension(); i++) {
		std::cout << i << ": " << tempU[i] << std::endl;
	}

	std::cout << "Check lower bounds: " << std::endl;
	for (int i = 0; i < GetDimension(); i++) {
		std::cout << i << ": " << tempL[i] << std::endl;
	}*/
}

void PYProblem::GetBounds(double* lower, double* upper) {
	//if (this->mIsInit) - fix!!!
	/*std::cout << "Getting bounds..." << std::endl;
	std::cout << "Current dimension: " << Dimension << std::endl;*/
	for (int i = 0; i < Dimension; i++)
	{
		lower[i] = lowerBounds[i];
		upper[i] = upperBounds[i];
	}
}

double PYProblem::CalculateFunctionals(const double* y, int fNumber) {
	//std::cout << "CalculateFunctionals()PYGlobalizer - begin" << std::endl;
	if (fNumber >= functionsOfProblem.size())
		throw EXCEPTION("Error function number");

	//std::cout << "CalculateFunctionals()PYGlobalizer - begin2" << std::endl;
	double temp = 0.0;

	try {
		//std::cout << "Calling Python function..." << std::endl;
		temp = functionsOfProblem[fNumber](y);
		//std::cout << "Python function returned: " << temp << std::endl;
	}
	catch (const py::error_already_set& e) {
		std::cerr << "PYTHON ERROR: " << e.what() << std::endl;
		PyErr_Print();  // Печатает полный traceback Python
		throw;
	}
	catch (const std::exception& e) {
		std::cerr << "C++ EXCEPTION: " << e.what() << std::endl;
		throw;
	}
	catch (...) {
		std::cerr << "UNKNOWN EXCEPTION occurred while calling Python function" << std::endl;
		throw;
	}

	//std::cout << "Func number" << fNumber << std::endl;
	return temp;
}