import sys, os

sys.path.append(os.path.dirname(sys.argv[0]))

from interface_cpp import DrawingProcess

def read_args():
    build_path = str(sys.argv[1])
    trials_file_name = str(sys.argv[2])
    problem_file_name = str(sys.argv[3])
    eps = float(sys.argv[4])
    plot_type = str(sys.argv[5])
    obj_func_type = str(sys.argv[6])

    str_parameters = str(sys.argv[7])
    str_parameters = str_parameters.replace("[", '')
    str_parameters = str_parameters.replace("]", '')
    params = list(map(int, str_parameters.split(",")))

    displacement_of_points = False
    if sys.argv[8].lower() == "true":
        displacement_of_points = True

    output_file_name = str(sys.argv[9])

    figure_show = False
    if sys.argv[10].lower() == "true":
        figure_show = True

    return build_path, trials_file_name, problem_file_name, eps, plot_type, obj_func_type, params, displacement_of_points, output_file_name, figure_show

if __name__ == "__main__":
    print("Start ", str(sys.argv[0]), "...")

    build_path, trials_file_name, problem_file_name, eps, plot_type, obj_func_type, params, displacement_of_points, output_file_name, figure_show = read_args()

    dp = DrawingProcess(build_path, trials_file_name, problem_file_name, eps)

    dp.draw_plot(plotter_type=plot_type,
                 object_function_plotter_type=obj_func_type,
                 parameters_numbers=params,
                 is_points_at_bottom=displacement_of_points,
                 output_file=output_file_name,
                 is_need_show_figure=figure_show
                 )
