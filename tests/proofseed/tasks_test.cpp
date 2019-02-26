// clazy:skip

#include "proofseed/asynqro_extra.h"
#include "proofseed/tasks.h"

#include "gtest/proof/test_global.h"

#include <QDateTime>
#include <QPair>
#include <QThread>
#include <QTimer>

using namespace Proof;
using namespace Proof::tasks;

TEST(TasksTest, signalWaiting)
{
    QThread thread;
    thread.start();
    std::atomic_bool ready{false};
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(&thread);
    Future<int> future = run([timer, &ready]() {
        int result = 0;
        std::function<bool()> timerCallback = [&result]() {
            result = 42;
            return true;
        };
        addSignalWaiter(timer, &QTimer::timeout, timerCallback);
        ready = true;
        fireSignalWaiters();
        return result;
    });
    while (!ready)
        ;
    EXPECT_FALSE(future.isCompleted());
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 1));
    future.wait(1000);
    ASSERT_TRUE(future.isCompleted());
    EXPECT_EQ(42, future.result());
    thread.quit();
    thread.wait(100);
    delete timer;
}
