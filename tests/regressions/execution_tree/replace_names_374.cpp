// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #374: Redefining a PhySL function in Python is not possible

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const code = R"(
    define(ultimate_answer, 42)
    debug(ultimate_answer)
    define(ultimate_answer, block(
        debug("'42'")
    ))
    ultimate_answer()
)";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto ultimate_answer = phylanx::execution_tree::compile("code", code, snippets);
    ultimate_answer();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("42\n'42'\n"));

    return hpx::util::report_errors();
}
