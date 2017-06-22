//
// immer - immutable data structures for C++
// Copyright (C) 2016, 2017 Juan Pedro Bolivar Puente
//
// This file is part of immer.
//
// immer is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// immer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with immer.  If not, see <http://www.gnu.org/licenses/>.
//

#include "fuzzer_input.hpp"
#include <immer/flex_vector.hpp>
#include <iostream>

extern "C"
int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    constexpr auto var_count = 8;
    constexpr auto bits      = 2;

    using vector_t = immer::flex_vector<int, immer::default_memory_policy, bits, bits>;
    using size_t   = std::uint8_t;

    auto vars = std::array<vector_t, var_count>{};

    auto is_valid_var = [&] (auto idx) {
        return idx >= 0 && idx < var_count;
    };
    auto is_valid_var_neq = [](auto other) {
        return [=] (auto idx) {
            return idx >= 0 && idx < var_count && idx != other;
        };
    };
    auto is_valid_index = [] (auto& v) {
        return [&] (auto idx) { return idx >= 0 && idx < v.size(); };
    };
    auto is_valid_size = [] (auto& v) {
        return [&] (auto idx) { return idx >= 0 && idx <= v.size(); };
    };
    auto can_concat = [] (auto&& v1, auto&& v2) {
        using size_type = decltype(v1.size());
        auto max = std::numeric_limits<size_type>::max() >> (bits * 4);
        return v1.size() < max && v2.size() < max;
    };
    return fuzzer_input{data, size}.run([&] (auto& in)
    {
        enum ops {
            op_push_back,
            op_update,
            op_take,
            op_drop,
            op_concat,
            op_push_back_move,
            op_update_move,
            op_take_move,
            op_drop_move,
            op_concat_move_l,
            op_concat_move_r,
            op_concat_move_lr,
        };
        auto src = read<char>(in, is_valid_var);
        auto dst = read<char>(in, is_valid_var);
        switch (read<char>(in))
        {
        case op_push_back:
            vars[dst] = vars[src]
                .push_back(42);
            break;
        case op_update:
            vars[dst] = vars[src]
                .update(read<size_t>(in, is_valid_index(vars[src])),
                        [] (auto x) { return x + 1; });
            break;
        case op_take:
            vars[dst] = vars[src]
                .take(read<size_t>(in, is_valid_size(vars[src])));
            break;
        case op_drop:
            vars[dst] = vars[src]
                .drop(read<size_t>(in, is_valid_size(vars[src])));
            break;
        case op_concat: {
            auto src2 = read<char>(in, is_valid_var);
            if (can_concat(vars[src], vars[src2]))
                vars[dst] = vars[src] + vars[src2];
            break;
        }
        case op_push_back_move: {
            vars[dst] = std::move(vars[src])
                .push_back(21);
            break;
        }
        case op_update_move: {
            vars[dst] = std::move(vars[src])
                .update(read<size_t>(in, is_valid_index(vars[src])),
                        [] (auto x) { return x + 1; });
            break;
        }
        case op_take_move: {
            vars[dst] = std::move(vars[src])
                .take(read<size_t>(in, is_valid_size(vars[src])));
            break;
        }
        case op_drop_move: {
            vars[dst] = std::move(vars[src])
                .drop(read<size_t>(in, is_valid_size(vars[src])));
            break;
        }
        case op_concat_move_l: {
            auto src2 = read<char>(in, is_valid_var_neq(src));
            if (can_concat(vars[src], vars[src2]))
                vars[dst] = std::move(vars[src]) + vars[src2];
            break;
        }
        case op_concat_move_r: {
            auto src2 = read<char>(in, is_valid_var_neq(src));
            if (can_concat(vars[src], vars[src2]))
                vars[dst] = vars[src] + std::move(vars[src2]);
            break;
        }
        case op_concat_move_lr: {
            auto src2 = read<char>(in, is_valid_var_neq(src));
            if (can_concat(vars[src], vars[src2]))
                vars[dst] = std::move(vars[src]) + std::move(vars[src2]);
            break;
        }
        default:
            break;
        };
        return true;
    });
}
