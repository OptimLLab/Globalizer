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
// 1. МАТЕМАТИКА ИЗ SimpleMain.cpp (Встроенная)
// ==============================================================================
double StronginC3Functionals(const double* y, int fNumber)
{
    double res = 0.0;
    double x1 = y[0], x2 = y[1];
    switch (fNumber)
    {
    case 0: res = 0.01 * ((x1 - 2.2) * (x1 - 2.2) + (x2 - 1.2) * (x2 - 1.2) - 2.25); break;
    case 1: res = 100.0 * (1.0 - ((x1 - 2.0) / 1.2) * ((x1 - 2.0) / 1.2) - (x2 / 2.0) * (x2 / 2.0)); break;
    case 2: res = 10.0 * (x2 - 1.5 - 1.5 * sin(6.283 * (x1 - 1.75))); break;
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
// 2. ФАБРИКА ЗАДАЧ
// ==============================================================================
IProblem* CreateProblem(const json& config, int dimension) {
    // ВАЖНО: Читаем поля через ["ключ"], а не конвертируем весь объект
    std::string name = config["name"].get<std::string>();
    std::string type = config.value("type", "Benchmark");

    if (type == "Simple") {
        if (name == "Rastrigin") {
            return new ProblemFromFunctionPointers(dimension,
                std::vector<double>(dimension, -2.2),
                std::vector<double>(dimension, 1.8),
                std::vector<std::function<double(const double*)>>(1, [dimension](const double* y) {
                    double pi_ = 3.14159265358979323846;
                    double sum = 0.;
                    for (int j = 0; j < dimension; j++)
                        sum += y[j] * y[j] - 10. * cos(2.0 * pi_ * y[j]) + 10.0;
                    return sum;
                    }),
                true, 0, std::vector<double>(dimension, 0)
            );
        }
        else if (name == "StronginC3_Lambda") {
            return new ProblemFromFunctionPointers(dimension,
                { 0.0, -1.0 }, { 4.0, 3.0 },
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
                true, -1.489444, { 0.941176,  0.941176 }
            );
        }
        else if (name == "StronginC3_Pointer") {
            return new ProblemFromFunctionPointers(dimension,
                { 0.0, -1.0 }, { 4.0, 3.0 }, StronginC3Functionals, 4
            );
        }
    }
    return nullptr;
}

// ==============================================================================
// 3. ЗАПУСК ТЕСТА
// ==============================================================================
bool RunSingleTest(const json& config) {
    // ВАЖНО: Используем ["ключ"] для обязательных полей
    std::string name = config["name"].get<std::string>();
    std::string type = config.value("type", "Benchmark");

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << " Testing problem: " << name << " (Type: " << type << ")" << std::endl;

    // Безопасная инициализация параметров
    char progName[] = "tester";
    char* dummy_argv[] = { progName, nullptr };

    try {
        parameters.Init(1, dummy_argv, false);

        // Читаем параметры из JSON
        parameters.Dimension = config["dimension"].get<int>();
        parameters.r = config["r"].get<double>();
        parameters.Epsilon = config.value("epsilon", 0.01);
        parameters.m = config.value("m", 10);

        std::string multipliers = "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0";
        parameters.SetVal("functionSignMultiplier", multipliers);
    }
    catch (const std::exception& e) {
        std::cout << " [FAIL] Parameters Init Error: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cout << " [FAIL] Unknown crash during Parameters Initialization!" << std::endl;
        return false;
    }

    IProblem* manualProblem = nullptr;
    IGlobalOptimizationProblem* dllProblem = nullptr;
    GlobalOptimizationProblemManager problemManager;

    // 1. Пытаемся загрузить DLL (для Benchmark и GCGen)
    if (type == "Benchmark" || type == "GCGen") {
        std::string libName = name;
#ifdef _WIN32
        std::string libPath = libName + ".dll";
#else
        std::string libPath = "lib" + libName + ".so";
#endif
        if (problemManager.LoadProblemLibrary(libPath) == GlobalOptimizationProblemManager::OK_) {
            dllProblem = problemManager.GetProblem();
            if (dllProblem) {
                std::cout << " Loaded from DLL: " << libPath << std::endl;
                dllProblem->SetDimension(parameters.Dimension);
                dllProblem->Initialize();
            }
        }
        else {
            if (type == "GCGen") {
                std::cout << " [SKIP] GKLS/GCGen source not available and DLL not found." << std::endl;
                return true;
            }
        }
    }

    // 2. Встроенная реализация (Simple)
    if (!dllProblem) {
        manualProblem = CreateProblem(config, parameters.Dimension);
        if (manualProblem) {
            std::cout << " Using built-in implementation." << std::endl;
            manualProblem->Initialize();
        }
    }

    if (!dllProblem && !manualProblem) {
        std::cout << " [SKIP] Problem not found: " << name << std::endl;
        return false;
    }

    // 3. Решение
    Solver* solver = nullptr;
    if (dllProblem) solver = new Solver(dllProblem);
    else solver = new Solver(manualProblem);

    // Ловим исключения решателя
    try {
        if (solver->Solve() != SYSTEM_OK) {
            std::cout << " [FAIL] Solver returned error code (not SYSTEM_OK)." << std::endl;
            delete solver;
            // manualProblem не удаляем, так как Solver мог взять владение
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cout << " [FAIL] Exception: " << e.what() << std::endl;
        delete solver;
        return false;
    }
    catch (...) {
        std::cout << " [FAIL] Unknown Exception during Solve!" << std::endl;
        delete solver;
        return false;
    }

    // 4. Сверка
    SolutionResult* result = solver->GetSolutionResult();
    bool passed = false;

    if (result && result->BestTrial) {
        int funcCount = dllProblem ? dllProblem->GetNumberOfFunctions() : manualProblem->GetNumberOfFunctions();
        int criterionIndex = funcCount - 1;

        double actualValue = result->BestTrial->FuncValues[criterionIndex];

        // Читаем эталон из JSON
        double expectedValue = config["optimum"].get<double>();
        double tolerance = config["tolerance"].get<double>();

        double diff = std::abs(actualValue - expectedValue);

        std::cout << "      Expected: " << expectedValue << ", Actual: " << actualValue << std::endl;

        if (diff <= tolerance) {
            std::cout << " [PASS] " << name << " passed." << std::endl;
            passed = true;
        }
        else {
            std::cout << " [FAIL] Difference: " << diff << " > " << tolerance << std::endl;
            passed = false;
        }
    }
    else {
        std::cout << " [FAIL] No result returned." << std::endl;
        passed = false;
    }

    delete solver;
    // ВАЖНО: Мы НЕ удаляем manualProblem здесь, так как Solver часто удаляет переданную ему задачу в своем деструкторе.
    // Если мы удалим ее здесь, будет Double Free и программа упадет.

    return passed;
}

int main(int argc, char* argv[]) {
    std::string jsonPath = "tests.json";

    // Проверяем аргументы командной строки
    if (argc > 1) {
        jsonPath = argv[1];
    }

    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << " Cannot open config file: " << jsonPath << std::endl;
        return 1;
    }

    json testsData;
    try {
        testsData = json::parse(f);
    }
    catch (json::parse_error& e) {
        std::cerr << " JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    int passed = 0;
    int total = 0;

    for (const auto& testConfig : testsData) {
        if (RunSingleTest(testConfig)) {
            passed++;
        }
        total++;
    }

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Tests finished. Passed: " << passed << "/" << total << std::endl;

    return (passed == total) ? 0 : 1;
}