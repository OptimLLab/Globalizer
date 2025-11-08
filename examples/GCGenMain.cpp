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

    TGKLSProblem* gkls_problem = new TGKLSProblem(1, 2, Simple, TD);

    // Создаем обертку для GKLS задачи под интерфейс IOptProblem
    GlobalizerOptProblem* wrapped_problem = new GlobalizerOptProblem(gkls_problem);

    wrapped_problem->Initialize();
    parameters.Dimension = 2;
    Solver solver(wrapped_problem);
    if (solver.Solve() != SYSTEM_OK)
        throw EXCEPTION("Error: solver.Solve crash!!!");

    delete wrapped_problem;

    MPI_Finalize();
    return 0;
}
// - end of file ----------------------------------------------------------------------------------
