Seed of Proof
=============
Low-level primitives used by Proof framework

Why?
-----
Why create another implementation of futures when we already have one in std and one in Qt and few more in smaller libraries?
`std::future` is a mess, honestly. You can't do a lot with it, only wait when it is filled. And you need to keep future `std::async` returned, because it will wait for result in its destructor.
Qt future is nicer, but you can't use it outside QtConcurrent framework without private headers.
So we decided to create implementation tailored to our needs and opensource it in hope it will help someone.

Futures examples
----------------
Who needs boring description at the beginning of the README file when we can take a look on small examples first?

#### Assume we have future from somewhere, let's wait for it
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
width->wait();
height->wait();
FutureSP<double> area = width->result() * height->result();
```

#### Let's shorten it a bit, result automatically waits
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
FutureSP<double> area = width->result() * height->result();
```

#### Boooring. Why wait when we can transform?
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
FutureSP<double> area = width->zip(height)->map([](const auto &result) {
  return std::get<0>(result) * std::get<1>(result);
});
```

#### Ok, but what if our calculation is complicated enough and is inside some function, that returns future: `(double, double) -> FutureSP<double>`?
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
FutureSP<double> area = width->zip(height)->flatMap([](const auto &result) {
  return calcArea(std::get<0>(result), std::get<1>(result));
});
```

#### This `zip()` returns tuple? Yes, of any number of elements and can be chained
```c++
FutureSP<int> first = /*...*/;
FutureSP<double> second = /*...*/;
FutureSP<QString> third = /*...*/;
FutureSP<bool> fourth = /*...*/;
first->zip(second, third)
     ->zip(fourth)
     ->onSuccess([](const std::tuple<int, double, QString, bool> &result) {
  /*...*/
});
```

#### Ok, do you have sequencing? I want to transform `QVector<FutureSP<T>>` to `FutureSP<QVector<T>>`
```c++
QVector<int> values = {/*...*/};
QVector<FutureSP<int>> futures = map(values, [](int x)-> FutureSP<int> {/*...*/});
Future<int>::sequence(futures)->onSuccess([](const QVector<int> &result) {
  /*...*/
});
```

#### Ok, that is nice. What about error handling? Can we give some default value in case of error?
```c++
FutureSP<int> failable = /*...*/;
failable->onFailure([](const Failure &f) {
  /*It will happen if initial future fails*/
})->recover([](const Failure &) {
  /*It will happen if initial future fails and should return the same type, i.e. int*/
  return 42;
})->flatMap([](int x) {
  /*It will happen in any case, because recover here always succeeds*/
})->onSuccess([](int x) {
  /*It will happen in any result of failable and in success case of previous flatMap*/
})->onFailure([](const Failure &f) {
  /*It will happen in any result of failable and in failure case of previous flatMap*/
});
```

#### Is there any option to fail inside transformation?
```c++
FutureSP<int> bornToFail = /*...*/;
bornToFail->map([](int) -> int {
  if (goodMoonPhase())
    return 42;
  else
    return WithFailure("Moon is not that good, sorry", 1337, 13);
})->onSuccess([](int x) {
  /*...*/
})->onFailure([](const Failure &f) {
  /*...*/
});
```

Task scheduling examples
------------------------
#### We run two tasks asynchronously on thread pool, and when both are done, computation will happen on thread of last of them finishing. Last `run()` will simply done eventually without any notifications or waits
```c++
FutureSP<int> first = tasks::run([] {
  //Long task
  return 42;
});
FutureSP<int> second = tasks::run([] {
  //Long task
  return 33;
});
first->zip(second)->onSuccess([](auto &x) {
  long result = std::get<0>(x) + std::get<1>(x);
  //Do something with result
});
tasks::run([] {
  //Too long task to wait
});
```

#### Same task on array of data with no more than 6 jobs at one moment
```c++
tasks::TasksDispatcher->addCustomRestrictor("jobFetch", 6);
tasks::run(jobs, [](const JobSP &job) {
  //Some actual work here
}, RestrictionType::Custom, "jobFetch");
```

Future interface
----------------
Every transformation except first two return new `FutureSP`, so any recoverings down the chain will not affect failure/success state of top ones.
Once value (or error) is set in `FutureSP` - it becomes immutable in terms of value, error and failure/success state.
`FutureSP` can't be created directly, the only way to do it is to use `PromiseSP` and its `failure()`/`success()` methods or static methods `Future::successful()`/`Future::fail()`

 * **onSuccess**/**forEach**: `(T->void) -> FutureSP<T>` (returns itself)
 * **onFailure**: `(Failure->void) -> FutureSP<T>` (returns itself)
 * **map**: `(T->U) -> FutureSP<U>`
 * **filter**: `(T->bool) -> FutureSP<T>`
 * **flatMap**: `(T->FutureSP<U>) -> FutureSP<U>`
 * **andThen**: `(void->FutureSP<U>) -> FutureSP<U>`
 * **andThenValue**: `U -> FutureSP<U>`
 * **recover**: `(Failure->T) -> FutureSP<T>`
 * **recoverWith**: `(Failure->FutureSP<T>) -> FutureSP<T>`
 * **zip**: `(FutureSP<U>, ...) -> FutureSP<std::tuple<T, U, ...>>` (if T is already a tuple then U will be added to it to avoid tuples nesting)
 * **zipValue**: `U -> FutureSP<std::tuple<T, U>>` (if T is already a tuple then U will be added to it to avoid tuples nesting)

Sequencing array of futures into future of array can be done via static `Future::sequence()` method.

WithFailure helper struct
-------------------------
`WithFailure` objects can be used to fail inside transformations that don't return FutureSP. It has cast operators to T and FutureSP<T> that effectively sets error on returning from function (and shouldn't be used anymore, don't store them or reuse).

CancelableFuture helper struct
------------------------------
`CancelableFuture` mimics FutureSP and also adds ability to send cancel intention upstream. There is no guarantee that upstream supports cancelation though.
Only option to create `CancelableFuture` is from `PromiseSP` so only upstream can do it.

Tasks scheduling interface
--------------------------
All tasks are working on top of thread pool.
Main method for tasks starting is `tasks::run()` which returns `CancelableFuture`. It has few overloads that allows to set restrictor for task and/or run same task for array of data.
Tasks can be canceled only before they started their execution.
Restrictors can be used to make subthreadpools.

 * `Custom` restrictor type is simplest one and uses its name to separate different subpools. Size of subpool can be set via `addCustomRestrictor()`
 * `Intensive` type always use the same subpool for all names (so one can omit this attribute at all) and subpool size is determined in runtime by number of available cores
 * `ThreadBound` type always run tasks with same restrictor name on same thread. It doesn't exclusively block this thread from other tasks though, so it is safe to create as many different `ThreadBound` restrictors as needed.

Dependencies
------------
The only real dependency is Qt5. Code should be compilable by any modern compiler that supports C++14 (tested at GCC 4.9.2, Clang 6.0, MSVC 2017).
