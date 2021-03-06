//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const fib_code = R"(block(
    define(fib_test,
        block(
            define(x, 1.0),
            define(z, 0.0),
            define(y, 1.0),
            define(temp, 0.0),
            define(step, 2),
            while(
                step < 10,
                block(
                    store(z, x + y),
                    store(temp, y),
                    store(y, z),
                    store(x, temp),
                    store(step, step + 1)
                )
            ),
            z
        )
    ),
    fib_test
))";

std::map<std::string, std::size_t> expected_counts =
{
    { "access-variable$0", 9 },
    { "access-variable$1", 0 },
    { "access-variable$2", 8 },
    { "access-variable$3", 8 },
    { "access-variable$4", 0 },
    { "access-variable$5", 8 },
    { "access-variable$6", 0 },
    { "access-variable$7", 8 },
    { "access-variable$8", 0 },
    { "access-variable$9", 8 },
    { "access-variable$10", 0 },
    { "access-variable$11", 8 },
    { "access-variable$12", 1 },
    { "access-variable$13", 1 },
    { "__add$0", 8 },
    { "__add$1", 8 },
    { "block$0", 1 },
    { "block$1", 1 },
    { "block$2", 8 },
    { "define-variable$0", 2 },
    { "define-variable$1", 9 },
    { "define-variable$2", 10 },
    { "define-variable$3", 17 },
    { "define-variable$4", 9 },
    { "define-variable$5", 18 },
    { "__lt$0", 9 },
    { "store$0", 8 },
    { "store$1", 8 },
    { "store$2", 8 },
    { "store$3", 8 },
    { "store$4", 8 },
    { "while$0", 1 },
    { "variable$0", 2 },
    { "variable$1", 9 },
    { "variable$2", 10 },
    { "variable$3", 17 },
    { "variable$4", 9 },
    { "variable$5", 18 },
};

const std::vector<std::string> performance_counter_name_last_part{
    "count/eval", "time/eval", "eval_direct"
};

int main()
{
    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const fibonacci = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(fib_code), snippets);

    // Evaluate Fibonacci using the read data
    auto const result = fibonacci();

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    for (auto const& entry :
        phylanx::util::retrieve_counter_data(existing_primitive_instances,
            performance_counter_name_last_part,
            hpx::find_here()))
    {
        auto const tags =
            phylanx::execution_tree::compiler::parse_primitive_name(
                entry.first);

        std::string const expected_key(
            tags.primitive + "$" + std::to_string(tags.sequence_number));
        auto expected_value = expected_counts[expected_key];

        HPX_TEST_EQ(
            entry.second.size(), performance_counter_name_last_part.size());
        HPX_TEST_EQ(entry.second[0], expected_value);
        HPX_TEST_EQ(entry.second[1] != 0, expected_value != 0);
        HPX_TEST(entry.second[2] == -1 || entry.second[2] == 0 ||
            entry.second[2] == 1);
    }

    return hpx::util::report_errors();
}
