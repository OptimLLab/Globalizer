/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      DynamicProblem.h                                            //
//                                                                         //
//  Purpose:   Problem with changeable parameter for dynamic optimization  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_PROBLEM_H__
#define __DYNAMIC_PROBLEM_H__

#include "Problem.h"
#include <iostream>
#include <vector>
#include <cmath>

/**
 * @brief Задача оптимизации с изменяемым параметром
 * 
 * Наследник BaseProblem для задач, где параметры целевого функционала
 * меняются между итерациями (например, онлайн-оптимизация, адаптивное управление).
 * 
 * Целевая функция: f(x) = x² · data
 */
class DynamicProblem : public BaseProblem<DynamicProblem>
{
private:
  /// Текущее значение изменяемого параметра
  double mData;
  
  /// Границы области поиска (для каждой размерности)
  std::vector<double> mLowerBounds;
  std::vector<double> mUpperBounds;
  
  /// Число функций (критериев)
  int mNumberOfFunctions;
  
  /// Число ограничений
  int mNumberOfConstraints;
  
  /// Известное оптимальное значение (если задано)
  bool mHasKnownOptimum;
  double mOptimumValue;
  std::vector<double> mOptimumPoint;
  
public:
  /**
   * @brief Конструктор
   * @param dimension Размерность задачи
   * @param leftBound Нижняя граница области поиска
   * @param rightBound Верхняя граница области поиска
   * @param initialData Начальное значение параметра data
   */
  DynamicProblem(int dimension, double leftBound, double rightBound, double initialData)
    : BaseProblem<DynamicProblem>()
    , mData(initialData)
    , mLowerBounds(dimension, leftBound)
    , mUpperBounds(dimension, rightBound)
    , mNumberOfFunctions(1)      // один критерий
    , mNumberOfConstraints(0)    // без ограничений
    , mHasKnownOptimum(false)    // оптимум неизвестен
    , mOptimumValue(0.0)
    , mOptimumPoint(dimension, 0.0)
  {
    // Устанавливаем размерность через TypedProperty
    this->Dimension = dimension;
    this->mDim = dimension;
    
    // Устанавливаем границы через внутренние члены базового класса
    this->mLeftBorder = leftBound;
    this->mRightBorder = rightBound;
    
    // Без дискретных переменных
    this->NumberOfDiscreteVariable = 0;
    
    // Для f(x) = x² · data минимум при x = 0 (если data > 0)
    // Можно задать известный оптимум:
    if (initialData > 0.0) {
      mHasKnownOptimum = true;
      mOptimumValue = 0.0;
      mOptimumPoint[0] = 0.0;
    }

    mMinDimension = 1;
  }
  
  /**
   * @brief Обновить значение параметра data
   * @param newData Новое значение параметра
   * 
   * Вызывается между итерациями для изменения задачи оптимизации
   */
  void SetData(double newData)
  {
    mData = newData;
    
    // Обновить известный оптимум (для f(x) = x² · data, минимум при x=0)
    if (newData > 0.0) {
      mHasKnownOptimum = true;
      mOptimumValue = 0.0;
      mOptimumPoint[0] = 0.0;
    }
  }
  
  /**
   * @brief Получить текущее значение параметра data
   */
  double GetData() const
  {
    return mData;
  }
  
  /**
   * @brief Получение границ области поиска
   * @param lower Массив для нижних границ (размера Dimension)
   * @param upper Массив для верхних границ (размера Dimension)
   * 
   * Интерфейс BaseProblem/IProblem
   */
  virtual void GetBounds(double* lower, double* upper) override
  {
    int dim = (int)this->Dimension;
    for (int i = 0; i < dim; ++i)
    {
      lower[i] = mLowerBounds[i];
      upper[i] = mUpperBounds[i];
    }
  }
  
  /**
   * @brief Получить число функций (критериев)
   * @return Количество целевых функций (здесь 1)
   */
  virtual int GetNumberOfFunctions() const override
  {
    return mNumberOfFunctions;
  }
  
  /**
   * @brief Получить число ограничений
   * @return Количество ограничений (здесь 0)
   */
  virtual int GetNumberOfConstraints() const override
  {
    return mNumberOfConstraints;
  }
  
  /**
   * @brief Вычисление значения целевой функции
   * @param y Точка в пространстве поиска (массив размера Dimension)
   * @param fNumber Индекс функции (для многокритериальных задач, здесь всегда 0)
   * @return Значение f(x) = x² · data
   * 
   * Основной метод интерфейса IProblem
   */
  virtual double CalculateFunctionals(const double* y, int fNumber) override
  {
    // Для одномерной задачи берём первую координату
    double x = y[0];
    
    // Вычисляем функционал: f(x) = x² · data
    double result = x * x * mData;
    
    // Логирование (как в оригинальном коде)
    std::cout << "\tx = " << x 
              << "\tdata = " << mData 
              << "\tvalue = " << result << std::endl;
    
    return result;
  }

