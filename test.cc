
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Debug.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include <iostream>

using namespace llvm;
using namespace std;
int main(int argc, char **argv) {
    LLVMContext context;
    SMDiagnostic Err;

    const char *fileName;
    if (argc > 1) {
            fileName = argv[1];
    } else {
            fileName = "out.ll";
    }

    Expected<std::unique_ptr<Module> > m = parseIRFile(fileName, Err, context);
    if (!m) {
        errs() << toString(m.takeError()) << "\n";
    }
    {
        auto &functionList = m->get()->getFunctionList();
        for (auto &function : functionList) {
            //printf("%s\n", function.getName().data());
            for (auto &bb : function) {
                for (auto &instruction : bb) {
                    //printf("  %s\n", instruction.getOpcodeName());
                    CallInst *callInst = dyn_cast<CallInst>(&instruction);

                    if (callInst == nullptr) {
                        continue;
                    }
                    //printf("have call\n");

                    Function *calledFunction = callInst->getCalledFunction();

                    if (calledFunction == nullptr) {
                        //printf("no calledFunction\n");
                        continue;
                    }

                    StringRef cfName = calledFunction->getName();
                    if (cfName.find("llvm.memcpy") != std::string::npos) {
                        auto size_operand = callInst->getOperand(2);
                        auto size_constant = dyn_cast<ConstantInt>(size_operand);
                        if (!size_constant) {
                                //printf("not constant\n");
                                continue;
                        }
                        auto size = size_constant->getValue().getLimitedValue();
                        MDNode* metadata = callInst->getMetadata("dbg");
                        if (!metadata) {
                                //printf("no dbg\n");
                                continue;
                        }
                        std::cout << cfName.data() << " of " << size << " in " << function.getName().data() << " @ " << std::endl;
                        DILocation *debugLocation = dyn_cast<DILocation>(metadata);
                        while (debugLocation) {
                                DILocalScope *scope = debugLocation->getScope();
                                if (scope) {
                                        DISubprogram *subprogram = scope->getSubprogram();
                                        if (subprogram) {
                                                const char* name = subprogram->getName().data();
                                                std::cout << "  " << name << " ";
                                        }
                                }
                                std::cout << "  " << debugLocation->getFilename().data() << ":" << debugLocation->getLine() << std::endl;
                                debugLocation = debugLocation->getInlinedAt();
                        }
                    } else {
                        //printf("not memcpy\n");
                    }
                }
            }
        }
    }

}
