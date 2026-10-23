#include "Globalizer.h"
#include "DynamicProblem.h"
#include <iostream>

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

  // Параметры задачи
  parameters.Dimension = 1;
  parameters.StopCondition = IterationOnly;
  parameters.TypeCalculation = CPUMultiPoint;
  parameters.IterationsCount = 1000;
  parameters.NumPoints = 16;
  parameters.r = 10;
  parameters.IsPrintResultToConsole = false;


  int N = 8;
  SolutionResult** results = new SolutionResult * [N];
  Solver** solvers = new Solver * [N];
  DynamicProblem** problems = new DynamicProblem * [N];

//#pragma omp parallel for num_threads((int)N)
  for (int i = 0; i < N; i++)
  {
    double h = pow(2.0, 64.0) / N;

    double a = h * i;
    double b = h * (i + 1);
    double initialData = 1.0;  // начальное значение data
    // Создаём задачу с изменяемым параметром
    problems[i] = new DynamicProblem(
      parameters.Dimension,  // размерность
      a,                     // нижняя граница
      b,                     // верхняя граница
      initialData            // начальное значение data
    );

    // Инициализация задачи
    problems[i]->Initialize();

    // Решатель
    solvers[i] = new Solver(problems[i]);

    // Решаем задачу
    if (solvers[i]->Solve() != SYSTEM_OK)
      throw EXCEPTION("Error: solver.Solve crash!!!");

    results[i] = solvers[i]->GetSolutionResult();
  }

  for (int i = 0; i < N; i++)
  {
    double h = pow(2.0, 64.0) / N;

    double a = h * i;
    double b = h * (i + 1);
    std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
    std::cout << "a = " << a << " b = " << b << std::endl;
    solvers[i]->PrintResultToConsole();
  }

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}

// - end of file ----------------------------------------------------------------------------------
