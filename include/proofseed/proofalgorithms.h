#ifndef PROOFALGORITHMS_H
#define PROOFALGORITHMS_H

#include "proofcore/helpers/prooftypetraits.h"

#include <QPair>

#include <type_traits>
#include <algorithm>

namespace Proof {
namespace algorithms {
namespace __util {
template<int current> struct level : level<current - 1> {};
template<> struct level<0> {};

template<typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.push_back(value), void())
{
    container.push_back(value);
}

template<typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value), void())
{
    container.insert(value);
}

template<typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value.key(), value.value()), void())
{
    container.insert(value.key(), value.value());
}

template<typename C, typename T>
auto addToContainer(C &container, const T &value) -> decltype(container.insert(value.first, value.second), void())
{
    container.insert(value.first, value.second);
}

template<typename C>
auto reserveContainer(C &container, long long size, level<1>) -> decltype(container.reserve(size), void())
{
    container.reserve(size);
}

template<typename C> void reserveContainer(C &, long long, level<0>) {}

//Caller
template<typename C> void reserveContainer(C &container, long long size) {
    reserveContainer(container, size, level<1>{});
}

template<typename C>
auto beginIterator(C &container, level<2>) -> decltype(container.cbegin(), typename C::const_iterator())
{
    return container.cbegin();
}

template<typename C>
auto beginIterator(C &container, level<1>) -> decltype(container.constBegin(), typename C::const_iterator())
{
    return container.constBegin();
}

template<typename C>
auto beginIterator(C &container, level<0>) -> decltype(container.begin(), typename C::iterator())
{
    return container.begin();
}

//Caller
template<typename C> auto beginIterator(C &container) {
    return beginIterator(container, level<2>{});
}

template<typename C>
auto endIterator(C &container, level<2>) -> decltype(container.cend(), typename C::const_iterator())
{
    return container.cend();
}

template<typename C>
auto endIterator(C &container, level<1>) -> decltype(container.constEnd(), typename C::const_iterator())
{
    return container.constEnd();
}

template<typename C>
auto endIterator(C &container, level<0>) -> decltype(container.end(), typename C::iterator())
{
    return container.end();
}

//Caller
template<typename C> auto endIterator(C &container) {
    return endIterator(container, level<2>{});
}

//TODO: remove this workaround with wrapper for const_cast after msvc fix its INTERNAL COMPILER ERROR
template<typename T> T & constCastWrapper(const T &ref)
{
    return const_cast<T &>(ref);
}
}

template<typename Container, typename Predicate>
auto eraseIf(Container &container, const Predicate &predicate)
-> decltype(predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            void())
{
    for (auto it = container.begin(); it != container.end();) {
        if (predicate(it.key(), it.value()))
            it = container.erase(it);
        else
            ++it;
    }
}

template<typename Container, typename Predicate>
auto eraseIf(Container &container, const Predicate &predicate)
-> decltype(predicate(*(__util::beginIterator(container))),
            void())
{
    container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
}

template<typename Container>
void makeUnique(Container &container)
{
    container.erase(std::unique(container.begin(), container.end()), container.end());
}

template<typename Container, typename Predicate, typename Result = typename Container::value_type>
auto findIf(const Container &container, const Predicate &predicate, const Result &defaultValue = Result())
-> decltype(predicate(*(__util::beginIterator(container))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            return *it;
    }
    return defaultValue;
}

template<typename Container, typename Predicate, typename Result = QPair<typename Container::key_type, typename Container::mapped_type>>
auto findIf(const Container &container, const Predicate &predicate, const Result &defaultValue = Result())
-> decltype(predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            return qMakePair(it.key(), it.value());
    }
    return defaultValue;
}

template<typename Container, typename Predicate>
auto exists(const Container &container, const Predicate &predicate)
-> decltype(predicate(*(__util::beginIterator(container))),
            bool())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            return true;
    }
    return false;
}

template<typename Container, typename Predicate>
auto exists(const Container &container, const Predicate &predicate)
-> decltype(predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            bool())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            return true;
    }
    return false;
}

template<typename Container, typename Predicate>
auto forAll(const Container &container, const Predicate &predicate)
-> decltype(predicate(*(__util::beginIterator(container))),
            bool())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (!predicate(*it))
            return false;
    }
    return true;
}

template<typename Container, typename Predicate>
auto forAll(const Container &container, const Predicate &predicate)
-> decltype(predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            bool())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (!predicate(it.key(), it.value()))
            return false;
    }
    return true;
}

