cmake_minimum_required(VERSION 3.12.0)
if(NOT PROOF_FULL_BUILD)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/modules")
    include(ProofInternal)
endif()

project(ProofSeed VERSION ${PROOF_VERSION} LANGUAGES CXX)

find_package(Qt5Core CONFIG REQUIRED)
if(NOT PROOF_FULL_BUILD)
    proof_init()
    find_package(proof-gtest CONFIG REQUIRED)
    enable_testing()
endif()

set(ASYNQRO_QT_SUPPORT ON CACHE BOOL "Qt support in asynqro" FORCE)
set(ASYNQRO_BUILD_TESTS OFF CACHE BOOL "Asynqro tests" FORCE)
add_subdirectory(3rdparty/asynqro)

proof_add_target_sources(Seed
    src/proofseed/tasks.cpp
)

proof_add_target_headers(Seed
    include/proofseed/planting.h
    include/proofseed/proofalgorithms.h
    include/proofseed/asynqro_extra.h
    include/proofseed/proofseed_global.h
    include/proofseed/tasks.h
)

if (PROOF_CLANG_TIDY)
    proof_add_target_sources(Seed src/proofseed/dummy.cpp)
endif()

proof_add_module(Seed
    QT_LIBS Core
    OTHER_LIBS asynqro::asynqro
)

add_subdirectory(tests/proofseed)

