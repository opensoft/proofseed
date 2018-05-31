#ifndef PROOFTYPETRAITS_H
#define PROOFTYPETRAITS_H
#include <type_traits>

namespace Proof {
namespace detail {
//TODO: think about universal extractor instead of different namings per level

template <typename Checkable>
struct HasTypeParams : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct HasTypeParams<Wrapper<Args...>> : std::true_type
{};

template <typename Checkable, template <typename...> class Wrapper>
struct IsSpecialization : std::false_type
{};

template <template <typename...> class Wrapper, typename... Args>
struct IsSpecialization<Wrapper<Args...>, Wrapper> : std::true_type
{};

template <typename Checkable, template <typename...> class Wrapper1, template <typename...> class Wrapper2>
struct IsSpecialization2 : std::false_type
{};

template <template <typename...> class Wrapper1, template <typename...> class Wrapper2, typename... Args>
struct IsSpecialization2<Wrapper1<Wrapper2<Args...>>, Wrapper1, Wrapper2> : std::true_type
{};
} // namespace detail
} // namespace Proof

#endif // PROOFTYPETRAITS_H
