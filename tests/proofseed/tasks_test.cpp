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

TEST(TasksTest, emptySignalWaiting)
{
    Future<int> future = run([]() {
        fireSignalWaiters();
        return 42;
    });
    future.wait(1000);
    ASSERT_TRUE(future.isCompleted());
    EXPECT_EQ(42, future.result());
}

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

TEST(TasksTest, multipleSignalWaiting)
{
    QThread thread;
    thread.start();
    std::atomic_bool ready{false};
    std::atomic_bool ready2{false};
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(&thread);
    QTimer *timer2 = new QTimer;
    timer2->setSingleShot(true);
    timer2->moveToThread(&thread);
    Future<int> future = run([timer, timer2, &ready, &ready2]() {
        int result = 0;
        std::function<bool()> timerCallback = [&result, &ready2]() {
            result = 21;
            ready2 = true;
            return false;
        };
        std::function<bool()> timer2Callback = [&result]() {
            result = 42;
            return true;
        };
        addSignalWaiter(timer, &QTimer::timeout, timerCallback);
        addSignalWaiter(timer2, &QTimer::timeout, timer2Callback);
        ready = true;
        fireSignalWaiters();
        return result;
    });
    while (!ready)
        ;
    EXPECT_FALSE(future.isCompleted());
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 1));
    while (!ready2)
        ;
    ASSERT_FALSE(future.isCompleted());
    QMetaObject::invokeMethod(timer2, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 1));
    future.wait(1000);
    ASSERT_TRUE(future.isCompleted());
    EXPECT_EQ(42, future.result());
    thread.quit();
    thread.wait(100);
    delete timer;
}
