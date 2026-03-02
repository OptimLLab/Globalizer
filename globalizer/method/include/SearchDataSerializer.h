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
private:
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

  // В класс SearchDataSerializer добавить объявления:
private:
  // Подфункции для AppendNewPoints
  bool ReadFileContent(const std::string& filename, std::string& content);
  bool WriteFileContent(const std::string& filename, const std::string& content);

  bool FindTrialsPosition(const std::string& content, size_t& trialsPos, size_t& trialsEnd);
  bool HasExistingPoints(const std::string& content, size_t trialsPos, size_t trialsEnd);
  std::string BuildNewTrialsString(const std::vector<Trial*>& newTrials, bool hasExistingPoints);
  bool FindTrialsEnd(const std::string& content, size_t trialsPos, size_t& trialsEnd);
  bool AppendNewTrials(std::string& content, const std::vector<Trial*>& newTrials);

  bool FindCountPosition(const std::string& content, size_t& countPos, size_t& countEnd);
  bool UpdateCount(std::string& content, int newCount);

  bool FindBestTrialPosition(const std::string& content, size_t& bestTrialPos,
    size_t& bestTrialEnd, bool& hasComma);
  bool UpdateBestTrial(std::string& content, Trial* newBestTrial);

  bool FindTimestampPosition(const std::string& content, size_t& timestampStart,
    size_t& timestampEnd);
  bool UpdateTimestamp(std::string& content);

  bool FindMPosition(const std::string& content, size_t& mPos, size_t& mEnd);
  bool UpdateMArray(std::string& content, const double* M, int numOfFuncs);

  bool FindZPosition(const std::string& content, size_t& zPos, size_t& zEnd);
  bool UpdateZArray(std::string& content, const double* Z, int numOfFuncs);

  bool FindLocalRPosition(const std::string& content, size_t& localRPos, size_t& localREnd);
  bool UpdateLocalR(std::string& content, double local_r);

  // В класс SearchDataSerializer добавить:
private:
  // Копии предыдущих значений для отслеживания изменений
  std::vector<double> previousM;
  std::vector<double> previousZ;
  double previousLocalR;

  // Флаги изменений
  bool mArrayChanged;
  bool zArrayChanged;
  bool localRChanged;

  // Методы для проверки изменений
  void UpdateMChanges();
  void UpdateZChanges();
  void UpdateLocalRChanges();
  void ResetChangeFlags();

public:
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


  /// Загрузка состояния из файла
  bool LoadFromFile(const std::string& filename);

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

    void SkipWhitespace();

    std::string ParseString();

    double ParseNumber();

    std::string ParseWord();

    std::string ParseArrayAsString();

    std::string ParseObjectAsString();

  public:
    JSONParser(const std::string& str);

    void Reset();

    std::map<std::string, std::string> ParseObject();

    std::vector<std::map<std::string, std::string> > ParseArray();

    std::vector<double> ParseDoubleArray();

    std::vector<int> ParseIntArray();
  };
};

#endif // __SEARCH_DATA_SERIALIZER_H__