cmake_minimum_required(VERSION 3.10.3)
if(NOT DEFINED PROOF_FULL_BUILD)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/modules")
    include(ProofInternal)
    proof_init()
endif()

project(ProofSeed VERSION ${PROOF_VERSION} LANGUAGES CXX)

find_package(Qt5Core CONFIG REQUIRED)
if(NOT DEFINED PROOF_FULL_BUILD)
    find_package(proof-gtest CONFIG REQUIRED)
    enable_testing()
endif()

add_subdirectory(3rdparty/optional)

proof_add_target_sources(Seed
    src/proofseed/future.cpp
    src/proofseed/spinlock.cpp
    src/proofseed/tasks.cpp
)

proof_add_target_headers(Seed
    include/proofseed/planting.h
    include/proofseed/proofalgorithms.h
    include/proofseed/future.h
    include/proofseed/proofseed_global.h
    include/proofseed/spinlock.h
    include/proofseed/tasks.h
    include/proofseed/helpers/prooftypetraits.h
    include/proofseed/helpers/tuplemaker.h
)

proof_add_module(Seed
    QT_LIBS Core
    OTHER_LIBS optional
)

# TODO: return seed tests back after switch to android (compilation on gcc takes almost forever. only future_test.cpp takes around half an hour on -j6 and 3.5GB RAM)
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_subdirectory(tests/proofseed)
endif()

