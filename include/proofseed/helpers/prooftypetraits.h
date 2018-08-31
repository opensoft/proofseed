/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: denis.kormalev@opensoftdev.com (Denis Kormalev)
 *
 */
#ifndef PROOFTYPETRAITS_H
#define PROOFTYPETRAITS_H
#include <tuple>
#include <type_traits>

namespace Proof {
namespace detail {
template <int current>
struct level : level<current - 1>
{};
template <>
struct level<0>
{};
} // namespace detail

template <typename Checkable>
struct TypeParamsCount
{
    static constexpr int value = 0;
};

template <template <typename...> class Wrapper, typename... Args>
struct TypeParamsCount<Wrapper<Args...>>
{
    static constexpr int value = sizeof...(Args);
};

template <typename Checkable>
struct HasTypeParams
{
    static constexpr bool value = TypeParamsCount<Checkable>::value;
};

template <typename Checkable, template <typename...> class... Wrapper>
struct IsSpecialization : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct IsSpecialization<Wrapper<Args...>, Wrapper> : std::true_type
{};

template <template <typename...> class Wrapper, template <typename...> class Wrapper2,
          template <typename...> class... Others, typename... Args>
struct IsSpecialization<Wrapper<Wrapper2<Args...>>, Wrapper, Wrapper2, Others...>
    : IsSpecialization<Wrapper2<Args...>, Wrapper2, Others...>
{};

template <typename T, int... forkSolvers>
struct NestingLevel
{
    static constexpr int value = 0;
};

template <template <typename...> class Wrapper, typename FirstArg, typename... Args>
struct NestingLevel<Wrapper<FirstArg, Args...>>
{
    static constexpr int value = 1 + NestingLevel<FirstArg>::value;
};

template <template <typename...> class Wrapper, typename... Args, int forkSolver, int... forkSolvers>
struct NestingLevel<Wrapper<Args...>, forkSolver, forkSolvers...>
{
private:
    static constexpr int position = forkSolver >= sizeof...(Args) ? 1 : forkSolver;
    using ForkSolution = typename std::tuple_element<position, std::tuple<Args...>>::type;

public:
    static constexpr int value = 1 + NestingLevel<ForkSolution, forkSolvers...>::value;
};

} // namespace Proof

#endif // PROOFTYPETRAITS_H
