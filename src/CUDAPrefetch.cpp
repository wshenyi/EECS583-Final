#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Support/Format.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
namespace Prefetch{
    struct CudaPrefetchPass: public ModulePass {

        CudaPrefetchPass() : ModulePass(ID) {
        }

        void getAnalysisUsage(AnalysisUsage &AU) const{
        }

        virtual bool runOnModule(Module &M) override{
            errs() << "running on module " <<M.getName() << "\n";
            auto managed_memories = get_managed_memory_symbol_and_size(M);
            for(auto i:managed_memories){
                errs() << "managed memory: symbol " << *i.symbol
                       << " size " << *i.size << " flag" << *i.flags << "\n";
                get_last_modified_place_for_symbol_in_main(i.symbol);
            }
            return false; // template code is just return false
        }

        struct ManagedMemoryInfo{
            ManagedMemoryInfo(Value* symbol, Value* size, Value* flags){
                this->symbol = symbol;
                this->size = size;
                this->flags = flags;
            }
            Value* symbol;
            Value* size;
            Value* flags;

        };

        Instruction* get_last_modified_place_for_symbol_in_main(Value* symbol){
            for(auto i:symbol->users()){
                errs() << "uses for " << *symbol << "\n";
                errs() << *i << "\n";
            }
            return nullptr;
        }

        std::vector<ManagedMemoryInfo> get_managed_memory_symbol_and_size(Module &M){
            std::vector<ManagedMemoryInfo> result = std::vector<ManagedMemoryInfo>();
            std::string funtion_wrapper_name = get_wrapper_cuda_malloc_name(M);
            errs() << "found cudaMallocManaged wrapper: " << get_wrapper_cuda_malloc_name(M) << "\n";
            for(Module::iterator F = M.begin(), me = M.end(); F != me; ++F) {
                for (Function::iterator bb = F->begin(), be = F->end(); bb != be; ++bb) {
                    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
                        if (i->getOpcode() == Instruction::Call) {
                            CallInst *call_inst = dyn_cast<CallInst>(i);
                            if (call_inst->getCalledFunction()->getName() == funtion_wrapper_name){
                                ManagedMemoryInfo tmp = ManagedMemoryInfo(call_inst->getOperand(0),
                                                                          call_inst->getOperand(1),
                                                                          call_inst->getOperand(2)
                                );
                                result.push_back(tmp);
                            }

                        }
                    }
                }
            }
            return result;
        }

        std::string get_wrapper_cuda_malloc_name(Module &M){
            for(Module::iterator F = M.begin(), me = M.end(); F != me; ++F) {
                for (Function::iterator bb = F->begin(), be = F->end(); bb != be; ++bb) {
                    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
                        if (i->getOpcode() == Instruction::Call) {
                            CallInst *call_inst = dyn_cast<CallInst>(i);
                            if (call_inst->getCalledFunction()->getName() == "cudaMallocManaged") {
                                return std::string(F->getName());
                            }

                        }
                    }
                }
            }
            return "";
        }

        static char ID;
    };
} // end of namespace Prefetch

char Prefetch::CudaPrefetchPass::ID = 0;
static RegisterPass<Prefetch::CudaPrefetchPass> X("cuda-prefetch", "Automatically Insert Prefetch Instructions in IR level",
                                            false, false);

