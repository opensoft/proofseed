cmake_minimum_required(VERSION 3.12.0)
project(ProofSeedTest LANGUAGES CXX)

proof_add_target_sources(seed_tests
    algorithms_test.cpp
    tasks_test.cpp
    algorithms_map_test.cpp
    algorithms_flatten_test.cpp
)

proof_add_test(seed_tests
    PROOF_LIBS Seed
)
