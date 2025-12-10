#include "Globalizer.h"

#include <vector>
#include <string>
#include <iostream>

#include <filesystem>

#ifdef USE_PYTHON
#include <Python.h>
#endif

#ifdef USE_PYTHON
void make_problem_info_file(IProblem* problem, SolutionResult* result, const char* problem_file_name = "_problem_info.txt",
    Plotter::CalcsTypes calcs_type = Plotter::ObjectiveFunction, std::initializer_list<int> params = { 0, 1 }, int points_count = 100) {

    FILE* pf = fopen(problem_file_name, "w+");

    if (!pf) {
        throw std::logic_error("Error: cant open file");
    }

    fprintf(pf, "%d ", problem->GetDimension());

    double* left_bound = new double[problem->GetDimension()];
    double* right_bound = new double[problem->GetDimension()];
    problem->GetBounds(left_bound, right_bound);

    for (int i = 0; i < problem->GetDimension() - 1; i++) {
        fprintf(pf, "%lf_", left_bound[i]);
    }
    fprintf(pf, "%lf ", left_bound[problem->GetDimension() - 1]);
    for (int i = 0; i < problem->GetDimension() - 1; i++) {
        fprintf(pf, "%lf_", right_bound[i]);
    }
    fprintf(pf, "%lf\n", right_bound[problem->GetDimension() - 1]);

    if (calcs_type == Plotter::ObjectiveFunction) {
        double* section = (result->BestTrial)->y;
        double* x = new double[problem->GetDimension()];
        double z;

        for (int i = 0; i < problem->GetDimension(); i++) {
            x[i] = section[i];
        }

        double* dx = new double[params.size()];
        for (int i = 0; i < params.size(); i++) {
            dx[i] = fabs(right_bound[*(params.begin() + i)] - left_bound[*(params.begin() + i)]) / points_count;
        }

        if (params.size() == 2) {
            for (int k = 0; k < points_count; k++) {
                x[*(params.begin() + 0)] = left_bound[*(params.begin() + 0)] + k * dx[0];
                for (int j = 0; j < points_count; j++) {
                    x[*(params.begin() + 1)] = left_bound[*(params.begin() + 1)] + j * dx[1];

                    for (int i = 0; i < problem->GetDimension(); i++) {
                        fprintf(pf, "%lf ", x[i]);
                    }

                    for (int i = 0; i < problem->GetNumberOfFunctions(); i++) {
                        z = problem->CalculateFunctionals(x, i);
                        fprintf(pf, "| %lf ", z);
                    }
                    fprintf(pf, "\n");
                }
            }
        } else if (params.size() == 1) {
            for (int k = 0; k < points_count; k++) {
                x[*(params.begin() + 0)] = left_bound[*(params.begin() + 0)] + k * dx[0];
                for (int i = 0; i < problem->GetDimension(); i++) {
                    fprintf(pf, "%lf ", x[i]);
                }

                for (int i = 0; i < problem->GetNumberOfFunctions(); i++) {
                    z = problem->CalculateFunctionals(x, i);
                    fprintf(pf, "| %lf ", z);
                }
                fprintf(pf, "\n");
            }
        }
        delete[] x;
        delete[] dx;
    }

    delete[] left_bound;
    delete[] right_bound;

    fclose(pf);
}
#endif

#ifdef USE_PYTHON
void Plotter::draw_plot(IProblem* problem, SolutionResult* result, std::initializer_list<int> params,
    wchar_t* output_file_name, FigureTypes figure_type, CalcsTypes calcs_type,
    bool show_figure, bool move_points_under_graph) {

    if (parameters.libPath.ToString() == "iOptProblem.dll" || params.size() == 1) {
        printf("TBD...");
        return;
    }

    if (problem == nullptr) {
        throw std::invalid_argument("Error: problem is not init");
    }

    if (result == nullptr) {
        throw std::invalid_argument("Error: solution is not init");
    }

    if (params.size() > 2 || params.size() == 0) {
        throw std::invalid_argument("Error: the number of parameters must be 1 or 2");
    }

    for (const int* param = params.begin(); param != params.end(); param++) {
        if (*param >= problem->GetDimension() || *param < 0) {
            throw std::invalid_argument("Error: invalid parameter value");
        }
    }

    if (params.size() == 2 && (*(params.begin() + 0) == *(params.begin() + 1))) {
        throw std::invalid_argument("Error: parameters cannot match");
    }

    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    std::filesystem::path build_path = std::filesystem::absolute(".\\");
    std::filesystem::path script_path = std::filesystem::absolute("..\\lib\\plotters\\start_cpp.py");

    std::string solver_output_file = parameters.iterPointsSavePath.ToString();
    std::string problem_info_file = "_problem_info.txt";

    std::wstring _params = L"[";
    _params += std::to_wstring(*(params.begin()));
    if (params.size() > 1) {
        _params += L", ";
        _params += std::to_wstring(*(params.begin() + 1));
    }
    _params += L"]";

    std::wstring _solver_output_file = std::wstring(solver_output_file.begin(), solver_output_file.end());
    std::wstring _problem_info_file = std::wstring(problem_info_file.begin(), problem_info_file.end());
    std::wstring _eps = std::to_wstring(parameters.Epsilon);

    make_problem_info_file(problem, result, problem_info_file.c_str(), calcs_type, params, 100);

    wchar_t* __build_path = _wcsdup(build_path.wstring().c_str());
    wchar_t* __script_path = _wcsdup(script_path.wstring().c_str());
    wchar_t* __params = _wcsdup(_params.c_str());

    wchar_t* __solver_output_file = _solver_output_file.data();
    wchar_t* __problem_info_file = _problem_info_file.data();
    wchar_t* __eps = _eps.data();

    wchar_t* __show_figure = show_figure ? L"True" : L"False";
    wchar_t* __move_points_under_graph = move_points_under_graph ? L"True" : L"False";

    wchar_t* __figure_type = figure_type == Plotter::LevelLayers ? L"lines layers" : L"surface";
    wchar_t* __calcs_type = calcs_type == Plotter::ObjectiveFunction ? L"objective function" :
        calcs_type == Plotter::Approximation ? L"approximation" :
        calcs_type == Plotter::Interpolation ? L"interpolation" :
        calcs_type == Plotter::ByPoints ? L"by points" :
        L"only points";

    wchar_t* args[] = {
            __script_path,
            __build_path,
            __solver_output_file,
            __problem_info_file,
            __eps,
            __figure_type,
            __calcs_type,
            __params,
            __move_points_under_graph,
            output_file_name,
            __show_figure,
            NULL
    };

    PySys_SetArgvEx(sizeof(args) / sizeof(wchar_t*) - 1, args, 0);


    FILE* file = nullptr;

    if ((file = _Py_fopen(script_path.string().c_str(), "r")) == NULL) {
        std::cout << "Python script wasn\'t opened!\n";
    } else {
        std::cout << "The Python charting script has been opened...\n";
        std::cout << "The Python charting script has been launched...\n";

        if (PyRun_SimpleFile(file, script_path.string().c_str()) != 0)
            std::cout << "An unknown error occurred while executing the script...\n";

        std::cout << "The Python charting script has terminated.\n";

    }

    if (Py_IsInitialized()) {
        Py_Finalize();
    }


    delete __build_path;
    delete __script_path;
    delete __params;

}
#endif