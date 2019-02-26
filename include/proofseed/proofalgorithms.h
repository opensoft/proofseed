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
#ifndef PROOFALGORITHMS_H
#define PROOFALGORITHMS_H

#include "asynqro/impl/containers_traverse.h"

#include <QPair>

#include <algorithm>
#include <tuple>
#include <type_traits>

namespace Proof {
namespace algorithms {
namespace detail {
template <typename C, typename T>
void addToContainer(C &container, const T &value)
{
    asynqro::traverse::detail::containers::add(container, value);
}

template <typename C>
void reserveContainer(C &container, long long size)
{
    asynqro::traverse::detail::containers::reserve(container, size);
}

template <typename C>
auto beginIterator(C &container)
{
    return asynqro::traverse::detail::containers::begin(container);
}

template <typename C>
auto endIterator(C &container)
{
    return asynqro::traverse::detail::containers::end(container);
}

//TODO: remove this workaround with wrapper for const_cast after msvc fix its INTERNAL COMPILER ERROR
template <typename T>
T &constCastWrapper(const T &ref)
{
    return const_cast<T &>(ref);
}
} // namespace detail

using asynqro::traverse::filter;
using asynqro::traverse::findIf;
using asynqro::traverse::flatten;
using asynqro::traverse::map;
using asynqro::traverse::reduce;

template <typename Container, typename Predicate>
auto eraseIf(Container &container, const Predicate &predicate)
    -> decltype(predicate(container.begin().key(), qAsConst(container.begin().value())), void())
{
    for (auto it = container.begin(); it != container.end();) {
        if (predicate(it.key(), qAsConst(it.value())))
            it = container.erase(it);
        else
            ++it;
    }
}

template <typename Container, typename Predicate>
auto eraseIf(Container &container, const Predicate &predicate)
    -> decltype(predicate(qAsConst(*container.begin())), void())
{
    container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
}

template <typename Container>
void makeUnique(Container &container)
{
    container.erase(std::unique(container.begin(), container.end()), container.end());
}

//Non-socketed type without indices
template <typename Container, typename Func,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
auto mapInPlace(Container &container, const Func &func)
    -> decltype(*container.begin() = func(qAsConst(*container.begin())), void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        *it = func(qAsConst(*it));
}

//Non-socketed type with indices
template <typename Container, typename Func,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
auto mapInPlace(Container &container, const Func &func)
    -> decltype(*container.begin() = func(0ll, qAsConst(*container.begin())), void())
{
    auto it = container.begin();
    auto end = container.end();
    long long counter = -1;
    for (; it != end; ++it)
        *it = func(++counter, qAsConst(*it));
}

//Single socketed type without indices
template <template <typename...> class Container, typename Input, typename Func>
auto mapInPlace(Container<Input> &container, const Func &func)
    -> decltype(*container.begin() = func(qAsConst(*container.begin())), void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        *it = func(qAsConst(*it));
}

//Single socketed type with indices
template <template <typename...> class Container, typename Input, typename Func>
auto mapInPlace(Container<Input> &container, const Func &func)
    -> decltype(*container.begin() = func(0ll, qAsConst(*container.begin())), void())
{
    auto it = container.begin();
    auto end = container.end();
    long long counter = -1;
    for (; it != end; ++it)
        *it = func(++counter, qAsConst(*it));
}

//Double socketed Qt type
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func>
auto mapInPlace(Container<InputKey, InputValue> &container, const Func &func)
    -> decltype(*container.begin() = func(container.begin().key(), qAsConst(container.begin().value())), void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        *it = func(it.key(), qAsConst(it.value()));
}

//Double socketed stl type
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func, typename... Args>
auto mapInPlace(Container<InputKey, InputValue, Args...> &container, const Func &func)
    -> decltype(container.begin()->second = func(qAsConst(container.begin()->first), qAsConst(container.begin()->second)),
                void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        it->second = func(qAsConst(it->first), qAsConst(it->second));
}

template <typename Container, typename Predicate>
auto exists(const Container &container, const Predicate &predicate)
    -> decltype(predicate(*detail::beginIterator(container)), bool())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            return true;
    }
    return false;
}

template <typename Container, typename Predicate>
auto exists(const Container &container, const Predicate &predicate)
    -> decltype(predicate(detail::beginIterator(container).key(), detail::beginIterator(container).value()), bool())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            return true;
    }
    return false;
}

template <typename Container, typename Predicate>
auto forAll(const Container &container, const Predicate &predicate)
    -> decltype(predicate(*detail::beginIterator(container)), bool())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (!predicate(*it))
            return false;
    }
    return true;
}

