/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Lebedev.i                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __SEARCH_DATA_SERIALIZER_H__
#define __SEARCH_DATA_SERIALIZER_H__

#include "SearchData.h"
#include "Trial.h"
#include "SearchInterval.h"
#include "SearcDataIterator.h"
#include "Parameters.h"
#include "TrialFactory.h"
#include "SearchIntervalFactory.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <stack>
#include <cmath>
#include <cctype>
#include <map>
#include <algorithm>
#include "Task.h"

// ------------------------------------------------------------------------------------------------
/**
 * \brief Класс для сериализации поисковых данных в JSON формат
 */
class SearchDataSerializer
{
protected:
  /// Поисковая информация
  SearchData* pSearchData;

  /// текущая задача
  Task* pTask;

  /// Рабочее имя файла
  std::string currentFileName;
  /// Произведено ли начальное сохранение
  bool isFirstSave;

  /// Экранирование специальных символов в строке для JSON
  static std::string EscapeJsonString(const std::string& str);

  /// Форматирование числа с плавающей точкой для JSON
  static std::string FormatDouble(double value);

  /// Получение текущего времени в ISO формате
  static std::string GetCurrentTimeISO();

  /// Преобразование int в строку
  static std::string IntToString(int value);

  /// Преобразование строки в double
  static double StringToDouble(const std::string& str);

  /// Преобразование строки в int
  static int StringToInt(const std::string& str);

  /// Удаление пробелов в начале и конце строки
  static std::string Trim(const std::string& str);

  /// Разделение строки по разделителю
  static std::vector<std::string> Split(const std::string& str, char delimiter);

  /// Сериализация Trial в JSON строку
  std::string TrialToJson(Trial* trial);

  /// Сериализация SearchInterval в JSON строку
  std::string IntervalToJson(SearchInterval* interval);

  /// Создание Trial из JSON данных
  Trial* CreateTrialFromJSON(const std::map<std::string, std::string>& data);

  /// Создание SearchInterval из JSON данных
  SearchInterval* CreateIntervalFromJSON(const std::map<std::string, std::string>& data,
    Trial* leftPoint, Trial* rightPoint);

  /// Сохранение полного состояния в файл
  bool SaveFullState(const std::string& filename);

  /// Добавление новых точек в существующий файл
  bool AppendNewPoints(const std::vector<Trial*>& newTrials,
    const std::vector<SearchInterval*>& newIntervals,
    Trial* newBestTrial);

  // Подфункции для AppendNewPoints
  /// Чтение содержимого файла
  bool ReadFileContent(const std::string& filename, std::string& content);

  /// Запись содержимого в файл
  bool WriteFileContent(const std::string& filename, const std::string& content);

  /// Поиск позиции массива trials и его закрывающей скобки
  bool FindTrialsPosition(const std::string& content, size_t& trialsPos, size_t& trialsEnd);

  /// Проверка, есть ли уже точки в массиве trials
  bool HasExistingPoints(const std::string& content, size_t trialsPos, size_t trialsEnd);

  /// Формирование строки с новыми точками для вставки
  std::string BuildNewTrialsString(const std::vector<Trial*>& newTrials, bool hasExistingPoints);

  /// Поиск закрывающей скобки массива trials (упрощенный)
  bool FindTrialsEnd(const std::string& content, size_t trialsPos, size_t& trialsEnd);

  /// Добавление новых точек в массив trials
  bool AppendNewTrials(std::string& content, const std::vector<Trial*>& newTrials);

  /// Поиск позиции поля Count
  bool FindCountPosition(const std::string& content, size_t& countPos, size_t& countEnd);

  /// Обновление поля Count
  bool UpdateCount(std::string& content, int newCount);

  /// Поиск позиции поля best_trial (улучшенная версия)
  bool FindBestTrialPosition(const std::string& content, size_t& bestTrialPos,
    size_t& bestTrialEnd, bool& hasComma);

  /// Обновление поля best_trial (двухэтапный подход)
  bool UpdateBestTrial(std::string& content, Trial* newBestTrial);

  /// Поиск позиции временной метки
  bool FindTimestampPosition(const std::string& content, size_t& timestampStart,
    size_t& timestampEnd);

  /// Обновление временной метки (альтернативный вариант)
  bool UpdateTimestamp(std::string& content);

  /// Поиск позиции массива M
  bool FindMPosition(const std::string& content, size_t& mPos, size_t& mEnd);

  /// Обновление массива M
  bool UpdateMArray(std::string& content, const double* M, int numOfFuncs);

  /// Поиск позиции массива Z
  bool FindZPosition(const std::string& content, size_t& zPos, size_t& zEnd);

  /// Обновление массива Z
  bool UpdateZArray(std::string& content, const double* Z, int numOfFuncs);

  /// Поиск позиции поля local_r
  bool FindLocalRPosition(const std::string& content, size_t& localRPos, size_t& localREnd);

