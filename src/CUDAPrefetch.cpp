//===-- Frequent Path Loop Invariant Code Motion Pass ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// EECS583 F22 - This pass can be used as a template for your Frequent Path LICM
//               homework assignment. The pass gets registered as "fplicm".
//
// This pass performs loop invariant code motion, attempting to remove as much
// code from the body of a loop as possible.  It does this by either hoisting
// code into the preheader block, or by sinking code to the exit blocks if it is
// safe.
//
////===----------------------------------------------------------------------===//

// include LLVM header files
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"

// include NVVM header files
#include "nvvm.h"
#include "nvPTXCompiler.h"

// include standard header files
#include <vector>

using namespace llvm;


namespace Prefetch {
    struct InsertPrefetchPass : public FunctionPass {
        static char ID;

        InsertPrefetchPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) override {

            return false;
        }

    private:

    };
} // end of namespace Prefetch

char Prefetch::InsertPrefetchPass::ID = 0;
static RegisterPass<Prefetch::InsertPrefetchPass> X("cuda-prefetch", "Automatically Insert Prefetch Instructions in IR level",
                                            false, false);

