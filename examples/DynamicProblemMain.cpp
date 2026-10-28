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
  parameters.IterationsCount = 10000;
  parameters.NumPoints = 16;
  parameters.r = 10;
  parameters.IsPrintResultToConsole = false;

  int iterationCount = 100;
  double initialData = 100.0;  // начальное значение data
  double range = pow(2.0, 64.0) / initialData;
  int N = 8;

  SolutionResult** results = new SolutionResult * [N * iterationCount];
  double left = 0;
  double right = 0;
  double h = range / (N * iterationCount);

  for (int iteration = 0; iteration < iterationCount; iteration++)
  {
    Solver** solvers = new Solver * [N];
    DynamicProblem** problems = new DynamicProblem * [N];


    left = h * N * iteration;
    right = h * N * (iteration + 1);

    #pragma omp parallel for num_threads((int)N)
    for (int i = 0; i < N; i++)
    {
      double a = left + h * i;
      double b = left + h * (i + 1);

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

      results[iteration * N + i] = solvers[i]->GetSolutionResult();
    }

    for (int i = 0; i < N; i++)
    {


      double a = left + h * i;
      double b = left + h * (i + 1);
      std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
      //std::cout << std::fixed << std::setprecision(0);
      std::cout << "a = " << a << " b = " << b << std::endl;
      solvers[i]->PrintResultToConsole();

      delete solvers[i];
      delete problems[i];
    }

    delete[] solvers;
    delete[] problems;
  }

  Trial* optimum = results[0]->BestTrial;
  int optI = 0;
  int optIteration = 0;

  for (int iteration = 0; iteration < iterationCount; iteration++)
  {
    for (int i = 0; i < N; i++)
    {
      int index = iteration * N + i;
      if (optimum->GetValue() > results[index]->BestTrial->GetValue())
      {
        optimum = results[index]->BestTrial;
        optI = i;
        optIteration = iteration;

      }
    }
  }

  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";

  left = h * N * optIteration;
  right = h * N * (optIteration + 1);
  double a = left + h * optI;
  double b = left + h * (optI + 1);

  DynamicProblem* problem = new DynamicProblem(
    parameters.Dimension,  // размерность
    a,                     // нижняя граница
    b,                     // верхняя граница
    initialData            // начальное значение data
  );
  problem->Initialize();

  double oldY = optimum->y[0];

  double optVal = problem->FinishCalculateFunctionals(optimum->y, 0);

  std::cout << "optimum y = " << optimum->y[0] << " optimum value = " << optimum->GetValue() << std::endl;

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}

// - end of file ----------------------------------------------------------------------------------
