#define ZEROERR_ENABLE_PFR
#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/print.h"
#include "zeroerr/unittest.h"

#ifdef HAVE_LLVM
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_os_ostream.h"

namespace zeroerr {

struct A {
    int a;
    int b;
};


void PrinterExt(Printer& P, A s, unsigned level, const char* lb, rank<1>) {
    P.os << "A(" << s.a << ", " << s.b << ")";
}


template <typename T>
typename std::enable_if<
    std::is_base_of<llvm::Value, T>::value || std::is_base_of<llvm::Type, T>::value, void>::type
PrinterExt(Printer& P, T* s, unsigned level, const char* lb, rank<2>) {
    if (s == nullptr) {
        P.os << P.tab(level) << "nullptr" << lb;
    } else {
        llvm::raw_os_ostream os(P.os);
        s->print(os);
    }
}

template <typename T>
typename std::enable_if<std::is_base_of<llvm::Function, T>::value, void>::type PrinterExt(
    Printer& P, T* s, unsigned level, const char* lb, rank<3>) {
    if (s == nullptr) {
        P.os << P.tab(level) << "nullptr" << lb;
    } else {
        llvm::raw_os_ostream os(P.os);
        os << P.tab(level) << "Function " << FgCyan << *s << Reset;
    }
}


}  // namespace zeroerr


TEST_CASE("first check the PrinterExt function support") {
    CHECK(zeroerr::detail::has_extension<zeroerr::A>::value == true);

    auto print = zeroerr::getStderrPrinter();
    print("Hello");
    print(zeroerr::A{1, 2});
}


TEST_CASE("customize printing of LLVM pointers") {
    llvm::LLVMContext        context;
    std::vector<llvm::Type*> args   = {llvm::Type::getInt32Ty(context)};
    llvm::Module*            module = new llvm::Module("test_module", context);

    auto* f =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false),
                               llvm::GlobalValue::ExternalLinkage, "test", module);
    dbg(dbg(f)->getType());
}

#endif