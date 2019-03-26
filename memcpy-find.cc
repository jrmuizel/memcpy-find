
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

void print_all(vector<tuple<CallInst*, uint64_t, Function*>> &memcpys) {
    std::sort(memcpys.begin(), memcpys.end(), [](auto& x, auto &y) { return get<1>(x) > get<1>(y); });
    for (auto& i : memcpys) {
        auto callInst = get<0>(i);
        auto size = get<1>(i);
        auto function = get<2>(i);
        cout << "memcpy" << " of " << size << " in " << function->getName().data() << " @ " << std::endl;
        MDNode* metadata = callInst->getMetadata("dbg");
        if (!metadata) {
            cout << "  no debug info" << endl;
            continue;
        }
        DILocation *debugLocation = dyn_cast<DILocation>(metadata);
        while (debugLocation) {
            DILocalScope *scope = debugLocation->getScope();
            cout << "  ";
            if (scope) {
                DISubprogram *subprogram = scope->getSubprogram();
                if (subprogram) {
                    const char* name = subprogram->getName().data();
                    cout << name << " ";
                }
            }

            cout << debugLocation->getFilename().data() << ":" << debugLocation->getLine() << std::endl;
            debugLocation = debugLocation->getInlinedAt();
        }
    }
}

void print_summary(vector<tuple<CallInst*, uint64_t, Function*>> &memcpys) {
    std::sort(memcpys.begin(), memcpys.end(), [](auto& x, auto &y) { return get<1>(x) > get<1>(y); });
    for (auto& i : memcpys) {
        auto callInst = get<0>(i);
        auto size = get<1>(i);
        auto function = get<2>(i);
        cout << "memcpy" << " of " << size << std::endl;
    }
}

int main(int argc, char **argv) {
    LLVMContext context;
    SMDiagnostic Err;

    const char *fileName;
    if (argc > 1) {
        fileName = argv[1];
    } else {
        printf("missing ir filename\n");
        abort();
    }
    bool summary = false;

    if (argc > 2) {
        if (std::string(argv[2]) == "summary") {
            summary = true;
        }
    }

    Expected<std::unique_ptr<Module> > m = parseIRFile(fileName, Err, context);
    if (!m) {
        errs() << toString(m.takeError()) << "\n";
    }
    vector<tuple<CallInst*, uint64_t, Function*>> memcpys;
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
                        memcpys.push_back({callInst, size, &function});
                    }
                }
            }
        }
    }
    if (summary) {
        print_summary(memcpys);
    } else {
        print_all(memcpys);
    }
}
