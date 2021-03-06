// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PARALLEL_MAP_OPERATION_APR_26_2018_0624PM)
#define PHYLANX_PARALLEL_MAP_OPERATION_APR_26_2018_0624PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class parallel_map_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<parallel_map_operation>
    {
    public:
        static match_pattern_type const match_data;

        parallel_map_operation() = default;

        parallel_map_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        hpx::future<primitive_argument_type> map_1(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args,
            primitive const* p) const;
        hpx::future<primitive_argument_type> map_n(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args,
            primitive const* p) const;
    };

    inline primitive create_parallel_map_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "parallel_map", std::move(operands), name, codename);
    }
}}}

#endif
