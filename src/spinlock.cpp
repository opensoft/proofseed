#include "spinlock_p.h"

#include <QThread>

static constexpr unsigned long SLEEP_MSECS = 1;
static constexpr size_t ITERATIONS_COUNT = 10;

namespace Proof {

SpinLock::SpinLock()
{
}

void SpinLock::lock()
{
    while (!tryLock())
        QThread::msleep(SLEEP_MSECS);
}

bool SpinLock::tryLock()
{
    bool result = false;
    for (size_t i = 0; !result && i < ITERATIONS_COUNT; ++i)
        result = !m_lock.test_and_set(std::memory_order_acquire);
    return result;
}

void SpinLock::unlock()
{
    m_lock.clear(std::memory_order_release);
}

} // namespace Proof
