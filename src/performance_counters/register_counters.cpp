//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/runtime/startup_function.hpp>
#include <hpx/include/util.hpp>
#include <hpx/include/performance_counters.hpp>

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace performance_counters
{
    ///////////////////////////////////////////////////////////////////////////
    // This function will be registered as a startup function for HPX below.
    // That means it will be executed in an HPX-thread before hpx_main, but
    // after the runtime has been initialized and started.
    void startup()
    {
        // Install the counter types, de-installation of the types is handled
        // automatically.
        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/copy_constructions",
            &ir::node_data<double>::copy_construction_count,
            "returns the current value of the copy-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/move_constructions",
            &ir::node_data<double>::move_construction_count,
            "returns the current value of the move-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/copy_assignments",
            &ir::node_data<double>::copy_assignment_count,
            "returns the current value of the copy-assignment count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/move_assignments",
            &ir::node_data<double>::move_assignment_count,
            "returns the current value of the move-assignment count of "
                "any node_data<double>");
    }

    ///////////////////////////////////////////////////////////////////////////
    bool get_startup(hpx::startup_function_type& startup_func,
        bool& pre_startup)
    {
        // return our startup-function if performance counters are required
        startup_func = startup;   // function to run during startup
        pre_startup = true;       // run 'startup' as pre-startup function
        return true;
    }
}}

///////////////////////////////////////////////////////////////////////////////
// Register a startup function that will be called as a HPX-thread during
// runtime startup. We use this function to register our performance counter
// type and performance counter instances.
HPX_REGISTER_STARTUP_MODULE(phylanx::performance_counters::get_startup);

