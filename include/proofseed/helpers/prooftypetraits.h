#ifndef PROOFTYPETRAITS_H
#define PROOFTYPETRAITS_H
#include <type_traits>

namespace Proof {
namespace detail {
template <typename Checkable>
struct HasTypeParams : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct HasTypeParams<Wrapper<Args...>> : std::true_type
{};

template <int, typename Checkable, template <typename...> class... Wrapper>
struct IsSpecialization : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct IsSpecialization<1, Wrapper<Args...>, Wrapper> : std::true_type
{};

template <int depth, template <typename...> class Wrapper, template <typename...> class... Others, typename... Args>
struct IsSpecialization<depth, Wrapper<Args...>, Wrapper, Others...> : IsSpecialization<depth - 1, Args..., Others...>
{
    static_assert(depth > 1, "IsSpecialization depth should be positive");
};
} // namespace detail
} // namespace Proof

#endif // PROOFTYPETRAITS_H
