#include "Globalizer.h"
#include <iostream>

double CalculateFunctional(double x, double data)
{
  double result = x * x * data;
  std::cout << "\tx = " << x << "\tdata = " << data << "\tvalue = " << result << std::endl;
  return result;
}

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

  IProblem* problem = nullptr;

  parameters.Dimension = 1;
  parameters.StopCondition = IterationOnly;

  double a = 0, b = 10; // (double)(pow(2.0, 64.0));
  double data = 100.0;

  problem = new ProblemFromFunctionPointers(parameters.Dimension, // размерность задачи
    std::vector<double>(parameters.Dimension, a), // нижняя граница
    std::vector<double>(parameters.Dimension, b), //  верхняя граница
    std::vector<std::function<double(const double*)>>(1, [&](const double* y)
      {
        double result = CalculateFunctional(y[0], data);
        return result;
      }) // критерий
  );


  problem->Initialize();


  // Решатель
  Solver solver(problem);
  // Инициализируем решатель
  solver.Initialize();

  std::cout << std::endl << std::endl;

  // Пошагово решаем задачу
  bool finished = false;
  for (int iter = 0; iter < 300; ++iter)
  {
    data = (double)iter + 1;
    solver.DoIteration(finished);
  }

  std::cout << std::endl << std::endl;

  auto result = solver.GetSolutionResult();

  solver.PrintResultToConsole();

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}

// - end of file ----------------------------------------------------------------------------------