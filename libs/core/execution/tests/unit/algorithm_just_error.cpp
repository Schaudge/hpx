//  Copyright (c) 2021 ETH Zurich
//  Copyright (c) 2022 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if defined(HPX_HAVE_STDEXEC)
#include <hpx/execution/algorithms/just.hpp>
#else
#include <hpx/modules/execution.hpp>
#endif

#include <hpx/modules/testing.hpp>

#include "algorithm_test_utils.hpp"

#include <atomic>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>

namespace ex = hpx::execution::experimental;

int main()
{
    {
        std::atomic<bool> set_error_called{false};
        auto s = ex::just_error(std::runtime_error("error"));

        static_assert(ex::is_sender_v<decltype(s)>);
#if defined(HPX_HAVE_STDEXEC)
        static_assert(ex::is_sender_in_v<decltype(s), ex::empty_env>);
#else
        static_assert(ex::is_sender_v<decltype(s), ex::empty_env>);
#endif

        check_value_types<hpx::variant<>>(s);
#if defined(HPX_HAVE_STDEXEC)
        // no longer throws exceptions in the form std::exception_ptr in STDEXEC
        check_error_types<hpx::variant<std::runtime_error>>(s);
#else
        check_error_types<hpx::variant<std::exception_ptr, std::runtime_error>>(
            s);
#endif
        check_sends_stopped<false>(s);

        auto r = error_callback_receiver<check_exception_ptr>{
            check_exception_ptr{}, set_error_called};
        auto os = ex::connect(std::move(s), std::move(r));
        ex::start(os);
        HPX_TEST(set_error_called);
    }

    return hpx::util::report_errors();
}
