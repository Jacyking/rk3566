import numpy as np
from scipy.optimize import curve_fit

def func1(x, a, f):
    return a * x[0] + f

def func2(x, b, f):
    return b * x[1] + f

def func3(x, a, c, f):
    return a * x[0] + c * x[0]**2 + f

def func4(x, b, d, f):
    return b * x[1] + d * x[1]**2 + f

def func5(x, a, b, f):
    return a * x[0] + b * x[1] + f

def func6(x, a, b, c, d, e, f):
    return a * x[0] + b * x[1] + c * x[0]**2 + d * x[1]**2 + e * x[0] * x[1] + f

i_ = 0
popt_ = np.array([])
func_list_ = np.array([func1, func2, func3, func4, func5, func6])

def formula(i, x1, x2, y):
    global i_
    global popt_
    global func_list_
    i_ = i
    popt_, pcov = curve_fit(func_list_[i_], (x1, x2), y)

def density(x):
    global i_
    global popt_
    global func_list_
    return func_list_[i_]((x[0], x[1]), *popt_)