template<typename Container, typename Predicate,
         typename = typename std::enable_if_t<!Proof::__util::HasTypeParams<Container>::value>>
auto forEach(const Container &container, const Predicate &predicate)
-> decltype(predicate(*(__util::beginIterator(container))),
            void())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it)
        predicate(*it);
}

template<typename Container, typename Predicate,
         typename = typename std::enable_if_t<!Proof::__util::HasTypeParams<Container>::value>>
auto forEach(const Container &container, const Predicate &predicate)
-> decltype(predicate(0ll, *(__util::beginIterator(container))),
            void())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    long long counter = -1;
    for (; it != end; ++it)
        predicate(++counter, *it);
}

template<template<typename...> class Container, typename Input, typename Predicate>
auto forEach(const Container<Input> &container, const Predicate &predicate)
-> decltype(predicate(*(__util::beginIterator(container))),
            void())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it)
        predicate(*it);
}

template<template<typename...> class Container, typename Input, typename Predicate>
auto forEach(const Container<Input> &container, const Predicate &predicate)
-> decltype(predicate(0ll, *(__util::beginIterator(container))),
            void())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    long long counter = -1;
    for (; it != end; ++it)
        predicate(++counter, *it);
}

template<template<typename...> class Container, typename InputKey, typename InputValue, typename Predicate>
auto forEach(const Container<InputKey, InputValue> &container, const Predicate &predicate)
-> decltype(predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            void())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it)
        predicate(it.key(), it.value());
}

// filter, map and reduce are not lazy, they copy values.
// It is fine for almost all our cases with DTOs and PODs, but still is O(n).
// O(n) though is good enough for our amounts of data.
// If ever lazy variant will be needed probably we will need to use boost ranges or something similar then.
template<typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
-> decltype(__util::addToContainer(__util::constCastWrapper(container), *(__util::beginIterator(container))),
            predicate(*(__util::beginIterator(container))),
            Container())
{
    Container result;
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(*it))
            __util::addToContainer(result, *it);
    }
    return result;
}

template<typename Container, typename Predicate>
auto filter(const Container &container, const Predicate &predicate)
-> decltype(__util::addToContainer(__util::constCastWrapper(container), __util::beginIterator(container)),
            predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value()),
            Container())
{
    Container result;
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it) {
        if (predicate(it.key(), it.value()))
            __util::addToContainer(result, it);
    }
    return result;
}

//Non-socketed type without indices
template<typename Container, typename Predicate, typename Result,
         typename = typename std::enable_if_t<!Proof::__util::HasTypeParams<Container>::value>>
auto map(const Container &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, predicate(*(__util::beginIterator(container)))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    __util::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        __util::addToContainer(destination, predicate(*it));
    return destination;
}

//Non-socketed type with indices
template<typename Container, typename Predicate, typename Result,
         typename = typename std::enable_if_t<!Proof::__util::HasTypeParams<Container>::value>>
auto map(const Container &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, predicate(0ll, *(__util::beginIterator(container)))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    __util::reserveContainer(destination, container.size());
    long long counter = -1;
    for (; it != end; ++it)
        __util::addToContainer(destination, predicate(++counter, *it));
    return destination;
}

//Single socketed type without indices
template<template<typename...> class Container, typename Input,
         typename Predicate, typename Result>
auto map(const Container<Input> &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, predicate(*(__util::beginIterator(container)))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    __util::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        __util::addToContainer(destination, predicate(*it));
    return destination;
}

//Single socketed type with indices
template<template<typename...> class Container, typename Input,
         typename Predicate, typename Result>
auto map(const Container<Input> &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, predicate(0ll, *(__util::beginIterator(container)))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    __util::reserveContainer(destination, container.size());
    long long counter = -1;
    for (; it != end; ++it)
        __util::addToContainer(destination, predicate(++counter, *it));
    return destination;
}

//Double socketed type
template<template<typename...> class Container, typename InputKey, typename InputValue,
         typename Predicate, typename Result>
auto map(const Container<InputKey, InputValue> &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, predicate(__util::beginIterator(container).key(), __util::beginIterator(container).value())),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    __util::reserveContainer(destination, container.size());
    for (; it != end; ++it)
        __util::addToContainer(destination, predicate(it.key(), it.value()));
    return destination;
}

