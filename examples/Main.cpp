/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HDOptimizationMain.cpp                                      //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Lebedev I., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);
  IProblem* problem = nullptr;
#ifdef _GLOBALIZER_BENCHMARKS
  GlobalOptimizationProblemManager manager;
  IGlobalOptimizationProblem* globalizerBenchmarksProblem = 0;
  if (InitProblemGlobalizerBenchmarks(manager, globalizerBenchmarksProblem))
  {
    print << "Error during problem initialization\n";
    return 0;
  }
  problem = new GlobalizerBenchmarksProblem(globalizerBenchmarksProblem);
#else
  problem = new ProblemFromFunctionPointers(parameters.Dimension, // размерность задачи
    std::vector<double>(parameters.Dimension, -2.2), // нижняя граница
    std::vector<double>(parameters.Dimension, 1.8), //  верхняя граница
    std::vector<std::function<double(const double*)>>(1, [](const double* y)
      {
        double pi_ = 3.14159265358979323846;
        double sum = 0.;
        for (int j = 0; j < parameters.Dimension; j++)
          sum += y[j] * y[j] - 10. * cos(2.0 * pi_ * y[j]) + 10.0;
        return sum;
      }), // критерий
    true, // определен ли оптимум
    0, // значение глобального оптимума
    std::vector<double>(parameters.Dimension, 0) // координаты глобального минимума
  );
  problem->Initialize();
#endif

  SolutionResult* result = GlobalizerSolveProblem(problem);




  MPI_Finalize();
  return 0;
}
// - end of file ----------------------------------------------------------------------------------
