#include "HDSolver.h"
#include "TaskFactory.h"

// ------------------------------------------------------------------------------------------------
void HDSolver::SetDimentions(std::vector<int> _dimentions)
{
  if (_dimentions.size() == 0)
  {
    dimensions.resize(parameters.Dimension);
    for (int i = 0; i < dimensions.size(); i++)
      dimensions[i] = 1;
  }
  else
  {
    dimensions = _dimentions;
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::CreateStartPoint()
{
  if (parameters.startPoint.GetIsChange() == false)
  {
    double* A = new double[parameters.Dimension];
    double* B = new double[parameters.Dimension];
    problem->GetBounds(A, B);
    parameters.startPoint.SetSize(parameters.Dimension);
    for (int i = 0; i < parameters.Dimension; i++)
    {
      parameters.startPoint[i] = A[i] + (B[i] - A[i]) / 2.0;
    }
  }
  parameters.IsUseStartPoint = true;
}

// ------------------------------------------------------------------------------------------------
void HDSolver::Construct()
{
  solvers.resize(dimensions.size());
  tasks.resize(dimensions.size());
  for (int i = 0; i < dimensions.size(); i++)
  {
    solvers[i] = new Solver(problem);
    tasks[i] = nullptr;
  }
  solutionResult = nullptr;
  originalDimension = parameters.Dimension;
  CreateStartPoint();
}



// ------------------------------------------------------------------------------------------------
HDSolver::HDSolver(IProblem* _problem, std::vector<int> _dimentions)
{
  problem = _problem;
  // Настраиваем параметры
  AutoConfig();
  SetDimentions(_dimentions);
  Construct();
  finalSolver = nullptr;
  pData = nullptr;
  pTask = nullptr;
  parameters.startParameterNumber = 0;
}

// ------------------------------------------------------------------------------------------------
#ifdef _GLOBALIZER_BENCHMARKS
HDSolver::HDSolver(IGlobalOptimizationProblem* _problem, std::vector<int> _dimentions)
  : HDSolver::HDSolver(new GlobalizerBenchmarksProblem(_problem), _dimentions)
{
}
#endif

// ------------------------------------------------------------------------------------------------
HDSolver::~HDSolver()
{
  if (solutionResult == nullptr)
    delete solutionResult;
  for (int i = 0; i < solvers.size(); i++)
  {
    if (solvers[i] != nullptr)
    {
      auto solver = solvers[i];
      delete solver;
    }
    if (tasks[i] != nullptr)
    {
      auto task = tasks[i];
      delete task;
    }
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::AddPoint(Solver* solver, int curDimensions, std::vector<Trial*>& points, int startParameterNumber)
{
  std::vector<Trial*>& curPoints = solver->GetAllPoint();
  std::vector <double> curY(curDimensions);
  for (Trial* point : curPoints)
  {
    for (int k = 0; k < curDimensions; k++)
    {
      curY[k] = point->y[k];
    }
    for (int j = 0; j < originalDimension; j++)
    {
      point->y[j] = parameters.startPoint[j];
    }
    for (int j = 0; j < curDimensions; j++)
    {
      point->y[j + startParameterNumber] = curY[j];
    }

    points.push_back(point->Clone());
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::UpdateStartPoint(SolutionResult* solution, double& bestValue, int curDimensions,
  int startParameterNumber, std::vector<Trial*>& points, HDTask* curTask)
{
  if (solution->BestTrial->index == problem->GetNumberOfConstraints())
  {
#ifdef WIN32
    if (_finite(solution->BestTrial->GetValue()) != 0)
#else
    if (std::isfinite(solution->BestTrial->GetValue()) != 0)
#endif
    {
      if (bestValue > solution->BestTrial->GetValue())
      {
        bestValue = solution->BestTrial->GetValue();
        for (int j = 0; j < curDimensions; j++)
        {
          parameters.startPoint[j + startParameterNumber] = solution->BestTrial->y[j];
        }

        print << "\t Iteration " << points.size() << "\t" << "bestValue =\t" << bestValue << "\n";
        countIterationsWithoutImprovement = points.size();

      }
    }
  }

  for (int j = 0; j < curDimensions; j++)
  {
    if (j + startParameterNumber < alternativeStartingPoint.size())
      alternativeStartingPoint[j + startParameterNumber] = solution->BestTrial->y[j];
  }
}

// ------------------------------------------------------------------------------------------------
void  HDSolver::CreateData()
{
  // В случае, если не совпадают (задача пришла снаружи — берём новые), иначе всё равно
  IProblem* _problem = problem;

  /// Создание задачи (Task) // перенести в фабрику
  if (pTask == 0)
  {
    pTask = TaskFactory::CreateTask(_problem, 0);
  }
  /// Создаём данные для поисковой информации


  if (pData == 0)
  {
    pData = new SearchData(_problem->GetNumberOfFunctions());
    int qSize = GLOBALIZER_MAX((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints)
      / log(2.0) - 2)) - 1, 1023);
    pData->ResizeQueue(qSize);
  }
  else
  {
    pData->Clear();
  }

  parameters.serializer->SetSearchData(pData);
  parameters.serializer->SetTask(pTask);
}

void HDSolver::LoadPoint()
{
  std::string pointsPath = parameters.FirstPointFilePath;
  std::string pointsPathExtension = GetFileExtension(pointsPath);

  if (pointsPathExtension == "json")
  {
    CreateData();
    bool isOpen = parameters.serializer->LoadFromFile(pointsPath, fd);

    if (isOpen)
    {
        if (parameters.M_constant.GetSize() < fd.searchData.M.size())
            parameters.M_constant.SetSize(fd.searchData.M.size());

        for (int j = 0; j < fd.searchData.M.size(); j++)
        {
            parameters.M_constant[j] = fd.searchData.M[j];
        }
        parameters.startPoint.SetSize(parameters.Dimension);
        parameters.startParameterNumber = (fd.methodParams.start_parameter_number + 1) % parameters.Dimension;

        Trial* best = fd.trials[0];

        for (auto trial : fd.trials)
        {
            if (trial->index > best->index || trial->index == best->index &&
                trial->FuncValues[best->index] < best->FuncValues[best->index])
            {
                best = trial;
            }
        }

        parameters.startPoint.SetSize(parameters.Dimension);
        for (int i = 0; i < parameters.Dimension; i++)
        {
            parameters.startPoint[i] = best->y[i];
        }
    }
  }
}

// ------------------------------------------------------------------------------------------------
int HDSolver::Solve()
{
  try
  {
    auto doLV = parameters.localRefineSolution;
    auto isPrint = parameters.isPrintResultToConsole;

    parameters.M_constant.SetSize(problem->GetNumberOfFunctions());
    for (int j = 0; j < problem->GetNumberOfFunctions(); j++)
    {
      parameters.M_constant[j] = 1;
    }

    auto mnp = parameters.MaxNumOfPoints;
    int iterationCount = parameters.HDSolverIterationCount;

    alternativeStartingPoint.resize(originalDimension);
    for (int j = 0; j < originalDimension; j++)
    {
      alternativeStartingPoint[j] = parameters.startPoint[j];
    }

    std::vector<Trial*> points;
    double bestValue = MaxDouble;

    LoadPoint();

    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
      parameters.localRefineSolution = None;
      parameters.isPrintResultToConsole = false;
      int startParameterNumber = parameters.startParameterNumber;

      for (int i = startParameterNumber; i < solvers.size(); i++)
      {
        parameters.Dimension = dimensions[i];

        Solver* solver = solvers[i];
        if (tasks[i] != nullptr)
          delete tasks[i];
        tasks[i] = dynamic_cast<HDTask*>(TaskFactory::CreateTask(problem, 0));

        tasks[i]->SetStartParameterNumber(startParameterNumber);
        parameters.startParameterNumber = startParameterNumber;

        solver->Solve(tasks[i]);

        auto solution = solver->GetSolutionResult();

        AddPoint(solver, dimensions[i], points, startParameterNumber);

        UpdateStartPoint(solution, bestValue, dimensions[i], startParameterNumber, points, tasks[i]);

        startParameterNumber = startParameterNumber + parameters.Dimension;

        parameters.Dimension = originalDimension;

        Calculation::leafCalculation = 0;

        for (int j = 0; j < tasks[i]->GetNumOfFunc(); j++)
        {
          if (solver->GetData()->M[j] > parameters.M_constant[j])
          {
            parameters.M_constant[j] = solver->GetData()->M[j];
          }
        }

        if ((points.size() - countIterationsWithoutImprovement > parameters.MaxIterationsWithoutImprovement) && (parameters.stopCondition == MaxIterWithoutImprovement))
        {
          break;
        }

        if (bestValue == MaxDouble) //Если допустимая точка не найдена
        {
          for (int j = 0; j < originalDimension; j++)
          {
            parameters.startPoint[j] = alternativeStartingPoint[j];
          }
        }
      }

      if (iteration == iterationCount - 1)
      {
        parameters.localRefineSolution = doLV;
        parameters.isPrintResultToConsole = isPrint;
      }

      if ((parameters.localRefineSolution != None) || (iteration == iterationCount - 1)) //Если нужно локальное улучшение
      {
        std::string fileSerializer = parameters.fileSerializer.ToString();
        parameters.fileSerializer = "";

        if (finalSolver == nullptr)
          finalSolver = new Solver(problem);
        else
        {

          finalSolver = new Solver(problem);
        }

        finalSolver->SetPoint(points);

        finalSolver->Solve();

        if (solutionResult != nullptr)
          delete solutionResult;
        solutionResult = finalSolver->GetSolutionResult();

        AddPoint(finalSolver, originalDimension, points, 0);

        UpdateStartPoint(solutionResult, bestValue, originalDimension, startParameterNumber, points, dynamic_cast<HDTask*>(finalSolver->GetTask()));

        parameters.fileSerializer = fileSerializer;
      }


      if (bestValue == MaxDouble) //Если допустимая точка не найдена
      {
        for (int j = 0; j < originalDimension; j++)
        {
          parameters.startPoint[j] = alternativeStartingPoint[j];
        }
      }

      parameters.startParameterNumber = 0;
    }
    parameters.MaxNumOfPoints = mnp;

  }
  catch (const Exception& e)
  {
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  catch (...)
  {
    print << "\nUNKNOWN EXCEPTION !!!\n";
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    Exception e("UNKNOWN FILE", -1, "UNKNOWN FUCNTION", "UNKNOWN EXCEPTION");
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  if (parameters.GetProcRank() == 0)
  {
    if (parameters.GetProcNum() > 1) {
      int childNum = parameters.GetProcNum() - 1;
      int curr_child = 0;
      for (unsigned int i = 0; i < childNum; i++) {
        ///curr_child = parameters.parallel_tree.ProcChild[i];!!!!!
        int finish = 1;
        MPI_Send(&finish, 1, MPI_INT, curr_child, TagChildSolved, MPI_COMM_WORLD);
      }
    }
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
SolutionResult* HDSolver::GetSolutionResult()
{
  return solutionResult;
}

// ------------------------------------------------------------------------------------------------
void HDSolver::SetPoint(std::vector<Trial*>& points)
{
  finalSolver->SetPoint(points);
}

// ------------------------------------------------------------------------------------------------
std::vector<Trial*>& HDSolver::GetAllPoint()
{
  return finalSolver->GetAllPoint();
}

// ------------------------------------------------------------------------------------------------
void HDSolver::AutoConfig()
{
  parameters.TypeSolver = HDSearch;
  parameters.automaticParametersSetting = true;

  if (parameters.NumThread.GetIsChange() == false && parameters.NumPoints.GetIsChange() == false
    && parameters.TypeCalculation == OMP)
  {
    parameters.NumThread = 1;
    parameters.NumPoints = parameters.NumThread;
  }

  if (!parameters.MaxIterationsWithoutImprovement.GetIsChange())
    parameters.MaxIterationsWithoutImprovement = parameters.iterationsCount / 10;

  if (parameters.HDSolverIterationCount.GetIsChange() == false)
  {
    if (parameters.MaxNumOfPoints.GetIsChange() == true && parameters.iterationsCount.GetIsChange() == true)
    {
      parameters.HDSolverIterationCount = parameters.iterationsCount / parameters.MaxNumOfPoints / parameters.Dimension;
    }
    else if (parameters.MaxNumOfPoints.GetIsChange() == true && parameters.iterationsCount.GetIsChange() == false) // если задачно только число итераций на размерность
    {
      parameters.iterationsCount = parameters.MaxNumOfPoints * parameters.Dimension * parameters.HDSolverIterationCount;
    }
    else if (parameters.MaxNumOfPoints.GetIsChange() == false) // если задачно только общее число итераций
    {
      int trialsPerDimention = parameters.iterationsCount / parameters.Dimension;
      if (trialsPerDimention < 100)
      {
        //Если небольшой ресурс и большая размерность - запускаем координатный решатель один раз 
        parameters.HDSolverIterationCount = 1;
        parameters.MaxNumOfPoints = parameters.iterationsCount / parameters.Dimension + 3;
      }
      else
      {
        //Если большой ресурс и большая размерность - запускаем координатный решатель несколько раз 
        parameters.MaxNumOfPoints = 100;
        parameters.HDSolverIterationCount = parameters.iterationsCount / parameters.MaxNumOfPoints / parameters.Dimension;
      }
    }
  }
  else // если задачно число итераций HDSolver
  {
    if (parameters.MaxNumOfPoints.GetIsChange() == false && parameters.iterationsCount.GetIsChange() == false) // в случае значений по умолчанию
    {
      parameters.MaxNumOfPoints = parameters.iterationsCount / parameters.HDSolverIterationCount / parameters.Dimension;
    }
    else if (parameters.MaxNumOfPoints.GetIsChange() == true)
    {
      parameters.iterationsCount = parameters.MaxNumOfPoints * parameters.Dimension * parameters.HDSolverIterationCount;
    }
  }

}