template <typename Container, typename Predicate>
auto forAll(const Container &container, const Predicate &predicate)
    -> decltype(predicate(detail::beginIterator(container).key(), detail::beginIterator(container).value()), bool())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (!predicate(it.key(), it.value()))
            return false;
    }
    return true;
}

template <typename Container, typename Func,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
auto forEach(const Container &container, const Func &func) -> decltype(func(*detail::beginIterator(container)), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        func(*it);
}

template <typename Container, typename Func,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
auto forEach(const Container &container, const Func &func)
    -> decltype(func(0ll, *detail::beginIterator(container)), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    long long counter = -1;
    for (; it != end; ++it)
        func(++counter, *it);
}

template <template <typename...> class Container, typename Input, typename Func>
auto forEach(const Container<Input> &container, const Func &func)
    -> decltype(func(*detail::beginIterator(container)), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        func(*it);
}

template <template <typename...> class Container, typename Input, typename Func>
auto forEach(const Container<Input> &container, const Func &func)
    -> decltype(func(0ll, *detail::beginIterator(container)), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    long long counter = -1;
    for (; it != end; ++it)
        func(++counter, *it);
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func>
auto forEach(const Container<InputKey, InputValue> &container, const Func &func)
    -> decltype(func(detail::beginIterator(container).key(), detail::beginIterator(container).value()), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        func(it.key(), it.value());
}

inline auto identity()
{
    return [](const auto &x) { return x; };
}

inline auto keyIdentity()
{
    return [](const auto &x, const auto &) { return x; };
}

inline auto valueIdentity()
{
    return [](const auto &, const auto &x) { return x; };
}

template <typename Container, typename Input = typename Container::value_type,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
QSet<Input> toSet(const Container &container)
{
    return map(container, identity(), QSet<Input>());
}

template <typename Container, typename Input = typename Container::value_type,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
QVector<Input> toVector(const Container &container)
{
    return map(container, identity(), QVector<Input>());
}

template <typename Container, typename Input = typename Container::value_type,
          typename = typename std::enable_if_t<!asynqro::traverse::detail::HasTypeParams_V<Container>>>
QList<Input> toList(const Container &container)
{
    return map(container, identity(), QList<Input>());
}

template <template <typename...> class Container, typename Input>
QSet<Input> toSet(const Container<Input> &container)
{
    return map(container, identity(), QSet<Input>());
}

template <template <typename...> class Container, typename Input>
QVector<Input> toVector(const Container<Input> &container)
{
    return map(container, identity(), QVector<Input>());
}

template <template <typename...> class Container, typename Input>
QList<Input> toList(const Container<Input> &container)
{
    return map(container, identity(), QList<Input>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QSet<InputKey> toKeysSet(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, keyIdentity(), QSet<InputKey>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QVector<InputKey> toKeysVector(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, keyIdentity(), QVector<InputKey>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QList<InputKey> toKeysList(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, keyIdentity(), QList<InputKey>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QSet<InputValue> toValuesSet(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, valueIdentity(), QSet<InputValue>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QVector<InputValue> toValuesVector(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, valueIdentity(), QVector<InputValue>());
}

template <template <typename...> class Container, typename InputKey, typename InputValue, typename... Args>
QList<InputValue> toValuesList(const Container<InputKey, InputValue, Args...> &container)
{
    return map(container, valueIdentity(), QList<InputValue>());
}

template <template <typename...> class Container1, template <typename...> class Container2, typename Input,
          typename Predicate, typename Result>
auto flatFilter(const Container1<Container2<Input>> &container, const Predicate &predicate, Result destination)
    -> decltype(detail::addToContainer(destination, *detail::beginIterator(*detail::beginIterator(container))),
                predicate(*detail::beginIterator(*detail::beginIterator(container))), Result())
{
    auto outerIt = detail::beginIterator(container);
    auto outerEnd = detail::endIterator(container);
    for (; outerIt != outerEnd; ++outerIt) {
        auto it = detail::beginIterator(*outerIt);
        auto end = detail::endIterator(*outerIt);
        for (; it != end; ++it) {
            if (predicate(*it))
                detail::addToContainer(destination, *it);
        }
    }
    return destination;
}

template <template <typename...> class Container1, template <typename...> class Container2, typename Input, typename Predicate>
Container1<Input> flatFilter(const Container1<Container2<Input>> &container, const Predicate &predicate)
{
    return flatFilter(container, predicate, Container1<Input>());
}

//extract tuple from tuple
template <int... I, typename... T>
constexpr auto sieve(const std::tuple<T...> &arg)
{
    return std::make_tuple(std::get<I>(arg)...);
}

} // namespace algorithms
} // namespace Proof

#endif // PROOFALGORITHMS_H
