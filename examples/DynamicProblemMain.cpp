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
  parameters.NumPoints = 16;
  //parameters.IsSerializeToDashBoard = true;

  double a = 0.0;
  double b = pow(2.0, 64.0);
  double initialData = 1.0;  // начальное значение data

  // Создаём задачу с изменяемым параметром
  DynamicProblem* problem = new DynamicProblem(
    parameters.Dimension,  // размерность
    a,                     // нижняя граница
    b,                     // верхняя граница
    initialData            // начальное значение data
  );

  // Инициализация задачи
  problem->Initialize();

  // Решатель
  Solver solver(problem);
  solver.Initialize();

  std::cout << std::endl << std::endl;

  // Пошагово решаем задачу с изменением параметра data
  bool finished = false;
  Performance Timer;
  Timer.Start();
  for (int iter = 0; iter < 1000000; ++iter)
  {
    // Обновляем параметр задачи перед очередной итерацией
    double newData = (double)(iter + 1);
    problem->SetData(newData);
    
    // Выполняем итерацию оптимизации с новым значением data
    solver.DoIteration(finished);
  }
  double duration = Timer.GetTime();
  std::cout <<"\nSolve time = " << duration << "\n";

  std::cout << std::endl << std::endl;

  // Получаем и выводим результат
  auto result = solver.GetSolutionResult();
  solver.PrintResultToConsole();

  // Очистка
  delete problem;

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}

// - end of file ----------------------------------------------------------------------------------
