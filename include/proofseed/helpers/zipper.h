#ifndef PROOF_ZIPPER_H
#define PROOF_ZIPPER_H

#include "proofcore/helpers/prooftypetraits.h"

#include <QSharedPointer>

#include <tuple>
#include <type_traits>

namespace Proof {
namespace __util {
template <typename...> struct Zipper;

template <template <typename...> class Wrapper, typename Left, typename Right>
struct Zipper<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value && !IsSpecialization<Right, std::tuple>::value, std::true_type>,
        Wrapper<Left>, Wrapper<Right>>
{
    typedef Wrapper<std::tuple<Left, Right>> type;
};

template <template <typename...> class Wrapper, typename ...Left, typename Right>
struct Zipper<std::enable_if_t<!IsSpecialization<Right, std::tuple>::value, std::true_type>,
        Wrapper<std::tuple<Left...>>, Wrapper<Right>>
{
    typedef Wrapper<std::tuple<Left..., Right>> type;
};

template <template <typename...> class Wrapper, typename Left, typename ...Right>
struct Zipper<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value, std::true_type>,
        Wrapper<Left>, Wrapper<std::tuple<Right...>>>
{
    typedef Wrapper<std::tuple<Left, Right...>> type;
};

template <template <typename...> class Wrapper, typename ...Left, typename ...Right>
struct Zipper<std::true_type, Wrapper<std::tuple<Left...>>, Wrapper<std::tuple<Right...>>>
{
    typedef Wrapper<std::tuple<Left..., Right...>> type;
};

template <typename Left, typename Middle, typename Right, typename ...Tail>
struct Zipper<std::true_type, Left, Middle, Right, Tail...>
{
    typedef typename Zipper<std::true_type, typename Zipper<std::true_type, Left, Middle>::type, Right, Tail...>::type type;
};

//TODO: think about making it more generic
template <typename...> struct ZipperSP;

template <template <typename...> class Wrapper, typename Left, typename Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value && !IsSpecialization<Right, std::tuple>::value, std::true_type>,
        QSharedPointer<Wrapper<Left>>, QSharedPointer<Wrapper<Right>>>
{
    typedef QSharedPointer<Wrapper<std::tuple<Left, Right>>> type;
};

template <template <typename...> class Wrapper, typename ...Left, typename Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Right, std::tuple>::value, std::true_type>,
        QSharedPointer<Wrapper<std::tuple<Left...>>>, QSharedPointer<Wrapper<Right>>>
{
    typedef QSharedPointer<Wrapper<std::tuple<Left..., Right>>> type;
};

template <template <typename...> class Wrapper, typename Left, typename ...Right>
struct ZipperSP<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value, std::true_type>,
        QSharedPointer<Wrapper<Left>>, QSharedPointer<Wrapper<std::tuple<Right...>>>>
{
    typedef QSharedPointer<Wrapper<std::tuple<Left, Right...>>> type;
};

template <template <typename...> class Wrapper, typename ...Left, typename ...Right>
struct ZipperSP<std::true_type, QSharedPointer<Wrapper<std::tuple<Left...>>>, QSharedPointer<Wrapper<std::tuple<Right...>>>>
{
    typedef QSharedPointer<Wrapper<std::tuple<Left..., Right...>>> type;
};

template <typename Left, typename Middle, typename Right, typename ...Tail>
struct ZipperSP<std::true_type, Left, Middle, Right, Tail...>
{
    typedef typename ZipperSP<std::true_type, typename ZipperSP<std::true_type, Left, Middle>::type, Right, Tail...>::type type;
};


}
}

#endif // PROOF_ZIPPER_H
