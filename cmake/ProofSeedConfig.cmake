include(CMakeFindDependencyMacro)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdparty")
find_dependency(Threads REQUIRED)
find_dependency(Qt5Core CONFIG REQUIRED)
find_dependency(Qt5Concurrent CONFIG REQUIRED)
find_dependency(asynqro CONFIG REQUIRED)
list(REMOVE_AT CMAKE_PREFIX_PATH -1)

if(NOT TARGET Proof::Seed)
    include("${CMAKE_CURRENT_LIST_DIR}/ProofSeedTargets.cmake")
endif()
