#ifndef PROOF_SPINLOCK_H
#define PROOF_SPINLOCK_H

#include "proofseed/proofseed_global.h"

#include <atomic>

namespace Proof {

class PROOF_SEED_EXPORT SpinLock
{
public:
    SpinLock();

    void lock();
    bool tryLock();
    void unlock();

private:
    std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

} // namespace Proof

#endif // PROOF_SPINLOCK_H
