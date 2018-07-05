#ifndef PROOFTYPETRAITS_H
#define PROOFTYPETRAITS_H
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
} // namespace Proof

#endif // PROOFTYPETRAITS_H