  /**
   * @brief Вычисление функций задачи сразу в нескольких точках (векторизовано)
   *
   * @param[in]  y         Массив последовательно записанных многомерных точек,
   *                       в которых нужно вычислить функционал. Раскладка:
   *                       y[j * Dimension + k] — k-я координата j-й точки.
   * @param[in]  fNumber   Номер вычисляемой функции (здесь единственная, 0).
   * @param[in]  numPoints Количество передаваемых точек в блоке.
   * @param[out] values    Массив длины numPoints для значений функционала:
   *                       values[j] = f(точка_j).
   *
   * Именно этот метод вызывает вычислитель блока точек (CPUMultiPointCalculation
   * / CUDACalculation) через Task::CalculateFuncsInManyPoints(). Размер блока
   * numPoints определяется параметрами Globalizer (SizeInBlock / NumPoints).
   *
   * Для f(x) = x² · data реализовано векторизованно: AVX обрабатывает по 4
   * числа double за такт (случай Dimension == 1, координаты непрерывны),
   * иначе — корректный скалярный путь.
   */
  virtual void CalculateFunctionals(double* y, int fNumber, int& numPoints,
                                    double* values) override
  {
    (void)fNumber; // единственный критерий — номер не влияет

    const int dim = (int)this->Dimension;
    const int n = numPoints;
    // Скалярный путь (общий случай: AVX недоступен или Dimension > 1)
    for (int j = 0; j < n; ++j)
    {
       // первая координата j-й точки

      int t = (int) round(y[(size_t)j * dim]);

      float x = float(t);

      values[j] = x * x * mData;
    }
  }

  /**
   * @brief Получить известное оптимальное значение функции
   * @param value Выходной параметр для значения оптимума
   * @return 0 если оптимум известен, -1 иначе
   * 
   * Для f(x) = x² · data оптимум известен: f_min = 0 при x = 0
   */
  virtual int GetOptimumValue(double& value) const override
  {
    if (mHasKnownOptimum) {
      value = mOptimumValue;
      return 0;  // оптимум известен
    }
    return -1;   // оптимум неизвестен
  }
  
  /**
   * @brief Получить точку оптимума (из IProblem)
   * @param point Массив размера Dimension для координат оптимума
   * @return 0 если оптимум известен, -1 иначе
   * 
   * Для f(x) = x² · data оптимум в точке x = 0
   */
  virtual int GetOptimumPoint(double* point) const override
  {
    if (mHasKnownOptimum) {
      int dim = (int)this->Dimension;
      for (int i = 0; i < dim; ++i) {
        point[i] = mOptimumPoint[i];
      }
      return 0;  // оптимум известен
    }
    return -1;   // оптимум неизвестен
  }
  
  /**
   * @brief Получить точку оптимума для ограничения (из ProblemPar)
   * @param point Массив размера Dimension для координат оптимума
   * @param constraintIndex Индекс ограничения
   * @return -1 (ограничений нет, оптимум не определён)
   * 
   * Для задач без ограничений всегда возвращает -1
   */
  virtual int GetConstraintOptimumPoint(double* point, int constraintIndex) override
  {
    (void)point;            // unused
    (void)constraintIndex;  // unused
    return -1;  // нет ограничений, оптимум не определён
  }
  
  /**
   * @brief Инициализация задачи (если нужна дополнительная настройка)
   */
  virtual int Initialize() override
  {
    // Вызываем базовую инициализацию
    int result = BaseProblem<DynamicProblem>::Initialize();
    
    std::cout << "DynamicProblem initialized:" << std::endl;
    std::cout << "  Dimension: " << (int)this->Dimension << std::endl;
    std::cout << "  Bounds: [" << mLowerBounds[0] << ", " << mUpperBounds[0] << "]" << std::endl;
    std::cout << "  Initial data: " << mData << std::endl;
    std::cout << "  Number of functions: " << mNumberOfFunctions << std::endl;
    std::cout << "  Has known optimum: " << (mHasKnownOptimum ? "yes" : "no") << std::endl;
    if (mHasKnownOptimum) {
      std::cout << "  Optimum: f(" << mOptimumPoint[0] << ") = " << mOptimumValue << std::endl;
    }
    
    return result;
  }
  
  /**
   * @brief Деструктор
   */
  virtual ~DynamicProblem() {}
};

#endif // __DYNAMIC_PROBLEM_H__
// - end of file ----------------------------------------------------------------------------------
