import matplotlib.pyplot as plt
import numpy as np
from sklearn.neural_network import MLPRegressor
from scipy import interpolate

class Plotter:
    """
    Базовый класс плоттеров
    """
    def plot_by_grid(self):
        """
        Отрисовка целевой функции с построением сетки в заданном сечении
        """
        pass
    def plot_approximation(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с использованием аппроксимации
        """
        pass
    def plot_interpolation(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с использованием интерполяции
        """
        pass
    def plot_by_points(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с натягиванием графика на точки
        """
        pass

    def plot_points(self):
        """
        Отрисовка точек поисковых испытаний
        """
        pass

    #@property
    def figure_style_settings_setup(self):
        plt.style.use("bmh") #"fivethirtyeight" / "bmh"
        plt.rcParams['contour.negative_linestyle'] = 'solid'
        plt.rcParams["figure.figsize"] = (8, 6)


    def plot_line(self, x, z, linecolor, linewidth, transparency):
        plt.plot(x, z, color=linecolor, linewidth=linewidth, alpha=transparency)

    def plot_contour(self, x1, x2, z, colormap, linewidths, levels):
        self.ax.contour(x1, x2, z, cmap=colormap, linewidths=linewidths, levels=levels)

    def plot_contourf(self, x1, x2, z, colormap, levels):
        self.ax.contourf(x1, x2, z, cmap=colormap, levels=levels)

    def plot_surface(self, x1, x2, z, colormap, transparency):
        self.ax.plot_surface(x1, x2, z, cmap=colormap, alpha=transparency)

    def plot_line_pulling_on_points(self, x, z, linecolor, linewidth, transparency):
        self.ax.plot(x, z, color=linecolor, linewidth=linewidth, alpha=transparency)

    def plot_contour_pulling_on_points(self, x1, x2, z, colormap, linewidths, levels, transparency):
        self.ax.tricontour(x1, x2, z, cmap=colormap, linewidths=linewidths, levels=levels, alpha=transparency)

    def plot_surface_pulling_on_points(self, x1, x2, z, colormap, transparency):
        self.ax.plot_trisurf(x1, x2, z, cmap=colormap, alpha=transparency)

class Plotter2D(Plotter):
    """
    Плоттер для построения графика зависимости целевой функции от заданного параметра для задач различной размерности
    """
    def __init__(self, parameter_number, left_bound, right_bound):
        self.index = parameter_number
        self.leftBound = left_bound
        self.rightBound = right_bound

        self.figure_style_settings_setup()

        self.fig, self.ax = plt.subplots(1, 1)
        if self.leftBound != None and self.rightBound != None:
            self.ax.set_xlim([self.leftBound, self.rightBound])
        self.ax.tick_params(axis='both', labelsize=8)
        self.ax.set_facecolor('white')

    def plot_by_grid(self, x, z, linecolor='black', linewidth=1, transparency=0.7):
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_approximation(self, points, values, points_count=100, linecolor='black', linewidth=1, transparency=0.7):
        nn = MLPRegressor(activation='logistic',    # tanh, identity, logistic, relu
                          solver='lbfgs',           # lbfgs, sgd , adam
                          alpha=0.001,
                          hidden_layer_sizes=(50,),
                          max_iter=5000,
                          tol=10e-8,
                          random_state=None)

        nn.fit(np.array(points), np.array(values))
        x = np.linspace(self.leftBound, self.rightBound, points_count)
        z = nn.predict(x[:, np.newaxis])
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_interpolation(self, points, values, points_count=100, linecolor='black', linewidth=1, transparency=0.7):
        x = np.linspace(min(np.array(points)), max(np.array(points)), points_count)
        z = interpolate.interp1d(np.array(points).flatten(), np.array(values).flatten(), kind=3)(x)
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_by_points(self, points, values, linecolor='black', linewidth=1, transparency=0.7):
        x, z = zip(*sorted(zip(np.array(points).flatten(), values)))
        self.plot_line_pulling_on_points(x, z, linecolor, linewidth, transparency)

    def plot_points(self, points, values, clr='blue', mrkr='o', mrkrs=4):
        self.ax.scatter(points, values, color=clr, marker=mrkr, s=mrkrs)

class Plotter3D(Plotter):
    """
    Плоттер для построения графика для задач различной размерности
    """
    def __init__(self, parameters_numbers, left_bounds, right_bounds, objective_function,
                 plotter_type, object_function_plotter_type):
        self.indexes = parameters_numbers
        self.leftBounds = left_bounds
        self.rightBounds = right_bounds
        self.objectiveFunction = objective_function

        self.plotterType = plotter_type
        self.objectiveFunctionPlotterType = object_function_plotter_type

        self.figure_style_settings_setup()

        self.fig = plt.Figure()

        if self.plotterType == 'surface':
            self.ax = plt.subplot(projection='3d')
        elif self.plotterType == 'lines layers':
            self.ax = plt.subplot()

        self.ax.set_xlim([self.leftBounds[0], self.rightBounds[0]])
        self.ax.set_ylim([self.leftBounds[1], self.rightBounds[1]])
        self.ax.tick_params(axis='both', labelsize=8)
        self.ax.set_facecolor('white')

    def plot_by_grid(self, x, z, colormap=plt.cm.viridis, linewidths=1, levels=20, transparency=1):
        x1 = [xi[0] for xi in x]
        x2 = [xi[1] for xi in x]

        self.plot_contour_pulling_on_points(x1, x2, z, colormap=colormap, linewidths=linewidths, levels=levels, transparency=transparency)

    def plot_approximation(self, points, values, points_count=100, colormap=plt.cm.viridis, transparency=0.6, linewidths=1, levels=25):
        nn = MLPRegressor(activation='logistic',  # can be tanh, identity, logistic, relu
                          solver='lbfgs',  # can be lbfgs, sgd , adam
                          alpha=0.001,
                          hidden_layer_sizes=(40,),
                          max_iter=10000,
                          tol=10e-6,
                          random_state=10)

        nn.fit(np.array(points)[:, self.indexes], values)
        x1 = np.linspace(self.leftBounds[0], self.rightBounds[0], points_count)
        x2 = np.linspace(self.leftBounds[1], self.rightBounds[1], points_count)
        x1, x2 = np.meshgrid(x1, x2)

        # np.c - cлияние осей X и Y в точки
        # ravel - развернуть (к одномерному массиву)
        xy = np.c_[x1.ravel(), x2.ravel()]

        # делаем предсказание значений
        z = nn.predict(xy)
        z = z.reshape(points_count, points_count)

        # полученная аппроксимация
        #self.plot_surface(x1, x2, z, colormap, transparency)

        if self.plotterType == 'lines layers':
            self.plot_contour(x1, x2, z, colormap=colormap, linewidths=linewidths, levels=levels)
        elif self.plotterType == 'surface':
            self.plot_surface(x1, x2, z, colormap=colormap, transparency=transparency)

    def plot_interpolation(self, points, values, points_count=100,
                           colormap=plt.cm.viridis, transparency=0.6, linewidths=1, levels=25):

        interp = interpolate.Rbf(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values)
        x1 = np.linspace(self.leftBounds[0], self.rightBounds[0], points_count)
        x2 = np.linspace(self.leftBounds[1], self.rightBounds[1], points_count)
        x1, x2 = np.meshgrid(x1, x2)
        z = interp(x1, x2)

        if self.plotterType == 'lines layers':
            self.plot_contour(x1, x2, z, colormap=colormap, linewidths=linewidths, levels=levels)
        elif self.plotterType == 'surface':
            self.plot_surface(x1, x2, z, colormap=colormap, transparency=transparency)

    def plot_by_points(self, points, values, colormap=plt.cm.viridis, transparency=0.9, linewidths=1, levels=25):
        if self.plotterType == 'lines layers':
            self.plot_contour_pulling_on_points(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                                colormap, linewidths, levels, transparency)
        elif self.plotterType == 'surface':
            self.plot_surface_pulling_on_points(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                                colormap, transparency)

    def plot_points(self, points, values, clr='blue', mrkr='o', mrkrs=3):
        if self.plotterType == 'lines layers':
            self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], color=clr, marker=mrkr, s=mrkrs)
        elif self.plotterType == 'surface':
            if self.objectiveFunctionPlotterType == 'by points':
                self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                s=mrkrs, color=clr, marker=mrkr, alpha=0.2)
            else:
                self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                s=mrkrs, color=clr, marker=mrkr, alpha=1.0)
