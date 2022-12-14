#include "llvm/Pass.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"

using namespace llvm;
namespace Prefetch{
    struct CudaPrefetchPass: public ModulePass {

        CudaPrefetchPass() : ModulePass(ID) {
        }

        struct ManagedMemoryInfo{
            ManagedMemoryInfo(Value* Addr, Value* Size, Value* Flags):
                    addr(Addr), size(Size), flags(Flags){errs() << "Struct\n";}
            Value* addr;
            Value* size;
            Value* flags;

        };

        virtual bool runOnModule(Module &M) override{
            errs() << "running on module " <<M.getName() << "\n";
            auto managed_memories = get_managed_memory_addr_and_size(M);
            auto positions = get_insert_positions(M);
            for (auto p: positions) {
                errs() << *p << "\n";
                for(auto i:managed_memories){
//                    errs() << "managed memory: addr " << *i.addr
//                           << " size " << *i.size << " flag" << *i.flags << "\n";
//                get_last_modified_place_for_addr_in_main(i.addr);

                    auto &context = M.getContext();
                    auto *new_load = new LoadInst(Type::getFloatPtrTy(context), i.addr, "preload", p);
                    auto *new_cast = new BitCastInst(new_load, Type::getInt8PtrTy(context), "cast", p);
                    auto *nullpointer= ConstantPointerNull::get(PointerType::get(StructType::getTypeByName(context, "struct.CUstream_st"), 0));
                    Value* args[] = {new_cast, i.size, ConstantInt::get(Type::getInt32Ty(context), 0), nullpointer};
                    auto fun = M.getOrInsertFunction(StringRef("cudaMemPrefetchAsync"), Type::getInt32Ty(context),
                                                     new_cast->getType(), i.size->getType(), Type::getInt32Ty(context), nullpointer->getType());
                    auto *prefetch_ins = CallInst::Create(fun, args, "", p);
                }
            }
            return false; // template code is just return false
        }

        std::vector<Instruction*> get_insert_positions(Module &M){
            std::vector<Instruction*> positions;

            for(auto & F : M) {
                for (auto & bb : F) {
                    for (auto & i : bb) {
                        if (i.getOpcode() == Instruction::Call || i.getOpcode() == Instruction::Invoke) {
                            if(std::string(dyn_cast<CallInst>(&i)->getCalledFunction()->getName()).find("__device_stub__") != std::string::npos){
                                positions.emplace_back(&i);
                            }
                        }
                    }
                }
            }

            return std::move(positions);
        }

        FunctionCallee declarePrefetchFunc(LLVMContext& ctx,
                                           Module& module){
            if (StructType *st = StructType::getTypeByName(ctx, "struct.CUstream_st")) {
                Type* returnType = Type::getInt32Ty(ctx);
                std::vector<Type*> fArgs { Type::getInt8PtrTy(ctx),
                                      Type::getInt64Ty(ctx),
                                      Type::getInt32Ty(ctx),
                                      st->getPointerTo()
                };
                FunctionType* prefetch = FunctionType::get(returnType, fArgs, false);
                errs() << "Insert prefetch\n";
                // declare dso_local i32 @cudaMemPrefetchAsync(i8* noundef, i64 noundef, i32 noundef, %struct.CUstream_st* noundef) #1
                // pass (addr, size, 0, null), where addr require bitcast to i8* first
                return module.getOrInsertFunction(StringRef("cudaMemPrefetchAsync"), prefetch);
            }
            return FunctionCallee(); // how to deal with this ?
        }

        std::vector<ManagedMemoryInfo> get_managed_memory_addr_and_size(Module &M){
            std::vector<ManagedMemoryInfo> result = std::vector<ManagedMemoryInfo>();
            auto mm_define_fun = get_wrapper_cuda_malloc_fun(M);

            if (mm_define_fun != nullptr) {
                errs() << "found cudaMallocManaged wrapper: " << mm_define_fun->getName() << "\n";
                for (auto ins: mm_define_fun->users()) {
                    errs() << *ins << "\n";
                    result.emplace_back(ins->getOperand(0), ins->getOperand(1), ins->getOperand(2));
                }
            }else{
                errs() << "cudaMallocManaged not found \n";
            }

            return std::move(result);
        }

        Function* get_wrapper_cuda_malloc_fun(Module &M){
            for(Module::iterator F = M.begin(), me = M.end(); F != me; ++F) {
                for (Function::iterator bb = F->begin(), be = F->end(); bb != be; ++bb) {
                    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
                        if (i->getOpcode() == Instruction::Call) {
                            CallInst *call_inst = dyn_cast<CallInst>(i);
                            if (call_inst->getCalledFunction()->getName() == "cudaMallocManaged") {
                                return &*F;
                            }
                        }
                    }
                }
            }
            return nullptr;
        }

        static char ID;
    };
} // end of namespace Prefetch

char Prefetch::CudaPrefetchPass::ID = 0;
static RegisterPass<Prefetch::CudaPrefetchPass> X("cuda-prefetch", "Automatically Insert Prefetch Instructions in IR level",
                                            false, false);

