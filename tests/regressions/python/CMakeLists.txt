# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    array_shape_486
    broadcasting_rules_410
    create_list_409
    exception_swallowed_369
    list_iter_space_429
    name_constants_411
    no_parans_decorator_407
    passing_compiler_state_453
    zero_dimensional_array_502
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_regression_test("python" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Regression"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}")

  add_phylanx_pseudo_target(tests.regressions.python_dir.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.regressions.python_dir tests.regressions.python_dir.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.regressions.python_dir.${test}_py ${test}_test_py)

endforeach()
