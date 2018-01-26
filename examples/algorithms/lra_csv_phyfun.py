#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import phylanx
et = phylanx.execution_tree
from phylanx.util import *

@phyfun
def lra(file_name,
        xlo1, xhi1, ylo1, yhi1,
        xlo2, xhi2, ylo2, yhi2,
        alpha, iterations, enable_output):
    data = file_read_csv(file_name)
    x = slice(data,xlo1,xhi1,ylo1,yhi1)
    y = slice(data,xlo2,xhi2,ylo2,yhi2)
    weights=constant(0.0, shape(x, 1))
    transx=transpose(x)
    pred=constant(0.0, shape(x, 0))
    error=constant(0.0, shape(x, 0))
    gradient=constant(0.0, shape(x, 1))
    step=0
    while step < iterations:
        if enable_output:
            print("step: ", step, ", ", weights)
        pred=1.0 / (1.0 + exp(-dot(x, weights)))
        error=pred - y
        gradient=dot(transx, error)
        weights=weights - (alpha * gradient)
        step += 1
    return weights

res = lra("breast_cancer.csv", 0,569,0,30, 0,569,30,31, 1e-5,750,0)
phy_print(res)
