/* Copyright 2019, OpenSoft Inc.
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
#ifndef PROOFSEED_ASYNQRO_EXTRA_H
#define PROOFSEED_ASYNQRO_EXTRA_H

#include "asynqro/asynqro"

#include <QVariant>

#include <string>
#include <type_traits>

namespace Proof {
struct Failure
{
    enum Hints
    {
        NoHint = 0x0,
        UserFriendlyHint = 0x1,
        CriticalHint = 0x2,
        DataIsHttpCodeHint = 0x4,
        FromExceptionHint = 0x8
    };
    Failure(const QString &message, long moduleCode, long errorCode, unsigned long hints = Failure::NoHint,
            const QVariant &data = QVariant()) noexcept
        : exists(true), moduleCode(moduleCode), errorCode(errorCode), hints(hints), message(message), data(data)
    {}
    explicit Failure(const QVariant &data) noexcept : exists(true), data(data) {}
    Failure() noexcept {}
    Failure(Failure &&) noexcept = default;
    Failure(const Failure &) = default;
    ~Failure() = default;
    Failure &operator=(Failure &&) noexcept = default;
    Failure &operator=(const Failure &) = default;
    operator QString() noexcept { return message; } // NOLINT(google-explicit-constructor)
    Failure withMessage(const QString &msg) const noexcept { return Failure(msg, moduleCode, errorCode, hints, data); }
    Failure withCode(long module, long error) const noexcept { return Failure(message, module, error, hints, data); }
    Failure withData(const QVariant &d) const noexcept { return Failure(message, moduleCode, errorCode, hints, d); }

    bool exists = false;
    long moduleCode = 0;
    long errorCode = 0;
    unsigned long hints = NoHint;
    QString message;
    QVariant data;
};

template <typename T>
using Future = asynqro::Future<T, Failure>;
template <typename T>
using Promise = asynqro::Promise<T, Failure>;
template <typename T>
using CancelableFuture = asynqro::CancelableFuture<T, Failure>;
using WithFailure = asynqro::WithFailure<Failure>;
using SpinLock = asynqro::detail::SpinLock;
using SpinLockHolder = asynqro::detail::SpinLockHolder;

namespace futures {
template <typename T>
auto successful(T &&value) noexcept
{
    return Proof::Future<std::decay_t<T>>::successful(std::forward<T>(value));
}

template <typename T>
auto successful(const T &value) noexcept
{
    return Proof::Future<T>::successful(value);
}

// This overload copies container to make sure that it will be reachable in future
template <typename T, template <typename...> typename F, template <typename...> typename Container, typename... Fs>
Proof::Future<Container<T>> sequence(const Container<F<T, Proof::Failure>, Fs...> &container) noexcept
{
    Container<F<T, Proof::Failure>> copy = container;
    return Proof::Future<T>::sequence(std::move(copy));
}

template <typename T, template <typename...> typename F, template <typename...> typename Container, typename... Fs>
Proof::Future<Container<T>> sequence(Container<F<T, Proof::Failure>, Fs...> &&container) noexcept
{
    return Proof::Future<T>::sequence(std::move(container));
}
} // namespace futures

namespace tasks {
using TaskType = asynqro::tasks::TaskType;
using TaskPriority = asynqro::tasks::TaskPriority;

constexpr int32_t USER_MIN_TAG = 10000;

struct RunnerInfo
{
    using PlainFailure = Proof::Failure;
    constexpr static bool deferredFailureShouldBeConverted = false;
};
using Runner = asynqro::tasks::TaskRunner<RunnerInfo>;

template <typename... T>
auto run(T &&... args)
{
    return asynqro::tasks::run<Runner>(std::forward<T>(args)...);
}

template <typename... T>
void runAndForget(T &&... args)
{
    asynqro::tasks::runAndForget<Runner>(std::forward<T>(args)...);
}

template <typename... T>
auto clusteredRun(T &&... args)
{
    return asynqro::tasks::clusteredRun<Runner>(std::forward<T>(args)...);
}

} // namespace tasks
} // namespace Proof

namespace asynqro {
namespace failure {
template <>
inline Proof::Failure failureFromString<Proof::Failure>(const std::string &s)
{
    QString message = QString::fromStdString(s);
    unsigned long hints = Proof::Failure::UserFriendlyHint;
    if (message.startsWith(QLatin1String("Exception", Qt::CaseSensitive)))
        hints |= Proof::Failure::FromExceptionHint;
    return Proof::Failure(message, 0, 0, hints);
}
} // namespace failure
} // namespace asynqro

#endif // PROOFSEED_ASYNQRO_EXTRA_H
