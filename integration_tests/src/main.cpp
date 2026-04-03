#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>
#include <functional>
#include <limits>

#include "json.hpp"

#define _GLOBALIZER_BENCHMARKS

#include "Solver.h"
#include "ProblemInterface.h"
#include "Parameters.h"
#include "Trial.h"
#include "GlobalizerProblem.h" 
#include "GlobalOptimizationProblemManager.h"
#include "IGlobalOptimizationProblem.h"

using json = nlohmann::json;

// ==============================================================================
// Вспомогательные функции
// ==============================================================================

/**
 * Вычисление Евклидова расстояния между двумя точками
 */
double CalculateDistance(const double* p1, const std::vector<double>& p2, int dim)
{
    double sum = 0.0;
    for (int i = 0; i < dim; i++)
    {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/**
 * Математика для задачи StronginC3 (3 ограничения + 1 критерий)
 */
double StronginC3Functionals(const double* y, int fNumber)
{
    double res = 0.0;
    double x1 = y[0];
    double x2 = y[1];

    switch (fNumber)
    {
    case 0:
        res = 0.01 * ((x1 - 2.2) * (x1 - 2.2) + (x2 - 1.2) * (x2 - 1.2) - 2.25);
        break;
    case 1:
        res = 100.0 * (1.0 - ((x1 - 2.0) / 1.2) * ((x1 - 2.0) / 1.2) - (x2 / 2.0) * (x2 / 2.0));
        break;
    case 2:
        res = 10.0 * (x2 - 1.5 - 1.5 * sin(6.283 * (x1 - 1.75)));
        break;
    case 3:
    {
        double t1 = pow(0.5 * x1 - 0.5, 4.0);
        double t2 = pow(x2 - 1.0, 4.0);
        res = 1.5 * x1 * x1 * exp(1.0 - x1 * x1 - 20.25 * (x1 - x2) * (x1 - x2));
        res = res + t1 * t2 * exp(2.0 - t1 - t2);
        res = -res;
    }
    break;
    }
    return res;
}

// ==============================================================================
// Фабрика задач
// ==============================================================================
IProblem* CreateProblem(const json& config, int dimension)
{
    std::string name = config["name"].get<std::string>();
    std::string type = config.value("type", "Benchmark");

    if (type == "Simple")
    {
        if (name == "Hill")
        {
            return new ProblemFromFunctionPointers(
                dimension,
                std::vector<double>(dimension, 0.0),
                std::vector<double>(dimension, 1.0),
                std::vector<std::function<double(const double*)>>(1, [dimension](const double* y)
                    {
                        double sum = 0.0;
                        for (int j = 0; j < dimension; j++)
                        {
                            double diff = y[j] - 0.5;
                            sum += diff * diff;
                        }
                        return sum;
                    }),
                true, 0.0, std::vector<double>(dimension, 0.5)
            );
        }
        else if (name == "Rastrigin" || name == "Rastrigin_3D" || name == "Rastrigin_5D")
        {
            return new ProblemFromFunctionPointers(
                dimension,
                std::vector<double>(dimension, -2.2),
                std::vector<double>(dimension, 1.8),
                std::vector<std::function<double(const double*)>>(1, [dimension](const double* y)
                    {
                        double pi = 3.14159265358979323846;
                        double sum = 0.0;
                        for (int j = 0; j < dimension; j++)
                        {
                            sum += y[j] * y[j] - 10.0 * cos(2.0 * pi * y[j]) + 10.0;
                        }
                        return sum;
                    }),
                true, 0.0, std::vector<double>(dimension, 0.0)
            );
        }
        else if (name == "StronginC3_Lambda")
        {
            return new ProblemFromFunctionPointers(
                dimension, { 0.0, -1.0 }, { 4.0, 3.0 },
                std::vector<std::function<double(const double*)>>({
                  [](const double* y) { return 0.01 * ((y[0] - 2.2) * (y[0] - 2.2) + (y[1] - 1.2) * (y[1] - 1.2) - 2.25); },
                  [](const double* y) { return 100.0 * (1.0 - ((y[0] - 2.0) / 1.2) * ((y[0] - 2.0) / 1.2) - (y[1] / 2.0) * (y[1] / 2.0)); },
                  [](const double* y) { return 10.0 * (y[1] - 1.5 - 1.5 * sin(6.283 * (y[0] - 1.75))); },
                  [](const double* y) {
                    double t1 = pow(0.5 * y[0] - 0.5, 4.0);
                    double t2 = pow(y[1] - 1.0, 4.0);
                    return -((1.5 * y[0] * y[0] * exp(1.0 - y[0] * y[0] - 20.25 * (y[0] - y[1]) * (y[0] - y[1]))) + t1 * t2 * exp(2.0 - t1 - t2));
                  }
                    }),
                true, -1.489444, { 0.941176, 0.941176 }
            );
        }
    }
    return nullptr;
}

// ==============================================================================
// Запуск одного теста
// ==============================================================================
bool RunSingleTest(const json& config)
{
    std::string name = config["name"].get<std::string>();
    std::string type = config.value("type", "Benchmark");
    int dimension = config["dimension"].get<int>();

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << " Testing problem: " << name << " (Type: " << type << ", Dim: " << dimension << ")" << std::endl;

    // 1. Загрузка/Создание задачи
    IProblem* manualProblem = nullptr;
    IGlobalOptimizationProblem* dllProblem = nullptr;
    GlobalOptimizationProblemManager problemManager;

    if (type == "Benchmark" || type == "GCGen")
    {
        std::string libPath = name + ".dll";
        if (problemManager.LoadProblemLibrary(libPath) == GlobalOptimizationProblemManager::OK_)
        {
            dllProblem = problemManager.GetProblem();
        }
    }

    if (!dllProblem)
    {
        manualProblem = CreateProblem(config, dimension);
    }

    if (!dllProblem && !manualProblem)
    {
        std::cout << " [SKIP] Problem not found." << std::endl;
        return false;
    }

    // 2. Настройка параметров Globalizer
    char progName[] = "tester";
    char* dummyArgv[] = { progName, nullptr };

    try
    {
        parameters.Init(1, dummyArgv, false);
        parameters.Dimension = dimension;
        parameters.r = config["r"].get<double>();
        parameters.Epsilon = config.value("epsilon", 0.01);

        // ВАЖНО: m здесь — это ПОРЯДОК ЭВОЛЬВЕНТЫ (плотность поиска).
        // Берем его из JSON (обычно 10-12). Не ставим 0!
        parameters.m = config.value("m", 10);

        std::string multipliers = "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 "
            "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0";
        parameters.SetVal("functionSignMultiplier", multipliers);
    }
    catch (...)
    {
        std::cout << " [FAIL] Parameters Initialization Error!" << std::endl;
        return false;
    }

    // Инициализируем задачу
    if (dllProblem)
    {
        dllProblem->SetDimension(dimension);
        dllProblem->Initialize();
    }
    else
    {
        manualProblem->Initialize();
    }

    // 3. Решение
    Solver* solver = (dllProblem) ? new Solver(dllProblem) : new Solver(manualProblem);

    try
    {
        if (solver->Solve() != SYSTEM_OK)
        {
            std::cout << " [FAIL] Solver returned error." << std::endl;
            delete solver; return false;
        }
    }
    catch (...)
    {
        std::cout << " [FAIL] Exception during Solve!" << std::endl;
        delete solver; return false;
    }

    // 4. Сверка результатов
    SolutionResult* result = solver->GetSolutionResult();
    bool passed = true;

    if (result && result->BestTrial)
    {
        int totalFunctions = (dllProblem) ? dllProblem->GetNumberOfFunctions() : manualProblem->GetNumberOfFunctions();

        // Проверка значения (критерий всегда последний в массиве)
        double actualValue = result->BestTrial->FuncValues[totalFunctions - 1];
        double expectedValue = config["optimum"].get<double>();
        double tolerance = config["tolerance"].get<double>();

        std::cout << " [Value]: Expected " << expectedValue << ", Actual " << actualValue << std::endl;
        if (std::abs(actualValue - expectedValue) > tolerance) passed = false;

        // Проверка координат
        if (config.contains("point"))
        {
            std::vector<double> expectedPoint = config["point"].get<std::vector<double>>();
            double dist = CalculateDistance(result->BestTrial->y, expectedPoint, dimension);
            std::cout << " [Point]: Distance to optimum: " << dist << std::endl;
            if (dist > 0.1) passed = false; // Допуск 0.1 для координат
        }

        // Проверка итераций
        if (config.contains("expected_iterations"))
        {
            int actualIters = result->TrialCount;
            int expectedIters = config["expected_iterations"].get<int>();
            std::cout << " [Iters]: iOpt: " << expectedIters << ", Globalizer: " << actualIters << std::endl;

            if (actualIters > expectedIters * 2.0)
                std::cout << " [WARN]: Globalizer is much slower than iOpt!" << std::endl;
        }
    }

    std::cout << (passed ? " [PASS]" : " [FAIL]") << std::endl;
    delete solver;
    return passed;
}

int main(int argc, char* argv[])
{
    std::string jsonPath = "tests.json";
    if (argc > 1) jsonPath = argv[1];

    std::ifstream f(jsonPath);
    if (!f.is_open())
    {
        std::cerr << "Cannot open config file: " << jsonPath << std::endl;
        return 1;
    }

    json testsData;
    try {
        testsData = json::parse(f);
    }
    catch (const std::exception& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return 1;
    }

    int passed = 0;
    for (const auto& testConfig : testsData)
    {
        if (RunSingleTest(testConfig)) passed++;
    }

    std::cout << "\nTests finished. Passed: " << passed << "/" << testsData.size() << std::endl;
    return (passed == testsData.size()) ? 0 : 1;
}