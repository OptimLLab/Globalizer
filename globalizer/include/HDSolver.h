#pragma once

#include "Solver.h"
#include "HDTask.h"
#include "SearchDataSerializer.h"

/**
 Базовый класс для решателя задач большой размерности
**/
class HDSolver : public ISolver
{
protected:

  /// Решатели для оптимизации по группам параметров
  std::vector< Solver*> solvers;
  /// Решатель для объединения всех остальных Решателей
  Solver* finalSolver;
  /// Размерости групп парамеров, по умолчанию по 1
  std::vector<int> dimensions;
  /// Решение задачи оптимизации
  SolutionResult* solutionResult;
  /// Размерность исходной задачи
  int originalDimension;
  /// Задачи для оптимизации по группам параметров
  std::vector <HDTask*> tasks;

  /// Общее описание задачи
  Task* pTask;
  /// База данных(поисковая информация)
  SearchData* pData;

  /// задача оптимизации
  IProblem* problem;

  SearchDataSerializer::LoadedFileData fd;

  int countIterationsWithoutImprovement = 0;

  /// альтернативная стартовая точка на случай если не удалось улучшить решение
  std::vector<double> alternativeStartingPoint;

  /// Задачть размерности
  void SetDimentions(std::vector<int> _dimentions);

  /// Создать начальную точку решения задач
  void CreateStartPoint();

  /// заполняет основные поля класса
  void Construct();

  /// Добавляет вычесленные точки в общий массив с точками
  void AddPoint(Solver* solver, int i, std::vector<Trial*>& points, int startParameterNumber);

  /// Обновляет стартовую точку
  void UpdateStartPoint(SolutionResult* solution, double& bestValue, int curDimensions,
    int startParameterNumber, std::vector<Trial*>& points, HDTask* curTask);

  void LoadPoint();
  void CreateData();

public:
  HDSolver(IProblem* problem, std::vector<int> _dimentions = {});

#ifdef _GLOBALIZER_BENCHMARKS

  HDSolver(IGlobalOptimizationProblem* problem, std::vector<int> _dimentions = {});

#endif

  virtual ~HDSolver();

  /// Решение задачи по умолчанию
  virtual int Solve();



  SolutionResult* GetSolutionResult();

  /// Добавляет точки испытаний
  virtual void SetPoint(std::vector<Trial*>& points);
  /// Возврящает все имеющиеся точки испытаний
  virtual std::vector<Trial*>& GetAllPoint();
};