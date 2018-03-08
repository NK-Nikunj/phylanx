#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import Phylanx
from phylanx.ast.utils import printout
import numpy as np

et = phylanx.execution_tree
cs = phylanx.compiler_state()

fib10 = et.eval("""
block(
    define(fib,n,
    if(n<2,n,
        fib(n-1)+fib(n-2))),
    fib)""", cs, 10)

assert fib10 == 55.0

sum10 = et.eval("""
block(
    define(sum10,
        block(
            define(i,0),
            define(n,0),
            while(n < 10,
                block(
                  store(n,n+1),
                  store(i,i+n)
                )),
            i)),
    sum10)""",cs)

assert sum10 == 55.0


@Phylanx("PhySL")
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n - 1) + fib(n - 2)


# assert fib.__physl_src__ == \
#     'block$1$0(define$1$0(fib$1$0, n$1$8, ' + \
#     'if(n$2$7 < 2, n$3$15, ' + \
#     '(fib((n$5$19 - 1)) + fib((n$5$28 - 2))))' + \
#     '), fib$1$0)\n'

assert fib(10) == 55.0


@Phylanx("PhySL")
def pass_str(a):
    return a


assert pass_str.__physl_src__ == \
    'define$60$0(pass_str$60$0, a$60$13, a$61$11)', pass_str.__physl_src__

assert "foo" == str(pass_str("foo"))


@Phylanx("PhySL")
def test_slice(a):
    return a[1:3, 1:4]


two = np.reshape(np.arange(30), (5, 6))
r1 = test_slice(two)
r2 = two[1:3, 1:4]

assert (r1 == r2).all()


@Phylanx("PhySL")
def test_slice1(a):
    return a[2:4]


v1 = np.arange(10)
assert (test_slice1(v1) == v1[2:4]).all()


@Phylanx("PhySL")
def foo():
    return 3


assert foo() == 3


@Phylanx("PhySL")
def foo2(n):
    if n == 1:
        return 2
    elif n == 3:
        return 4
    else:
        return 5


assert foo2(1) == 2 and foo2(3) == 4 and foo2(5) == 5


@Phylanx("PhySL")
def foo3():
    sumn = 0
    i = 0
    while i < 10:
        sumn += i
        i += 1
    return sumn


assert foo3() == 45


@Phylanx()
def foo4():
    return foo()


assert foo4() == 3
