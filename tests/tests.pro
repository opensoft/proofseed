TEMPLATE = app
TARGET = proofseed_tests

CONFIG += c++14 console
macx:CONFIG -= app_bundle
QT += core testlib
QT -= gui

LIBS += -lproof-gtest -lProofSeed

win32:CONFIG *= skip_target_version_ext
versionAtLeast(QT_VERSION, 5.11.0):!msvc:CONFIG += qtquickcompiler

INCLUDEPATH += ../include ..

SOURCES += \
    main.cpp \
    algorithms_test.cpp \
    future_test.cpp \
    tasks_test.cpp \
    algorithms_map_test.cpp \
    algorithms_flatten_test.cpp


PROOF_PATH = $$(PROOF_PATH)
!isEmpty(PROOF_PATH) {
    # Part of Proof
    message(Building proofseed as part of Proof framework)
    DESTDIR = $$PROOF_PATH/tests
    INCLUDEPATH += $$PROOF_PATH/include
    LIBS += -L$$PROOF_PATH/lib
}
