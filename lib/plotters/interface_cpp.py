from ctypes import *
import sys, os
sys.path.append(os.path.dirname(sys.argv[0]))

from painters import StaticPainter
from file_readers import ReadTrialsFile, ReadProblemFile

class DrawingProcess:
    def __init__(self,  build_path, trials_file_name, problem_file_name, eps):
        self.path = build_path
        self.eps = eps
        self.dim, self.lb, self.rb, self.x, self.z, self.c = ReadProblemFile(build_path, problem_file_name)
        self.points, self.values, self.sol_point, self.sol_value, self.x_nc, self.z_nc, self.cc = ReadTrialsFile(build_path, trials_file_name)

    def draw_plot(self,
                  plotter_type=None,
                  object_function_plotter_type="objective function",
                  parameters_numbers=[0],
                  is_points_at_bottom=True,
                  output_file="output.png",
                  is_need_show_figure=False):
        '''
        plotter_type: 'lines layers' 'surface'
        object_function_plotter_type: 'objective function' 'approximation' 'interpolation' 'by points' 'only points'
        Comments:
        'lines layers' + 'approximation', 'lines layers' + 'by points'- не желательно
        'surface' + 'objective function' - визуализирует в объеме линии уровня вместо поверхности
        '''
        painter = StaticPainter(parameters_numbers,
                                self.eps,
                                self.dim,
                                self.lb,
                                self.rb,
                                self.points,
                                self.values,
                                self.sol_point,
                                self.sol_value,
                                self.x,
                                self.z,
                                self.c,
                                self.x_nc,
                                self.z_nc,
                                self.cc,
                                plotter_type,
                                object_function_plotter_type,
                                is_points_at_bottom)
        painter.paint_objective_func()
        painter.paint_constraints()
        painter.paint_points()
        painter.paint_optimum()
        painter.save_image(self.path, output_file, is_need_show_figure)
