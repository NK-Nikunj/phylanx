// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/add_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        struct add_simd
        {
        public:
            explicit add_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
                -> decltype(a + std::declval<double>())
            {
                return a + scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDAdd<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a + blaze::set(scalar_);
            }

        private:
            double scalar_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const add_operation::match_data =
    {
        hpx::util::make_tuple("__add",
            std::vector<std::string>{"_1 + __2", "__add(_1, __2)"},
            &create_add_operation, &create_primitive<add_operation>)
    };

    //////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add0d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        lhs.scalar() += rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add0d0d(args_type && args) const
    {
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                result.scalar() += curr.scalar();
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add0d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() =
                blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() = blaze::map(
                rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add0d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add1d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() =
                blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add1d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        // Broadcasting rule 1: Dimensions are identical
        if (lhs_size == rhs_size)
        {
            // Avoid overwriting references, avoid memory reallocation when possible
            if (lhs.is_ref())
            {
                // Cannot reuse the memory if an operand is a reference
                if (rhs.is_ref())
                {
                    rhs = lhs.vector() + rhs.vector();
                }
                // Reuse the memory from rhs operand
                else
                {
                    rhs.vector() = lhs.vector() + rhs.vector();
                }
                return primitive_argument_type(std::move(rhs));
            }
            // Reuse the memory from lhs operand
            else
            {
                lhs.vector() += rhs.vector();
            }
            return primitive_argument_type(std::move(lhs));
        }
        // Broadcasting rule 2: One or two of the operand dimensions equal one
        else
        {
            if (lhs_size == 1)
            {
                if (rhs.is_ref())
                {
                    rhs = blaze::map(
                        rhs.vector(), detail::add_simd(lhs.vector()[0]));
                }
                else
                {
                    rhs.vector() = blaze::map(
                        rhs.vector(), detail::add_simd(lhs.vector()[0]));
                }
                return primitive_argument_type(std::move(rhs));
            }
            else if (rhs_size == 1)
            {
                if (lhs.is_ref())
                {
                    lhs = blaze::map(
                        lhs.vector(), detail::add_simd(rhs.vector()[0]));
                }
                else
                {
                    lhs.vector() += blaze::map(
                        lhs.vector(), detail::add_simd(rhs.vector()[0]));
                }
                return primitive_argument_type(std::move(lhs));
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add1d1d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match",
                name_, codename_));
    }

    primitive_argument_type add_operation::add1d1d(args_type && args) const
    {
        std::size_t const operand_size = args[0].dimension(0);
        for (auto const& i : args)
        {
            if (i.dimension(0) != operand_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "add_operation::add1d1d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match",
                        name_, codename_));
            }
        }

        return primitive_argument_type(std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.vector() + curr.vector();
                }
                else
                {
                    result.vector() += curr.vector();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type add_operation::add1d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_v = lhs.vector();
        auto rhs_m = rhs.matrix();

        // If dimensions match
        if (lhs_v.size() == rhs_m.columns())
        {
            if (rhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    rhs_m.rows(), rhs_m.columns()};
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(rhs_m, i) + blaze::trans(lhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != rhs_m.rows(); ++i)
                {
                    blaze::row(rhs_m, i) += blaze::trans(lhs_v);
                }

                return primitive_argument_type{std::move(rhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (lhs_v.size() == 1)
        {
            if (rhs.is_ref())
            {
                rhs = blaze::map(rhs_m, detail::add_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
            else
            {
                rhs_m = blaze::map(rhs_m, detail::add_simd(lhs_v[0]));

                return primitive_argument_type{std::move(rhs)};
            }
        }
        // If the matrix has only one column
        else if (rhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_v.size());

            // Replicate first and only column of rhs matrix
            for (std::size_t i = 0; i < result.columns(); ++i)
            {
                blaze::column(result, i) = blaze::column(rhs_m, 0);
            }

            // Replicate lhs vector
            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) += blaze::trans(lhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add1d2d",
            execution_tree::generate_error_message(
                "vector size does not match number of matrix columns",
                name_, codename_));
    }

    primitive_argument_type add_operation::add1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();

        switch (rhs_dims)
        {
        case 0:
            return add1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add1d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();

        switch (rhs_dims)
        {
        case 1:
            return add1d1d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add2d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_v = rhs.vector();

        // If dimensions match
        if (rhs_v.size() == lhs_m.columns())
        {
            if (lhs.is_ref())
            {
                blaze::DynamicMatrix<double> result{
                    lhs_m.rows(), lhs_m.columns()};
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(result, i) =
                        blaze::row(lhs_m, i) + blaze::trans(rhs_v);
                }
                return primitive_argument_type{std::move(result)};
            }
            else
            {
                for (std::size_t i = 0; i != lhs_m.rows(); ++i)
                {
                    blaze::row(lhs_m, i) += blaze::trans(rhs_v);
                }

                return primitive_argument_type{std::move(lhs)};
            }
        }
        // If the vector is effectively a scalar
        else if (rhs_v.size() == 1)
        {
            if (lhs.is_ref())
            {
                lhs = blaze::map(lhs_m, detail::add_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
            else
            {
                lhs_m = blaze::map(lhs_m, detail::add_simd(rhs_v[0]));

                return primitive_argument_type{std::move(lhs)};
            }
        }
        // If the matrix has only one column
        else if (lhs_m.columns() == 1)
        {
            blaze::DynamicMatrix<double> result(lhs_m.rows(), rhs_v.size());

            for (std::size_t i = 0; i< result.columns(); ++i)
            {
                blaze::column(result, i) = blaze::column(lhs_m, 0);
            }

            for (std::size_t i = 0; i < result.rows(); ++i)
            {
                blaze::row(result, i) += blaze::trans(rhs_v);
            }

            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d1d",
            execution_tree::generate_error_message(
                "vector size does not match either the number of matrix "
                "columns nor rows.",
                name_, codename_));
    }

    add_operation::stretch_operand add_operation::get_stretch_dimension(
        std::size_t lhs, std::size_t rhs) const
    {
        // 0: No copy, 1: stretch lhs, 2: stretch rhs
        if (lhs != rhs)
        {
            // lhs x dimension must be stretched
            if (lhs == 1)
            {
                return stretch_operand::lhs;
            }
            // rhs x dimensions must be stretched
            else if (rhs == 1)
            {
                return stretch_operand::rhs;
            }
        }
        return stretch_operand::neither;
    }

    primitive_argument_type add_operation::add2d2d_no_stretch(
        arg_type&& lhs, arg_type&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = lhs.matrix() + rhs.matrix();
            }
            // Reuse the memory from rhs operand
            else
            {
                rhs.matrix() = lhs.matrix() + rhs.matrix();
            }
            return primitive_argument_type(std::move(rhs));
        }
        // Reuse the memory from lhs operand
        else
        {
            lhs.matrix() += rhs.matrix();
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d2d_lhs_both(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(rhs_m, detail::add_simd(lhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(rhs_m, detail::add_simd(lhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::add2d2d_rhs_both(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs_m, detail::add_simd(rhs_m(0, 0)));
        }
        else
        {
            lhs.matrix() = blaze::map(lhs_m, detail::add_simd(rhs_m(0, 0)));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::add2d2d_lhs_row_rhs_col(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        blaze::DynamicMatrix<double> result(rhs_m.rows(), lhs_m.columns());
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            blaze::row(result, i) = blaze::row(lhs_m, 0);
        }
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            blaze::column(result, i) += blaze::column(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type add_operation::add2d2d_lhs_row(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Ensure we can reuse memory
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                rhs.dimension(0), rhs.dimension(1));

            for (std::size_t i = 0; i != result.rows(); ++i)
            {
                blaze::row(result, i) =
                    blaze::row(lhs_m, 0) + blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(0); ++i)
            {
                blaze::row(rhs_m, i) =
                    blaze::row(lhs_m, 0) + blaze::row(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type add_operation::add2d2d_lhs_col_rhs_row(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        blaze::DynamicMatrix<double> result(lhs_m.rows(), rhs_m.columns());
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            blaze::column(result, i) = blaze::column(lhs_m, 0);
        }
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            blaze::row(result, i) += blaze::row(rhs_m, 0);
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type add_operation::add2d2d_rhs_row(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Ensure we can reuse memory
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                lhs.dimension(0), lhs.dimension(1));

            for (std::size_t i = 0; i != result.rows(); ++i)
            {
                blaze::row(result, i) =
                    blaze::row(lhs_m, i) + blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(0); ++i)
            {
                blaze::row(lhs_m, i) =
                    blaze::row(lhs_m, i) + blaze::row(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type add_operation::add2d2d_lhs_col(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Avoid overwriting references, avoid memory
        // reallocation when possible
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                rhs.dimension(0), rhs.dimension(1));

            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                blaze::column(result, i) =
                    blaze::column(lhs_m, 0) + blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != rhs.dimension(1); ++i)
            {
                blaze::column(rhs_m, i) =
                    blaze::column(lhs_m, 0) + blaze::column(rhs_m, i);
            }
            return primitive_argument_type{std::move(rhs)};
        }
    }

    primitive_argument_type add_operation::add2d2d_rhs_col(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_m = lhs.matrix();
        auto rhs_m = rhs.matrix();

        // Avoid overwriting references, avoid memory
        // reallocation when possible
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> result(
                lhs.dimension(0), lhs.dimension(1));

            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                blaze::column(result, i) =
                    blaze::column(lhs_m, i) + blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(result)};
        }
        else
        {
            for (std::size_t i = 0; i != lhs.dimension(1); ++i)
            {
                blaze::column(lhs_m, i) =
                    blaze::column(lhs_m, i) + blaze::column(rhs_m, 0);
            }
            return primitive_argument_type{std::move(lhs)};
        }
    }

    primitive_argument_type add_operation::add2d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        // Dimensions are identical
        if (lhs_size == rhs_size)
        {
            return add2d2d_no_stretch(std::move(lhs), std::move(rhs));
        }
        // Check if broadcasting rules apply
        else
        {
            // Number of rows do not match
            stretch_operand stretch_rows =
                get_stretch_dimension(lhs_size[0], rhs_size[0]);
            // Number of columns do not match
            stretch_operand stretch_cols =
                get_stretch_dimension(lhs_size[1], rhs_size[1]);

            // Shorthand
            auto lhs_m = lhs.matrix();
            auto rhs_m = rhs.matrix();

            // If lhs rows needs stretching
            if (stretch_rows == stretch_operand::lhs)
            {
                // If both lhs axes need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return add2d2d_lhs_both(std::move(lhs), std::move(rhs));
                }
                // If lhs rows and rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return add2d2d_lhs_row_rhs_col(std::move(lhs), std::move(rhs));
                }
                // If only lhs rows need stretching
                else
                {
                    return add2d2d_lhs_row(std::move(lhs), std::move(rhs));
                }
            }
            // If lhs cols needs stretching
            else if (stretch_rows == stretch_operand::rhs)
            {
                // If lhs cols and rhs rows need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return add2d2d_lhs_col_rhs_row(std::move(lhs), std::move(rhs));
                }
                // If both rhs axes need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return add2d2d_rhs_both(std::move(lhs), std::move(rhs));
                }
                // If only rhs rows need stretching
                else
                {
                    return add2d2d_rhs_row(std::move(lhs), std::move(rhs));
                }
            }
            else
            {
                // If only lhs cols need stretching
                if (stretch_cols == stretch_operand::lhs)
                {
                    return add2d2d_lhs_col(std::move(lhs), std::move(rhs));
                }
                // If only rhs cols need stretching
                else if (stretch_cols == stretch_operand::rhs)
                {
                    return add2d2d_rhs_col(std::move(lhs), std::move(rhs));
                }
                // Otherwise no axis can be stretched
            }
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::add2d2d",
            execution_tree::generate_error_message(
                "the dimensions of the operands do not match",
                name_, codename_));
    }

    primitive_argument_type add_operation::add2d2d(args_type && args) const
    {
        auto const operand_size = args[0].dimensions();
        for (auto const& i : args)
        {
            if (i.dimensions() != operand_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "add_operation::add2d2d",
                    execution_tree::generate_error_message(
                        "the dimensions of the operands do not match",
                        name_, codename_));
            }
        }

        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.matrix() + curr.matrix();
                }
                else
                {
                    result.matrix() += curr.matrix();
                }
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add2d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 2:
            return add2d2d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    void add_operation::append_element(
        std::vector<primitive_argument_type>& result,
        primitive_argument_type&& rhs) const
    {
        if (is_list_operand_strict(rhs))
        {
            auto&& rhs_list =
                extract_list_value_strict(std::move(rhs), name_, codename_);

            for (auto&& elem : std::move(rhs_list))
            {
                result.emplace_back(std::move(elem));
            }
        }
        else
        {
            result.emplace_back(std::move(rhs));
        }
    }

    primitive_argument_type add_operation::handle_list_operands(
        primitive_argument_type&& op1, primitive_argument_type&& rhs) const
    {
        ir::range lhs =
            extract_list_value_strict(std::move(op1), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            append_element(result, std::move(rhs));
            return primitive_argument_type{std::move(result)};
        }

        append_element(lhs.args(), std::move(rhs));
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::handle_list_operands(
        std::vector<primitive_argument_type>&& ops) const
    {
        auto it = ops.begin();
        auto end = ops.end();

        ir::range lhs =
            extract_list_value_strict(std::move(ops[0]), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            for (++it; it != end; ++it)
            {
                append_element(result, std::move(*it));
            }
            return primitive_argument_type{std::move(result)};
        }

        for (++it; it != end; ++it)
        {
            append_element(lhs.args(), std::move(*it));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::handle_numeric_operands(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const
    {
        arg_type lhs = extract_numeric_value(std::move(op1), name_, codename_);
        arg_type rhs = extract_numeric_value(std::move(op2), name_, codename_);

        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return add0d(std::move(lhs), std::move(rhs));

        case 1:
            return add1d(std::move(lhs), std::move(rhs));

        case 2:
            return add2d(std::move(lhs), std::move(rhs));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::handle_numeric_operands",
            generate_error_message(
                "left hand side operand has unsupported number of dimensions"));
    }

    primitive_argument_type add_operation::handle_numeric_operands(
        std::vector<primitive_argument_type>&& ops) const
    {
        args_type args;
        args.reserve(ops.size());

        for (auto && op : std::move(ops))
        {
            args.emplace_back(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        std::size_t lhs_dims = args[0].num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return add0d(std::move(args));

        case 1:
            return add1d(std::move(args));

        case 2:
            return add2d(std::move(args));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::handle_numeric_operands",
            generate_error_message(
                "left hand side operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message("the add_operation primitive requires "
                    "at least two operands"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message(
                    "the add_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            // special case for 2 operands
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](primitive_argument_type&& lhs,
                        primitive_argument_type&& rhs)
                -> primitive_argument_type
                {
                    if (is_list_operand_strict(lhs))
                    {
                        return this_->handle_list_operands(
                            std::move(lhs), std::move(rhs));
                    }
                    return this_->handle_numeric_operands(
                        std::move(lhs), std::move(rhs));
                }),
                value_operand(operands[0], args, name_, codename_),
                value_operand(operands[1], args, name_, codename_));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& ops)
            ->  primitive_argument_type
            {
                if (is_list_operand_strict(ops[0]))
                {
                    return this_->handle_list_operands(std::move(ops));
                }
                return this_->handle_numeric_operands(std::move(ops));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
