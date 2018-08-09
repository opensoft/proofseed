#include "proofseed/future.h"

namespace Proof {
namespace futures {
namespace detail {
static thread_local std::experimental::optional<Failure> m_lastFailure;

bool hasLastFailure()
{
    return m_lastFailure.has_value();
}

Failure lastFailure()
{
    return m_lastFailure.value_or(Failure());
}

void resetLastFailure()
{
    m_lastFailure.reset();
}

void setLastFailure(Proof::Failure &&failure)
{
    m_lastFailure = std::move(failure);
}

void setLastFailure(const Proof::Failure &failure)
{
    m_lastFailure = failure;
}
} // namespace detail
} // namespace futures
} // namespace Proof
