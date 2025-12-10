/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      properties_test.cpp                                         //
//                                                                         //
//  Purpose:   Модульные тесты для классов параметров                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file properties_test.cpp

\authors Lebedev I.
\copyright ННГУ им. Н.И. Лобачевского

\brief Модульные тесты для классов параметров

*/

#include "Parameters.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

using namespace std;

int PropertiesIndex = 11;

class PropertiesTest : public ::testing::Test
{

public:

  bool boolVal;

  bool GetBool() const
  {
    return boolVal;
  }
  void SetBool(bool val)
  {
    boolVal = val;
  }

  virtual int CheckValue(int index = -1)
  {
    boolVal = !boolVal;

    return 0;
  }

  virtual void TestBody()
  {  }

  PropertiesTest()
  {
    boolVal = true;
  }
};

/**
 * Проверка класса TBool
 */
TEST(Properties_TBool, can_create_default_Bool)
{
  ASSERT_NO_THROW(TBool<PropertiesTest> a);
}

///  может создать объект TBool с заданным значением
TEST(Properties_TBool, can_create_Bool)
{
  bool val = true;
  ASSERT_NO_THROW(TBool<PropertiesTest> a(val));
}

///  инициализированное значение TBool соответствует ожидаемому
TEST(Properties_TBool, is_init_Bool_value)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, (bool)b);
}

///  работа геттера и сеттера для TBool
TEST(Properties_TBool, is_getter_and_setter_working_Bool)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);

  b = val;

  ASSERT_EQ(val, (bool)b);
}

///  работа SetIndex и GetIndex для TBool
TEST(Properties_TBool, is_SetIndex_and_GetIndex_working_Bool)
{
  int val = PropertiesIndex++;
  TBool<PropertiesTest> b;

  b.SetIndex(val);

  ASSERT_EQ(val, b.GetIndex());
}

///  метод GetData для TBool возвращает корректное значение
TEST(Properties_TBool, is_Bool_GetData_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, b.GetData());
}

///  метод GetValue для TBool возвращает корректное значение
TEST(Properties_TBool, is_Bool_GetValue_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, *((bool*)b.GetValue()));
}

///  метод Clone для TBool работает корректно
TEST(Properties_TBool, is_Bool_Clone_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  TBool<PropertiesTest>* c;

  b.Clone((BaseProperty<PropertiesTest>**) & c);

  ASSERT_EQ(val, c->GetData());
}

///  метод GetIsChange для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetIsChange_working)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);

  ASSERT_EQ(false, b.GetIsChange());

  b = val;

  ASSERT_EQ(true, b.GetIsChange());
}

///  метод Copy для TBool работает корректно
TEST(Properties_TBool, is_Bool_Copy_working)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);
  TBool<PropertiesTest> c(val);

  b.Copy((void*)&c);

  ASSERT_EQ(val, b.GetData());
}

///  метод GetCurrentStringValue для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetCurrentStringValue_working)
{
  bool val = true;
  string name = "n";
  string result = "n = true";
  TBool<PropertiesTest> b(val);

  b.SetName(name);

  ASSERT_EQ(result, b.GetCurrentStringValue());
}

///  работа SetName и GetName для TBool
TEST(Properties_TBool, is_Bool_SetName_and_GetName_working)
{
  string name = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetName(name));

  ASSERT_EQ(name, b.GetName());
}

///  метод IsNameEqual для TBool работает корректно
TEST(Properties_TBool, is_Bool_IsNameEqual_working)
{
  string name = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetName(name));

  ASSERT_EQ(true, b.IsNameEqual(name));
}

///  метод IsFlag для TBool возвращает false
TEST(Properties_TBool, is_Bool_IsFlag_working)
{
  TBool<PropertiesTest> b;

  ASSERT_EQ(false, b.IsFlag());
}

///  работа SetIsReadValue и GetIsReadValue для TBool
TEST(Properties_TBool, is_Bool_SetIsReadValue_and_GetIsReadValue_working)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetIsReadValue(true));

  ASSERT_EQ(true, b.GetIsReadValue());
}

///  работа SetIsPreChange и IsPreChange для TBool
TEST(Properties_TBool, is_Bool_SetIsPreChange_and_IsPreChange_working)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetIsPreChange(true));

  ASSERT_EQ(true, b.IsPreChange());
}

///  работа SetHelp и GetHelp для TBool
TEST(Properties_TBool, is_Bool_SetHelp_and_GetHelp_working)
{
  string help = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetHelp(help));

  ASSERT_EQ(help, b.GetHelp());
}

///  работа SetLink и GetLink для TBool
TEST(Properties_TBool, is_Bool_SetLink_and_GetLink_working)
{
  string link = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetLink(link));

  ASSERT_EQ(link, b.GetLink());
}

