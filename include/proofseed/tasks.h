/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: denis.kormalev@opensoftdev.com (Denis Kormalev)
 *
 */
#ifndef PROOF_TASKS_H
#define PROOF_TASKS_H

#include "proofseed/proofseed_global.h"

#include <QEventLoop>
#include <QSharedPointer>

#include <functional>

namespace Proof {
namespace tasks {

class PROOF_SEED_EXPORT TasksExtra
{
public:
    TasksExtra() = delete;
    TasksExtra(const TasksExtra &) = delete;
    TasksExtra(TasksExtra &&) = delete;
    TasksExtra &operator=(const TasksExtra &) = delete;
    TasksExtra &operator=(TasksExtra &&) = delete;
    ~TasksExtra() = delete;

    template <class SignalSender, class SignalType, class... Args>
    static void addSignalWaiter(SignalSender *sender, SignalType signal, std::function<bool(Args...)> callback) noexcept
    {
        std::function<void(const QSharedPointer<QEventLoop> &)> connector = [sender, signal, callback](
            const QSharedPointer<QEventLoop> &eventLoop) noexcept
        {
            auto connection = QSharedPointer<QMetaObject::Connection>::create();
            auto eventLoopWeak = eventLoop.toWeakRef();
            std::function<void(Args...)> slot = [callback, eventLoopWeak, connection](Args... args) {
                QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
                if (!eventLoop)
                    return;
                if (!callback(args...))
                    return;
                QObject::disconnect(*connection);
                if (!eventLoopStarted())
                    clearEventLoop();
                else
                    eventLoop->quit();
            };
            *connection = QObject::connect(sender, signal, eventLoop.data(), slot, Qt::QueuedConnection);
        };
        addSignalWaiterPrivate(std::move(connector));
    }

    static void fireSignalWaiters() noexcept;

private:
    static void addSignalWaiterPrivate(std::function<void(const QSharedPointer<QEventLoop> &)> &&connector) noexcept;
    static bool eventLoopStarted() noexcept;
    static void clearEventLoop() noexcept;
};

template <typename SignalSender, typename SignalType, typename... Args>
void addSignalWaiter(SignalSender *sender, SignalType signal, std::function<bool(Args...)> callback) noexcept
{
    TasksExtra::addSignalWaiter(sender, signal, callback);
}

inline void fireSignalWaiters() noexcept
{
    TasksExtra::fireSignalWaiters();
}
} // namespace tasks
} // namespace Proof

#endif // PROOF_TASKS_H
