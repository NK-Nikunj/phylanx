# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


# This function is never called, it just makes Flake 8 quiet
def shape(a, b):
    pass


@Phylanx()
def kernel(a, b):
    for i in range(1, shape(a, 0) - 1):
        a[i] = b[i + 1] + b[i - 1]
    return a


try:

    # Quiet Flake8
    class numpy:
        pass

    @Phylanx()
    def bad_kernel(a):
        return numpy.shape()

    raise Exception("Should fail because numpy.shape doesn't exist")

except LookupError:
    pass

bv = np.linspace(0, 10, 11)
av = np.zeros(len(bv))
bsum = (bv[2:] + bv[:-2])
bsum2 = kernel(av, bv)
assert np.all(bsum2[1:-1] == bsum)


@Phylanx()
def kernel2(a, b):
    a[1:-1] = b[2:] + b[:-2]
    return a


bsum3 = kernel2(av, bv)
assert np.all(bsum3[1:-1] == bsum)

ma = np.linspace(1, 12, 12).reshape((3, 4))
mr2 = np.linspace(1, 12, 12).reshape((3, 4))
mb = np.linspace(1, 12, 12).reshape((3, 4))


@Phylanx()
def kernel3(a, b):
    for i in range(1, shape(a, 0) - 1):
        for j in range(1, shape(a, 1) - 1):
            a[i, j] = b[i, j + 1] + b[i, j - 1]
    for i in range(1, shape(a, 0) - 1):
        for j in range(1, shape(a, 1) - 1):
            a[i, j] = a[i, j] + b[i + 1, j] + b[i - 1, j]
    return a


mr2[1:-1, 1:-1] = mb[2:, 1:-1] + mb[:-2, 1:-1] + mb[1:-1, 2:] + mb[1:-1, :-2]
mr = kernel3(ma, mb)
assert np.all(mr == mr2)

ma = np.linspace(1, 12, 12).reshape((3, 4))
mr2 = np.linspace(1, 12, 12).reshape((3, 4))
mb = np.linspace(1, 12, 12).reshape((3, 4))


@Phylanx()
def kernel4(a, b):
    a[1:-1, 1:-1] = b[2:, 1:-1] + b[:-2, 1:-1] + b[1:-1, 2:] + b[1:-1, :-2]
    return a


mr2[1:-1, 1:-1] = mb[2:, 1:-1] + mb[:-2, 1:-1] + mb[1:-1, 2:] + mb[1:-1, :-2]
mr = kernel4(ma, mb)
assert np.all(mr == mr2)


@Phylanx()
def kernel5(b):
    return b[2:, 1:-1] + b[:-2, 1:-1] + b[1:-1, 2:] + b[1:-1, :-2]


mr = kernel5(mb)
assert np.all(mr == mr2[1:-1, 1:-1])
