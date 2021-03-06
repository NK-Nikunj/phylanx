// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
}

///////////////////////////////////////////////////////////////////////////////
void test_len_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile(code)(), compile(expected_str)());
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_len_operation("len( make_list() )", "0");
    test_len_operation("len( make_list(1, 2) )", "2");
    test_len_operation("len( make_list(1, 2, 3) )", "3");
    test_len_operation("len( \"Question of Life, Universe, and Everything\" )", "42");

    return hpx::util::report_errors();
}
