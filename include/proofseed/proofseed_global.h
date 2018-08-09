#ifndef PROOFSEED_GLOBAL_H
#define PROOFSEED_GLOBAL_H

#include <QtGlobal>

#ifdef PROOF_SEED_LIB
#    define PROOF_SEED_EXPORT Q_DECL_EXPORT
#else
#    define PROOF_SEED_EXPORT Q_DECL_IMPORT
#endif

#endif // PROOFSEED_GLOBAL_H
