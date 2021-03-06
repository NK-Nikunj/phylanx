#  Copyright (c) 2018 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *
from phylanx.util import prange

# @Phylanx(debug=True)
# def test_prange():
#     for x in prange(0,10):
#         print (x)

# test_prange()


@Phylanx(debug=True)
def test_prange_list():
    arr = [0, 0, 0, 0, 0]
    arrlen = 5

    # TODO: add 'len' support
    # arrlen = len(arr)
    for i in prange(0, arrlen):
        arr[i] = i


test_prange_list()
