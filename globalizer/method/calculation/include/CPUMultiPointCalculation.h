/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      CPUMultiPointCalculation.h                                  //
//                                                                         //
//  Purpose:   CPU calculator that evaluates many trial points per         //
//             iteration through the problem's vectorized multi-point      //
//             interface (SIMD / intrinsics friendly).                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
 * \file CPUMultiPointCalculation.h
 *
 * \brief Вычислитель на ЦП, выполняющий за одну итерацию метода сразу блок
 *        испытаний через интерфейс задачи CalculateFuncsInManyPoints(...).
 *
 * \details Это CPU-аналог #CUDACalculation. Вместо запуска CUDA-ядра он
 *          собирает координаты всех точек блока в один непрерывный буфер и
 *          передаёт его задаче одним вызовом. Сама векторизация (интринсики
 *          AVX/SSE) живёт внутри реализации задачи — метода
 *          Problem::CalculateFunctionals(double* y, int fNumber,
 *                                        int& numPoints, double* values).
 *
 *          Размер блока точек определяется параметрами Globalizer
 *          (SizeInBlock / NumPoints) и накоплением через SetCountCalculation.
 */

#ifndef __CPU_MULTI_POINT_CALCULATION_H__
#define __CPU_MULTI_POINT_CALCULATION_H__

#include "Calculation.h"

/**
 * \brief Вычислитель на центральном процессоре с групповой обработкой точек.
 *
 * Полностью повторяет логику блочного накопления #CUDACalculation, но считает
 * на ЦП: один вызов pTask->CalculateFuncsInManyPoints() на весь блок точек.
 */
class CPUMultiPointCalculation : public Calculation
{
protected:
  /// Непрерывный буфер координат точек блока (numPoints * Dimension)
  double* coordinates = nullptr;
  /// Текущий размер буфера coordinates (в элементах)
  int coordinatesSize = 0;

  /// Буфер значений функционалов, заполняемый задачей
  double* FuncValues = nullptr;
  /// Текущий размер буфера FuncValues (в элементах)
  int FuncValuesSize = 0;

  /**
   * \brief Выполнить вычисления для накопленного блока точек.
   * \param[in]  inputSet  Входной набор испытаний.
   * \param[out] outputSet Результаты вычислений.
   *
   * Повторяет StartCalculate() из CUDACalculation, но без GPU:
   * собираем координаты, вызываем задачу один раз на весь блок,
   * раскладываем значения обратно по объектам Trial.
   */
  void StartCalculate(InformationForCalculation& inputSet,
    TResultForCalculation& outputSet);

public:
  /**
   * \brief Конструктор.
   * \param _pTask Ссылка на объект задачи.
   */
  CPUMultiPointCalculation(Task& _pTask) ;

  /**
   * \brief Деструктор. Освобождает рабочие буферы.
   */
  virtual ~CPUMultiPointCalculation();

  /**
   * \brief Выполняет вычисления для набора испытаний на ЦП.
   * \param[in]  inputSet  Входные данные.
   * \param[out] outputSet Выходные данные.
   *
   * Логика блочного накопления полностью совпадает с CUDACalculation:
   *  - если isStartComputingAway == true, считаем пришедший блок сразу;
   *  - иначе копим точки до countCalculation == 0, затем считаем всё разом.
   */
  virtual void Calculate(InformationForCalculation& inputSet,
    TResultForCalculation& outputSet);

  /**
   * \brief Сбрасывает внутреннее состояние вычислителя.
   */
  virtual void Reset();
};

#endif // __CPU_MULTI_POINT_CALCULATION_H__
// - end of file ----------------------------------------------------------------------------------
