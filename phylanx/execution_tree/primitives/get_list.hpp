//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_get_list_FEB_25_2018_1030AM)
#define PHYLANX_get_list_FEB_25_2018_1030AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class get_list
      : public primitive_component_base
      , public std::enable_shared_from_this<get_list>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

    public:
        static match_pattern_type const match_data;

        get_list() = default;

        ///
        /// \brief Creates a PhySL list by concatenating its arguments
        ///
        /// \param args Is a (possibly empty) list of any values to be
        ///             concatenated into a PhySL list in order.
        ///
        get_list(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };

    PHYLANX_EXPORT primitive create_get_list(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif
