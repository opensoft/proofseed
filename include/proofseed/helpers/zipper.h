#ifndef PROOF_ZIPPER_H
#define PROOF_ZIPPER_H

#include "proofcore/helpers/prooftypetraits.h"

#include <QSharedPointer>

#include <tuple>
#include <type_traits>

namespace Proof {
namespace detail {

//TODO: think about making it more generic
template <typename...>
struct ZipperSP;

template <template <typename...> class Wrapper, typename Left, typename Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value && !IsSpecialization<Right, std::tuple>::value,
                                 std::true_type>,
                QSharedPointer<Wrapper<Left>>, QSharedPointer<Wrapper<Right>>>
{
    using type = QSharedPointer<Wrapper<std::tuple<Left, Right>>>;
};

template <template <typename...> class Wrapper, typename... Left, typename Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Right, std::tuple>::value, std::true_type>,
                QSharedPointer<Wrapper<std::tuple<Left...>>>, QSharedPointer<Wrapper<Right>>>
{
    using type = QSharedPointer<Wrapper<std::tuple<Left..., Right>>>;
};

template <template <typename...> class Wrapper, typename Left, typename... Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value, std::true_type>,
                QSharedPointer<Wrapper<Left>>, QSharedPointer<Wrapper<std::tuple<Right...>>>>
{
    using type = QSharedPointer<Wrapper<std::tuple<Left, Right...>>>;
};

template <template <typename...> class Wrapper, typename... Left, typename... Right>
struct ZipperSP<std::true_type, QSharedPointer<Wrapper<std::tuple<Left...>>>, QSharedPointer<Wrapper<std::tuple<Right...>>>>
{
    using type = QSharedPointer<Wrapper<std::tuple<Left..., Right...>>>;
};

template <typename Left, typename Middle, typename Right, typename... Tail>
struct ZipperSP<std::true_type, Left, Middle, Right, Tail...>
{
    using type =
        typename ZipperSP<std::true_type, typename ZipperSP<std::true_type, Left, Middle>::type, Right, Tail...>::type;
};

} // namespace detail
} // namespace Proof

#endif // PROOF_ZIPPER_H