//Single socketed type without indices, short version
template<template<typename...> class Container, typename Input,
         typename Predicate>
auto map(const Container<Input> &container, const Predicate &predicate)
-> decltype (predicate(*(__util::beginIterator(container))),
             Container<typename std::result_of_t<Predicate(Input)>>())
{
    return map(container, predicate, Container<typename std::result_of_t<Predicate(Input)>>());
}

//Single socketed type with indices, short version
template<template<typename...> class Container, typename Input,
         typename Predicate>
auto map(const Container<Input> &container, const Predicate &predicate)
-> decltype (predicate(0ll, *(__util::beginIterator(container))),
             Container<typename std::result_of_t<Predicate(long long, Input)>>())
{
    return map(container, predicate, Container<typename std::result_of_t<Predicate(long long, Input)>>());
}

//Double socketed type, short version
template<template<typename...> class Container, typename InputKey, typename InputValue,
         typename Predicate,
         typename OutputKey = typename std::result_of_t<Predicate(InputKey, InputValue)>::first_type,
         typename OutputValue = typename std::result_of_t<Predicate(InputKey, InputValue)>::second_type>
Container<OutputKey, OutputValue> map(const Container<InputKey, InputValue> &container, const Predicate &predicate)
{
    return map(container, predicate, Container<OutputKey, OutputValue>());
}

template<typename Container, typename Predicate, typename Result>
auto reduce(const Container &container, const Predicate &predicate, Result acc)
-> decltype(acc = predicate(acc, *(__util::beginIterator(container))),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it)
        acc = predicate(acc, *it);
    return acc;
}

template<typename Container, typename Predicate, typename Result>
auto reduce(const Container &container, const Predicate &predicate, Result acc)
-> decltype(acc = predicate(acc, __util::beginIterator(container).key(), __util::beginIterator(container).value()),
            Result())
{
    auto it = __util::beginIterator(container);
    auto end = __util::endIterator(container);
    for (; it != end; ++it)
        acc = predicate(acc, it.key(), it.value());
    return acc;
}

template<template<typename...> class Container1, template<typename...> class Container2, typename Input, typename Result>
auto flatten(const Container1<Container2<Input>> &container, Result destination)
-> decltype(__util::addToContainer(destination, *(__util::beginIterator(*(__util::beginIterator(container))))),
            Result())
{
    long long estimatedSize = 0;
    int estimationsLeft = 100;
    auto outerIt = __util::beginIterator(container);
    auto outerEnd = __util::endIterator(container);
    for (; outerIt != outerEnd && estimationsLeft; ++outerIt, --estimationsLeft)
        estimatedSize += outerIt->size();
    if (!estimationsLeft)
        estimatedSize = (float)estimatedSize / 100.0 * container.size();
    __util::reserveContainer(destination, estimatedSize + destination.size());
    for (outerIt = __util::beginIterator(container); outerIt != outerEnd; ++outerIt) {
        auto it = __util::beginIterator(*outerIt);
        auto end = __util::endIterator(*outerIt);
        for (; it != end; ++it)
            __util::addToContainer(destination, *it);
    }
    return destination;
}

template<template<typename...> class Container1, template<typename...> class Container2, typename Input>
Container1<Input> flatten(const Container1<Container2<Input>> &container)
{
    return flatten(container, Container1<Input>());
}


template<template<typename...> class Container1, template<typename...> class Container2, typename Input,
         typename Predicate, typename Result>
auto flatFilter(const Container1<Container2<Input>> &container, const Predicate &predicate, Result destination)
-> decltype(__util::addToContainer(destination, *(__util::beginIterator(*(__util::beginIterator(container))))),
            predicate(*(__util::beginIterator(*(__util::beginIterator(container))))),
            Result())
{
    auto outerIt = __util::beginIterator(container);
    auto outerEnd = __util::endIterator(container);
    for (; outerIt != outerEnd; ++outerIt) {
        auto it = __util::beginIterator(*outerIt);
        auto end = __util::endIterator(*outerIt);
        for (; it != end; ++it) {
            if (predicate(*it))
                __util::addToContainer(destination, *it);
        }
    }
    return destination;
}

template<template<typename...> class Container1, template<typename...> class Container2, typename Input,
         typename Predicate>
Container1<Input> flatFilter(const Container1<Container2<Input>> &container, const Predicate &predicate)
{
    return flatFilter(container, predicate, Container1<Input>());
}
}
}

#endif // PROOFALGORITHMS_H