///  метод GetHelpString для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetHelpString_working)
{
  string result = "b (-b) - 'This is B' default:\ttrue";
  string link = "-b";
  string help = "This is B";
  string name = "b";
  bool val = true;
  TBool<PropertiesTest> b(val);

  b.SetLink(link);
  b.SetHelp(help);
  b.SetName(name);

  ASSERT_EQ(result, b.GetHelpString());
}

///  метод Init для TBool не вызывает исключения
TEST(Properties_TBool, is_init_function_Bool)
{
  PropertiesTest a;
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue));
}


///  метод InitializationParameterProperty для TBool работает корректно
TEST(Properties_TBool, is_InitializationParameterProperty_function_Bool)
{
  int index = 12;
  string result = "b (-b) - 'This is B' default:\ttrue";
  string link = "-b";
  string help = "This is B";
  string name = "b";
  string sep = "_";
  string defVal = "true";
  bool val = true;

  PropertiesTest a;
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.InitializationParameterProperty(&a, &PropertiesTest::CheckValue, index,
    sep, 1, name, help, link, defVal));

  ASSERT_EQ(val, b);
  ASSERT_EQ(link, b.GetLink());
  ASSERT_EQ(help, b.GetHelp());
  ASSERT_EQ(name, b.GetName());
  ASSERT_EQ(result, b.GetHelpString());
}

///  геттер владельца для TBool работает корректно
TEST(Properties_TBool, is_owner_getter_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(!val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);
  a.boolVal = val;

  ASSERT_EQ(val, (bool)b);
}

///  сеттер владельца для TBool работает корректно
TEST(Properties_TBool, is_owner_setter_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(!val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);

  a.boolVal = !val;

  b = val;

  ASSERT_EQ(val, a.boolVal);
}

///  метод CheckValue для TBool работает корректно
TEST(Properties_TBool, is_CheckValue_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b;

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue);

  b = !val;

  ASSERT_EQ(val, b);
}

///  метод GetAvailableData для TBool возвращает корректное значение
TEST(Properties_TBool, is_GetAvailableData_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);

  b = !val;

  ASSERT_EQ(val, b.GetAvailableData());
}

///  работа GetGetter и SetGetter для TBool
TEST(Properties_TBool, is_GetGetter_and_SetGetter_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetGetter(&PropertiesTest::GetBool));

  ASSERT_EQ(&PropertiesTest::GetBool, b.GetGetter());
}

///  метод GetIsHaveValue для TBool работает корректно
TEST(Properties_TBool, is_GetIsHaveValue_working_Bool)
{
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetGetter(&PropertiesTest::GetBool));

  ASSERT_EQ(false, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetGetter(0));

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetSetter(&PropertiesTest::SetBool));

  ASSERT_EQ(false, b.GetIsHaveValue());
}

///  метод SetIsHaveValue для TBool работает корректно
TEST(Properties_TBool, is_SetIsHaveValue_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue);

  a.boolVal = !val;

  ASSERT_EQ(false, b.GetIsHaveValue());

  ASSERT_EQ(!val, b);

  b.SetIsHaveValue(true);

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_EQ(val, b);
}

///  работа GetSetter и SetSetter для TBool
TEST(Properties_TBool, is_GetSetter_and_SetSetter_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetSetter(&PropertiesTest::SetBool));

  ASSERT_EQ(&PropertiesTest::SetBool, b.GetSetter());
}

///  работа SetCheckValue и GetCheckValue для TBool
TEST(Properties_TBool, is_SetCheckValue_and_GetCheckValue_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetCheckValue(&PropertiesTest::CheckValue));

  ASSERT_EQ(&PropertiesTest::CheckValue, b.GetCheckValue());
}

///  метод CheckValue (внутренний) для TBool работает корректно
TEST(Properties_TBool, is_CheckValue__working_Bool)
{
  PropertiesTest a;
  TBool<PropertiesTest> b;
  a.boolVal = false;

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, &PropertiesTest::CheckValue);

  ASSERT_NO_THROW(b.CheckValue());

  ASSERT_EQ(true, a.boolVal);
}


///  метод ToString для TBool работает корректно
TEST(Properties_TBool, is_Bool_ToString_working)
{
  string result = "true";
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(result, b.ToString());
}

///  метод FromString для TBool работает корректно
TEST(Properties_TBool, is_Bool_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TBool<PropertiesTest> b(!val);

  ASSERT_NO_THROW(b.FromString(sVal));

  ASSERT_EQ(val, b);
}