  /// Обновление поля local_r
  bool UpdateLocalR(std::string& content, double local_r);

  // Копии предыдущих значений для отслеживания изменений
  std::vector<double> previousM;
  std::vector<double> previousZ;
  double previousLocalR;

  // Флаги изменений
  bool mArrayChanged;
  bool zArrayChanged;
  bool localRChanged;

  // Методы для проверки изменений
  /// Обновление флага изменений массива M
  void UpdateMChanges();
  /// Обновление флага изменений массива Z
  void UpdateZChanges();
  /// Обновление флага изменений local_r
  void UpdateLocalRChanges();
  /// Сброс всех флагов изменений
  void ResetChangeFlags();



  // Внутренние структуры для хранения загруженных данных
  struct LoadedMethodParameters {
    double eps = 0.0;
    double r = 0.0;
    int iters_limit = 0;
    int number_of_parallel_points = 0;
    std::vector<double> start_point;
  };

  struct LoadedSearchData {
    int NumOfFuncs = 0;
    int Count = 0;
    std::vector<double> M;
    std::vector<double> Z;
    double local_r = 0.0;
  };

  // Методы для загрузки
  /// Считываем версию
  bool LoadVersion(const std::map<std::string, std::string>& root, std::string& version);
  /// Считываем тип сохранения
  bool LoadMode(const std::map<std::string, std::string>& root, std::string& mode);
  /// Считываем сохраненные параметры
  bool LoadMethodParameters(const std::map<std::string, std::string>& root, LoadedMethodParameters& params);
  /// Считываем общую информацию
  bool LoadSearchData(const std::map<std::string, std::string>& root, LoadedSearchData& searchData);
  /// Считываем точки испытаний 
  bool LoadTrials(const std::map<std::string, std::string>& root, std::vector<Trial*>& trials, std::map<double, Trial*>& trialMap);
  /// Считываем лучшую точку
  bool LoadBestTrial(const std::map<std::string, std::string>& root, const std::map<double, Trial*>& trialMap, Trial*& bestTrial);

public:

  // Добавить в SearchDataSerializer.h:
  struct LoadedFileData {
    std::string version;
    std::string mode;
    LoadedMethodParameters methodParams;
    LoadedSearchData searchData;
    std::vector<Trial*> trials;
    Trial* bestTrial = nullptr;

    LoadedFileData()
    {
    }

    // Запрещаем копирование
    LoadedFileData(const LoadedFileData&) = delete;
    LoadedFileData& operator=(const LoadedFileData&) = delete;

    // Разрешаем перемещение
    LoadedFileData(LoadedFileData&& other) noexcept
      : version(std::move(other.version))
      , mode(std::move(other.mode))
      , methodParams(std::move(other.methodParams))
      , searchData(std::move(other.searchData))
      , trials(std::move(other.trials))
      , bestTrial(other.bestTrial)
    {
      other.bestTrial = nullptr;
    }
  };

  // Метод для загрузки с возвратом данных
  bool LoadFromFile(const std::string& filename, LoadedFileData& outData);


  SearchDataSerializer();

  /// Задаем поисковую информацию
  void SetSearchData(SearchData* data);

  /// Задачем задачу
  void SetTask(Task* task);

  /// Сериализация полного состояния в JSON строку
  std::string SerializeFullState();

  /// Сохранение состояния (первый раз - полное, потом - добавление)
  bool SaveProgress(const std::string& filename,
    const std::vector<Trial*>& newTrials = std::vector<Trial*>(),
    const std::vector<SearchInterval*>& newIntervals = std::vector<SearchInterval*>(),
    Trial* newBestTrial = nullptr);


  /// Очищаем информацию о файле
  void ResetFirstSave();


  /**
   * \brief Простой JSON парсер для чтения данных из файла
   */
  class JSONParser
  {
  private:
    /// Весь файл в строке
    std::string content;

    /// текущая позиция
    size_t pos;

    /// Пропускаем проблемы в разбиремом тексте
    void SkipWhitespace();

    /// Подготовка строки к работе
    std::string ParseString();

    /// преобразование формата чисел
    double ParseNumber();

    /// Убираем не печатные символы
    std::string ParseWord();

    /// Обработка квадратных скобок
    std::string ParseArrayAsString();

    /// Обработка фигурных скобок
    std::string ParseObjectAsString();

  public:
    JSONParser(const std::string& str);

    /// Сброс текущей позиции
    void Reset();

    /// Разбираем файл
    std::map<std::string, std::string> ParseObject();

    /// разбираем массивы
    std::vector<std::map<std::string, std::string> > ParseArray();

    /// Разбираем масивы double
    std::vector<double> ParseDoubleArray();

    /// Разбираем масивы int
    std::vector<int> ParseIntArray();
  };
};

#endif // __SEARCH_DATA_SERIALIZER_H__