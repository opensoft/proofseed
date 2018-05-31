#ifndef PROOF_TUPLEMAKER_H
#define PROOF_TUPLEMAKER_H
#include <tuple>

namespace Proof {
namespace detail {
template <typename T>
struct TupleMaker
{
    static std::tuple<T> result(const T &v) { return std::make_tuple(v); }
};

template <typename... T>
struct TupleMaker<std::tuple<T...>>
{
    static std::tuple<T...> result(const std::tuple<T...> &v) { return v; }
};
} // namespace detail
} // namespace Proof

#endif // PROOF_TUPLEMAKER_H
