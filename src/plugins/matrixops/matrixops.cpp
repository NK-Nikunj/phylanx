//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/matrixops/matrixops.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <string>

PHYLANX_REGISTER_PLUGIN_MODULE();

PHYLANX_REGISTER_PLUGIN_FACTORY(add_dimension_plugin,
    phylanx::execution_tree::primitives::add_dimension::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    argmax_plugin, phylanx::execution_tree::primitives::argmax::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    argmin_plugin, phylanx::execution_tree::primitives::argmin::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(column_set_operation_plugin,
    phylanx::execution_tree::primitives::column_set_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(column_slicing_operation_plugin,
    phylanx::execution_tree::primitives::slicing_operation::match_data[2]);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    constant_plugin, phylanx::execution_tree::primitives::constant::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(cross_operation_plugin,
    phylanx::execution_tree::primitives::cross_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(determinant_plugin,
    phylanx::execution_tree::primitives::determinant::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(diag_operation_plugin,
    phylanx::execution_tree::primitives::diag_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(dot_operation_plugin,
    phylanx::execution_tree::primitives::dot_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(extract_shape_plugin,
    phylanx::execution_tree::primitives::extract_shape::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(gradient_operation_plugin,
    phylanx::execution_tree::primitives::gradient_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(hstack_operation_plugin,
    phylanx::execution_tree::primitives::hstack_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    identity_plugin, phylanx::execution_tree::primitives::identity::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(inverse_operation_plugin,
    phylanx::execution_tree::primitives::inverse_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(linearmatrix_plugin,
    phylanx::execution_tree::primitives::linearmatrix::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    linspace_plugin, phylanx::execution_tree::primitives::linspace::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(mean_operation_plugin,
    phylanx::execution_tree::primitives::mean_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(power_operation_plugin,
    phylanx::execution_tree::primitives::power_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(
    random_plugin, phylanx::execution_tree::primitives::random::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(row_set_operation_plugin,
    phylanx::execution_tree::primitives::row_set_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(row_slicing_operation_plugin,
    phylanx::execution_tree::primitives::slicing_operation::match_data[1]);
PHYLANX_REGISTER_PLUGIN_FACTORY(set_operation_plugin,
    phylanx::execution_tree::primitives::set_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(shuffle_operation_plugin,
    phylanx::execution_tree::primitives::shuffle_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(slicing_operation_plugin,
    phylanx::execution_tree::primitives::slicing_operation::match_data[0]);
PHYLANX_REGISTER_PLUGIN_FACTORY(square_root_operation_plugin,
    phylanx::execution_tree::primitives::square_root_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(sum_operation_plugin,
    phylanx::execution_tree::primitives::sum_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(transpose_operation_plugin,
    phylanx::execution_tree::primitives::transpose_operation::match_data);
PHYLANX_REGISTER_PLUGIN_FACTORY(vstack_operation_plugin,
    phylanx::execution_tree::primitives::vstack_operation::match_data);

PHYLANX_REGISTER_PLUGIN_FACTORY(get_seed,
    phylanx::execution_tree::primitives::get_seed_match_data,
    "get_seed_action");
PHYLANX_REGISTER_PLUGIN_FACTORY(set_seed,
    phylanx::execution_tree::primitives::set_seed_match_data,
    "set_seed_action");

namespace phylanx {
namespace plugin {
    struct generic_operation_plugin : plugin_base
    {
        void register_known_primitives() override
        {
            namespace pet = phylanx::execution_tree;

            std::string generic_operation_name("__gen");
            for (auto const& pattern :
                pet::primitives::generic_operation::match_data)
            {
                pet::register_pattern(generic_operation_name, pattern);
            }
        }
    };
}
}

PHYLANX_REGISTER_PLUGIN_FACTORY(phylanx::plugin::generic_operation_plugin,
    generic_operation_plugin,
    phylanx::execution_tree::primitives::make_list::match_data, "__gen");