///  оператор присваивания строки для TBool работает корректно
TEST(Properties_TBool, is_Bool_operator_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TBool<PropertiesTest> b(!val);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TBool работает корректно
TEST(Properties_TBool, is_Bool_operator_ToString_working)
{
  string result = "true";
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TFlag
 */
TEST(Properties_TFlag, can_create_default_Flag)
{
  ASSERT_NO_THROW(TFlag<PropertiesTest> a);
}

///  может создать объект TFlag с заданным значением
TEST(Properties_TFlag, can_create_Flag)
{
  bool val = true;
  ASSERT_NO_THROW(TFlag<PropertiesTest> a(val));
}

///  инициализированное значение TFlag соответствует ожидаемому
TEST(Properties_TFlag, is_init_Flag_value)
{
  bool val = true;
  TFlag<PropertiesTest> b(val);
  ASSERT_EQ(val, (bool)b);
}

///  метод IsFlag для TFlag возвращает true
TEST(Properties_TFlag, is_Flag_IsFlag_working)
{
  TFlag<PropertiesTest> b;

  ASSERT_EQ(true, b.IsFlag());
}

///  оператор присваивания строки для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_operator_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TFlag<PropertiesTest> b(!val);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_operator_ToString_working)
{
  string result = "true";
  bool val = true;
  TFlag<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TInt
 */
TEST(Properties_TInt, can_create_default_Int)
{
  ASSERT_NO_THROW(TInt<PropertiesTest> a);
}

///  может создать объект TInt с заданным значением
TEST(Properties_TInt, can_create_Int)
{
  int val = 42;
  ASSERT_NO_THROW(TInt<PropertiesTest> a(val));
}

///  инициализированное значение TInt соответствует ожидаемому
TEST(Properties_TInt, is_init_Int_value)
{
  int val = 42;
  TInt<PropertiesTest> b(val);
  ASSERT_EQ(val, (int)b);
}

///  оператор присваивания строки для TInt работает корректно
TEST(Properties_TInt, is_Int_operator_FromString_working)
{
  int val = 17;
  string sVal = "17";
  TInt<PropertiesTest> b(val + 1);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TInt работает корректно
TEST(Properties_TInt, is_Int_operator_ToString_working)
{
  int val = 17;
  string result = "17";
  TInt<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TDouble
 */
TEST(Properties_TDouble, can_create_default_Double)
{
  ASSERT_NO_THROW(TDouble<PropertiesTest> a);
}

///  может создать объект TDouble с заданным значением
TEST(Properties_TDouble, can_create_Double)
{
  double val = 17.3;
  ASSERT_NO_THROW(TDouble<PropertiesTest> a(val));
}

///  инициализированное значение TDouble соответствует ожидаемому
TEST(Properties_TDouble, is_init_Double_value)
{
  double val = 17.3;
  TDouble<PropertiesTest> b(val);
  ASSERT_EQ(val, (double)b);
}

///  оператор присваивания строки для TDouble работает корректно
TEST(Properties_TDouble, is_Double_operator_FromString_working)
{
  double val = 17.3;
  string sVal = "17.3";
  TDouble<PropertiesTest> b(val + 1);

  b = sVal;

  ASSERT_EQ(val, (double)b);
}

///  оператор преобразования в строку для TDouble работает корректно
TEST(Properties_TDouble, is_Double_operator_ToString_working)
{
  double val = 17.987654;
  string result = "17.987654";
  TDouble<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TString
 */
TEST(Properties_TString, can_create_default_String)
{
  ASSERT_NO_THROW(TString<PropertiesTest> a);
}

///  может создать объект TString с заданным значением
TEST(Properties_TString, can_create_String)
{
  string val = "abc";
  ASSERT_NO_THROW(TString<PropertiesTest> a(val));
}

///  инициализированное значение TString соответствует ожидаемому
TEST(Properties_TString, is_init_String_value)
{
  string val = "abc";
  TString<PropertiesTest> b(val);
  ASSERT_EQ(val, b.GetData());
}

/**
 * Проверка класса TStrings
 */
TEST(Properties_TStrings, can_create_default_Strings)
{
  ASSERT_NO_THROW(TStrings<PropertiesTest> a);
}

///  инициализированное значение TStrings соответствует ожидаемому
TEST(Properties_TStrings, is_init_Strings_value)
{
  string val[3] = { "abc", "def", "gih" };
  TStrings<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}


///  оператор присваивания строки для TStrings работает корректно
TEST(Properties_TStrings, is_Strings_operator_FromString_working)
{
  string val = "a_b_c";
  string result[] = { "a", "b", "c" };
  TStrings<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
  //ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TStrings работает корректно
TEST(Properties_TStrings, is_Strings_operator_ToString_working)
{
  string sVal[] = { "a", "b", "c" };
  string result = "a_b_c";
  TStrings<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TInts
 */
TEST(Properties_TInts, can_create_Ints)
{
  ASSERT_NO_THROW(TInts<PropertiesTest> a);
}

///  инициализированное значение TInts соответствует ожидаемому
TEST(Properties_TInts, is_init_Ints_value)
{
  int val[3] = { 1, 2, 3 };
  TInts<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}

///  оператор присваивания строки для TInts работает корректно
TEST(Properties_TInts, is_Ints_operator_FromString_working)
{
  string val = "1_2_3";
  int result[] = { 1, 2, 3 };
  TInts<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
}

///  оператор преобразования в строку для TInts работает корректно
TEST(Properties_TInts, is_Ints_operator_ToString_working)
{
  int sVal[] = { 1, 2, 3 };
  string result = "1_2_3";
  TInts<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TDoubles
 */
TEST(Properties_TDoubles, can_create_Doubles)
{
  ASSERT_NO_THROW(TDoubles<PropertiesTest> a);
}

///  инициализированное значение TDoubles соответствует ожидаемому
TEST(Properties_TDoubles, is_init_Doubles_value)
{
  double val[3] = { 1.1, 2.30, 3.54 };
  ASSERT_NO_THROW(TDoubles<PropertiesTest> a);
  TDoubles<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}


///  оператор присваивания строки для TDoubles работает корректно
TEST(Properties_TDoubles, is_TDoubles_operator_FromString_working)
{
  string val = "1.1_2.2_3.3";
  double result[] = { 1.1, 2.2, 3.3 };
  TDoubles<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
}

///  оператор преобразования в строку для TDoubles работает корректно
TEST(Properties_TDoubles, is_TDoubles_operator_ToString_working)
{
  double sVal[] = { 1.1, 2.2, 3.3 };
  string result = "1.100000_2.200000_3.300000";
  TDoubles<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TETypeMethod
 */
TEST(Properties_TETypeMethod, can_create_default_ETypeMethod)
{
  ASSERT_NO_THROW(TETypeMethod<PropertiesTest> a);
}


/**
 * Проверка класса TESeparableMethodType
 */
TEST(Properties_TESeparableMethodType, can_create_default_ESeparableMethodType)
{
  ASSERT_NO_THROW(TESeparableMethodType<PropertiesTest> a);
}

///  может создать объект TESeparableMethodType с заданным значением
TEST(Properties_TESeparableMethodType, can_create_ESeparableMethodType)
{
  ESeparableMethodType val = GridSearch;
  ASSERT_NO_THROW(TESeparableMethodType<PropertiesTest> a(val));
}

///  инициализированное значение TESeparableMethodType соответствует ожидаемому
TEST(Properties_TESeparableMethodType, is_init_ESeparableMethodType_value)
{
  ESeparableMethodType val = GridSearch;
  TESeparableMethodType<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TELocalMethodScheme
 */
TEST(Properties_TELocalMethodScheme, can_create_default_ELocalMethodScheme)
{
  ASSERT_NO_THROW(TELocalMethodScheme<PropertiesTest> a);
}


/**
 * Проверка класса TEStopCondition
 */
TEST(Properties_TEStopCondition, can_create_default_EStopCondition)
{
  ASSERT_NO_THROW(TEStopCondition<PropertiesTest> a);
}

///  может создать объект TEStopCondition с заданным значением
TEST(Properties_TEStopCondition, can_create_EStopCondition)
{
  EStopCondition val = OptimumVicinity2;
  ASSERT_NO_THROW(TEStopCondition<PropertiesTest> a(val));
}

///  инициализированное значение TEStopCondition соответствует ожидаемому
TEST(Properties_TEStopCondition, is_init_EStopCondition_value)
{
  EStopCondition val = OptimumVicinity2;
  TEStopCondition<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TETypeCalculation
 */
TEST(Properties_TETypeCalculation, can_create_default_ETypeCalculation)
{
  ASSERT_NO_THROW(TETypeCalculation<PropertiesTest> a);
}

///  может создать объект TETypeCalculation с заданным значением
TEST(Properties_TETypeCalculation, can_create_ETypeCalculation)
{
  ETypeCalculation val = CUDA;
  ASSERT_NO_THROW(TETypeCalculation<PropertiesTest> a(val));
}

///  инициализированное значение TETypeCalculation соответствует ожидаемому
TEST(Properties_TETypeCalculation, is_init_ETypeCalculation_value)
{
  ETypeCalculation val = CUDA;
  TETypeCalculation<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TETypeProcess
 */
TEST(Properties_TETypeProcess, can_create_default_ETypeProcess)
{
  ASSERT_NO_THROW(TETypeProcess<PropertiesTest> a);
}


/**
 * Проверка класса TEMapType
 */
TEST(Properties_TEMapType, can_create_default_EMapType)
{
  ASSERT_NO_THROW(TEMapType<PropertiesTest> a);
}
