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
auto area = width->result() * height->result();
```

#### Let's shorten it a bit, result automatically waits
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
auto area = width->result() * height->result();
```

#### Boooring. Why wait when we can transform?
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
auto area = width->zip(height)->map([](const auto &result) {
  return std::get<0>(result) * std::get<1>(result);
});
```

#### Ok, but what if our calculation is complicated enough and is inside some function, that returns future: `(double, double) -> FutureSP<double>`?
```c++
FutureSP<double> width = /*...*/;
FutureSP<double> height = /*...*/;
auto area = width->zip(height)->flatMap([](const auto &result) {
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
auto first = tasks::run([] {
  //Long task
  return 42;
});
auto second = tasks::run([] {
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
