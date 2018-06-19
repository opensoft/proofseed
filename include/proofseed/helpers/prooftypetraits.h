#ifndef PROOFTYPETRAITS_H
#define PROOFTYPETRAITS_H
#include <type_traits>

namespace Proof {
template <typename Checkable>
struct HasTypeParams : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct HasTypeParams<Wrapper<Args...>> : std::true_type
{};

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
} // namespace Proof

#endif // PROOFTYPETRAITS_H
