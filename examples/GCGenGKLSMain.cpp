/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      GCGenMain.cpp                                               //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Lebedev I., Barkalov K.,                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "Globalizer.h"
#include "GKLS/GKLSProblem.hpp"
#include "GlobalizerProblemGCGen.h"

int main(int argc, char* argv[])
{
    GlobalizerInitialization(argc, argv, true, true);

    GKLSClass gkls = Simple;
    if (parameters.Comment.ToString() == "simple")
      gkls = Simple;
    else
      gkls = Hard;

    if (!(parameters.Dimension.GetIsChange()))
      parameters.Dimension = 2;
    int N = parameters.Dimension;
    print << N << "\n";
    TGKLSProblem* gkls_problem = new TGKLSProblem(parameters.maxCountLocalPoint, N, gkls, TD);

    // Создаем обертку для GKLS задачи под интерфейс IOptProblem
    GlobalizerOptProblem* wrapped_problem = new GlobalizerOptProblem(gkls_problem);

    wrapped_problem->Initialize();

    Solver solver(wrapped_problem);
    if (solver.Solve() != SYSTEM_OK)
        throw EXCEPTION("Error: solver.Solve crash!!!");

    draw_plot(wrapped_problem, solver.GetSolutionResult(), { 0, 1 }, L"output_lines_levels_interpolation.png", Plotter::LevelLayers, Plotter::Interpolation, true);

    delete wrapped_problem;

    MPI_Finalize();
    return 0;
}
// - end of file ----------------------------------------------------------------------------------
