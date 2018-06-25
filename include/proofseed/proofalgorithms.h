#ifndef PROOFALGORITHMS_H
#define PROOFALGORITHMS_H

#include "proofcore/helpers/prooftypetraits.h"

#include <QPair>

#include <algorithm>
#include <type_traits>

namespace Proof {
namespace algorithms {
namespace detail {
template <int current>
struct level : level<current - 1>
{};
template <>
struct level<0>
{};

template <typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.push_back(value), void())
{
    container.push_back(value);
}

template <typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value), void())
{
    container.insert(value);
}

template <typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value.key(), value.value()), void())
{
    container.insert(value.key(), value.value());
}

template <typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value.first, value.second), void())
{
    container.insert(value.first, value.second);
}

template <typename C>
auto reserveContainer(C &container, long long size, level<1>) -> decltype(container.reserve(size), void())
{
    container.reserve(size);
}

template <typename C>
void reserveContainer(C &, long long, level<0>)
{}

//Caller
template <typename C>
void reserveContainer(C &container, long long size)
{
    reserveContainer(container, size, level<1>{});
}

template <typename C>
auto beginIterator(C &container, level<3>) -> decltype(container.cbegin())
{
    return container.cbegin();
}

template <typename C>
auto beginIterator(C &container, level<2>) -> decltype(container.constBegin())
{
    return container.constBegin();
}

template <typename C>
auto beginIterator(C &container, level<1>) -> decltype(container.begin())
{
    return container.begin();
}

template <typename C>
auto beginIterator(C &container, level<0>) -> decltype(std::begin(container))
{
    return std::begin(container);
}

//Caller
template <typename C>
auto beginIterator(C &container)
{
    return beginIterator(container, level<3>{});
}

template <typename C>
auto endIterator(C &container, level<3>) -> decltype(container.cend())
{
    return container.cend();
}

template <typename C>
auto endIterator(C &container, level<2>) -> decltype(container.constEnd())
{
    return container.constEnd();
}

template <typename C>
auto endIterator(C &container, level<1>) -> decltype(container.end())
{
    return container.end();
}

template <typename C>
auto endIterator(C &container, level<0>) -> decltype(std::end(container))
{
    return std::end(container);
}

//Caller
template <typename C>
auto endIterator(C &container)
{
    return endIterator(container, level<3>{});
}

//TODO: remove this workaround with wrapper for const_cast after msvc fix its INTERNAL COMPILER ERROR
template <typename T>
T &constCastWrapper(const T &ref)
{
    return const_cast<T &>(ref);
}
} // namespace detail

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
template <typename Container, typename Func, typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
auto mapInPlace(Container &container, const Func &func)
    -> decltype(*container.begin() = func(qAsConst(*container.begin())), void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        *it = func(qAsConst(*it));
}

//Non-socketed type with indices
template <typename Container, typename Func, typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
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
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func>
auto mapInPlace(Container<InputKey, InputValue> &container, const Func &func)
    -> decltype(container.begin()->second = func(qAsConst(container.begin()->first), qAsConst(container.begin()->second)),
                void())
{
    auto it = container.begin();
    auto end = container.end();
    for (; it != end; ++it)
        it->second = func(qAsConst(it->first), qAsConst(it->second));
}

template <typename Container, typename Predicate, typename Result = typename Container::value_type>
auto findIf(const Container &container, const Predicate &predicate, const Result &defaultValue = Result())
    -> decltype(predicate(*detail::beginIterator(container)), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            return *it;
    }
    return defaultValue;
}

template <typename Container, typename Predicate,
          typename Result = QPair<typename Container::key_type, typename Container::mapped_type>>
auto findIf(const Container &container, const Predicate &predicate, const Result &defaultValue = Result())
    -> decltype(predicate(detail::beginIterator(container).key(), detail::beginIterator(container).value()), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            return qMakePair(it.key(), it.value());
    }
    return defaultValue;
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

template <typename Container, typename Func, typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
auto forEach(const Container &container, const Func &func) -> decltype(func(*detail::beginIterator(container)), void())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        func(*it);
}

template <typename Container, typename Func, typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
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

// filter, map and reduce are not lazy, they copy values.
// It is fine for almost all our cases with DTOs and PODs, but still is O(n).
// O(n) though is good enough for our amounts of data.
// If ever lazy variant will be needed probably we will need to use boost ranges or something similar then.
template <typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
    -> decltype(detail::addToContainer(detail::constCastWrapper(container), *(detail::beginIterator(container))),
                predicate(*detail::beginIterator(container)), Container())
{
    Container result;
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            detail::addToContainer(result, *it);
    }
    return result;
}

template <typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
    -> decltype(detail::addToContainer(detail::constCastWrapper(container), detail::beginIterator(container)),
                predicate(detail::beginIterator(container).key(), detail::beginIterator(container).value()), Container())
{
    Container result;
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            detail::addToContainer(result, it);
    }
    return result;
}

