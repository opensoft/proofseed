TEMPLATE = lib
TARGET = ProofSeed
CONFIG += qt c++14
QT += core
QT -= gui

DEFINES += PROOF_SEED_LIB
INCLUDEPATH += $$PWD include

win32:CONFIG *= skip_target_version_ext
msvc {
    # QtCore/qvector.h(656): warning C4127: conditional expression is constant
    QMAKE_CXXFLAGS += /wd4127
}

HEADERS += \
    include/proofseed/planting.h \
    include/proofseed/proofalgorithms.h \
    include/proofseed/future.h \
    include/proofseed/proofseed_global.h \
    include/proofseed/spinlock.h \
    include/proofseed/tasks.h \
    include/proofseed/helpers/prooftypetraits.h \
    include/proofseed/helpers/tuplemaker.h

SOURCES += \
    src/future.cpp \
    src/spinlock.cpp \
    src/tasks.cpp

PROOF_PATH = $$(PROOF_PATH)
isEmpty(PROOF_PATH) {
    # Standalone
    message(Building proofseed as standalone library)
    target.path = $$PREFIX/lib/
    headers.path = $$PREFIX/include/proofseed/
    headers.files = include/proofseed/*.h
    helpers_headers.path = $$PREFIX/include/proofseed/helpers/
    helpers_headers.files = include/proofseed/helpers/*.h
    3rdparty_optional_headers.path = $$PREFIX/include/3rdparty/optional/
    3rdparty_optional_headers.files = 3rdparty/optional/*.hpp
    INSTALLS += target headers helpers_headers 3rdparty_optional_headers
} else {
    # Part of Proof
    message(Building proofseed as part of Proof framework)
    include($$PROOF_PATH/proof.pri)
    DESTDIR = $$BUILDPATH/lib
}

