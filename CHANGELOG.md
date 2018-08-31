ProofSeed Changelog
===============

## Not Released
#### Features
 * `Future::innerFlatten()` added
 * trait `NestingLevel` added
 * `sieve` method added for "sieving" tuple and keep only specified elements

#### Bug Fixing
 * Fix for possible crash on tasks scheduler destruction

## 0.18.8.10
#### Features
 * reduceByMutation algorithm
 * `Future<>` overload added for shorter successful and sequence methods usage
 * Future::reduce renamed to innerReduce
 * Additional morphisms added to Future with container value: innerReduceByMutation, innerMap, innerFilter
 * Future::zipValue method added
 * Failure::withMessage, withCode, withData methods added

#### Bug Fixing
 * --

## 0.18.7.6
#### Features
 * flatten algorithm now reserves destination
 * forAll, exists and mapInPlace algorithms
 * toSet/toVector/toList/toKeysSet/toKeysVector/toKeysList/toValuesSet/toValuesVector/toValuesList algorithms
 * identity/keyIdentity/valueIdentity helper functions in algorithms
 * Future::andThenValue and Future::filter operations
 * CancelableFuture
 * Generic IsSpecialization trait
 * IsSpecialization and HasTypeParams traits moved out from detail namespace
 * Future::zip() can now accept any FutureSP-like (should contain Type type which contains Value type) arguments
 * tasks::RestrictionType::ThreadBound for tasks that should always be on exactly the same thread
 * Failure::Hints enum now has DataIsHttpCodeHint value to determine if data contains http status code

#### Bug Fixing
 * proper tasks finishing waiting in sequenceRunWithFailure test

## 0.18.4.12
#### Features
 * forEach algorithm
 * flatten algorithm
 * flatFilter algorithm
 * All algorithms can work with QJsonArray now (and any other container that provide either cbegin/cend, constBegin/constEnd or begin/end)
 * algorithms::map() overload for cases when index is also needed
 * tasks::run() overload for data containers with functor returning Future
 * tasks::run() overload for data containers with functor returning void
 * tasks::run() overload for data containers with index available in task lambda

#### Bug Fixing
 * --

## 0.18.3.14
Initial release