//Non-socketed type without indices
template <typename Container, typename Func, typename Result,
          typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
auto map(const Container &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(*detail::beginIterator(container))), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        detail::addToContainer(destination, func(*it));
    return destination;
}

//Non-socketed type with indices
template <typename Container, typename Func, typename Result,
          typename = typename std::enable_if_t<!Proof::HasTypeParams<Container>::value>>
auto map(const Container &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(0ll, *detail::beginIterator(container))), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    long long counter = -1;
    for (; it != end; ++it)
        detail::addToContainer(destination, func(++counter, *it));
    return destination;
}

//Single socketed type without indices
template <template <typename...> class Container, typename Input, typename Func, typename Result>
auto map(const Container<Input> &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(*detail::beginIterator(container))), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        detail::addToContainer(destination, func(*it));
    return destination;
}

//Single socketed type with indices
template <template <typename...> class Container, typename Input, typename Func, typename Result>
auto map(const Container<Input> &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(0ll, *detail::beginIterator(container))), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    long long counter = -1;
    for (; it != end; ++it)
        detail::addToContainer(destination, func(++counter, *it));
    return destination;
}

//Double socketed Qt type
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func, typename Result>
auto map(const Container<InputKey, InputValue> &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(detail::beginIterator(container).key(),
                                                         detail::beginIterator(container).value())),
                Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        detail::addToContainer(destination, func(it.key(), it.value()));
    return destination;
}

//Double socketed stl type
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func, typename Result>
auto map(const Container<InputKey, InputValue> &container, const Func &func, Result destination)
    -> decltype(detail::addToContainer(destination, func(detail::beginIterator(container)->first,
                                                         detail::beginIterator(container)->second)),
                Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    detail::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        detail::addToContainer(destination, func(it->first, it->second));
    return destination;
}

//Single socketed type without indices, short version
template <template <typename...> class Container, typename Input, typename Func>
auto map(const Container<Input> &container, const Func &func)
    -> decltype(func(*detail::beginIterator(container)), Container<typename std::result_of_t<Func(Input)>>())
{
    return map(container, func, Container<typename std::result_of_t<Func(Input)>>());
}

//Single socketed type with indices, short version
template <template <typename...> class Container, typename Input, typename Func>
auto map(const Container<Input> &container, const Func &func)
    -> decltype(func(0ll, *detail::beginIterator(container)),
                Container<typename std::result_of_t<Func(long long, Input)>>())
{
    return map(container, func, Container<typename std::result_of_t<Func(long long, Input)>>());
}

//Double socketed type, short version
template <template <typename...> class Container, typename InputKey, typename InputValue, typename Func,
          typename OutputKey = typename std::result_of_t<Func(InputKey, InputValue)>::first_type,
          typename OutputValue = typename std::result_of_t<Func(InputKey, InputValue)>::second_type>
Container<OutputKey, OutputValue> map(const Container<InputKey, InputValue> &container, const Func &func)
{
    return map(container, func, Container<OutputKey, OutputValue>());
}

template <typename Container, typename Func, typename Result>
auto reduce(const Container &container, const Func &func, Result acc)
    -> decltype(acc = func(acc, *detail::beginIterator(container)), Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        acc = func(acc, *it);
    return acc;
}

template <typename Container, typename Func, typename Result>
auto reduce(const Container &container, const Func &func, Result acc)
    -> decltype(acc = func(acc, detail::beginIterator(container).key(), detail::beginIterator(container).value()),
                Result())
{
    auto it = detail::beginIterator(container);
    auto end = detail::endIterator(container);
    for (; it != end; ++it)
        acc = func(acc, it.key(), it.value());
    return acc;
}

template <template <typename...> class Container1, template <typename...> class Container2, typename Input, typename Result>
auto flatten(const Container1<Container2<Input>> &container, Result destination)
    -> decltype(detail::addToContainer(destination, *detail::beginIterator(*detail::beginIterator(container))), Result())
{
    long long estimatedSize = 0;
    int estimationsLeft = 100;
    auto outerIt = detail::beginIterator(container);
    auto outerEnd = detail::endIterator(container);
    for (; outerIt != outerEnd && estimationsLeft; ++outerIt, --estimationsLeft)
        estimatedSize += outerIt->size();
    if (!estimationsLeft)
        estimatedSize = (float)estimatedSize / 100.0 * container.size();
    detail::reserveContainer(destination, estimatedSize + destination.size());
    for (outerIt = detail::beginIterator(container); outerIt != outerEnd; ++outerIt) {
        auto it = detail::beginIterator(*outerIt);
        auto end = detail::endIterator(*outerIt);
        for (; it != end; ++it)
            detail::addToContainer(destination, *it);
    }
    return destination;
}

template <template <typename...> class Container1, template <typename...> class Container2, typename Input>
Container1<Input> flatten(const Container1<Container2<Input>> &container)
{
    return flatten(container, Container1<Input>());
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
} // namespace algorithms
} // namespace Proof

#endif // PROOFALGORITHMS_H
