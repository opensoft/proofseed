cmake_minimum_required(VERSION 3.10.3)
project(ProofSeedTest LANGUAGES CXX)

proof_add_target_sources(seed_test
    algorithms_test.cpp
    future_test.cpp
    tasks_test.cpp
    algorithms_map_test.cpp
    algorithms_flatten_test.cpp
    future_exceptions_test.cpp
)

proof_add_test(seed_test
    PROOF_LIBS Seed
)
