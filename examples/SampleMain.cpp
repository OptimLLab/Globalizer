/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      SampleMain.cpp                                              //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Lebedev I.                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv, true, true);

#ifdef _GLOBALIZER_BENCHMARKS
  GlobalOptimizationProblemManager manager;
  IGlobalOptimizationProblem* problem = 0;
  if (InitProblemGlobalizerBenchmarks(manager, problem))
  {
    print << "Error during problem initialization\n";
    return 0;
  }

  // Решатель
  Solver solver(problem);
  // Решаем задачу
  if (solver.Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

#ifdef USE_PYTHON
  // Пример параметров командной строки для запуска: -lib rastrigin.dll -N 3 -sip my_points.txt
  draw_plot(solver.GetProblem(), solver.GetSolutionResult(), { 0, 1 }, L"output_lines_levels_objective_dll.png", Plotter::LevelLayers, Plotter::ObjectiveFunction, true);
#endif

#endif
  MPI_Finalize();
  return 0;
}
// - end of file ----------------------------------------------------------------------------------
