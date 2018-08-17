TEMPLATE = app
TARGET = seed_tests

CONFIG += c++14 console
macx:CONFIG -= app_bundle
QT += core testlib
QT -= gui

LIBS += -lproof-gtest -lProofSeed

win32:CONFIG *= skip_target_version_ext
versionAtLeast(QT_VERSION, 5.11.0):!msvc:CONFIG += qtquickcompiler

INCLUDEPATH += include .

SOURCES += \
    tests/proofseed/main.cpp \
    tests/proofseed/algorithms_test.cpp \
    tests/proofseed/future_test.cpp \
    tests/proofseed/tasks_test.cpp \
    tests/proofseed/algorithms_map_test.cpp \
    tests/proofseed/algorithms_flatten_test.cpp


PROOF_PATH = $$(PROOF_PATH)
!isEmpty(PROOF_PATH) {
    # Part of Proof
    message(Building proofseed tests as part of Proof framework)
    DESTDIR = $$PROOF_PATH/tests
    INCLUDEPATH += $$PROOF_PATH/include
    LIBS += -L$$PROOF_PATH/lib
}
