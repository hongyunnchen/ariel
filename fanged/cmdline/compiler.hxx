//===-----------------------------------------------------------*- C++ -*-===//
// (C) Copyright 2010 Bryce Lelbach
//
// Use, modification and distribution of this software is subject to the Boost
// Software License, Version 1.0.
//
// Relative to repository root: /credit/BOOST_LICENSE_1_0.rst
// Online: http://www.boost.org/LICENSE_1_0.txt
//===----------------------------------------------------------------------===//

#if !defined(FANGED_CMDLINE_COMPILER_HXX)
#define FANGED_CMDLINE_COMPILER_HXX

#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>

#include <llvm/LLVMContext.h>
#include <llvm/Config/config.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/Timer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/System/Host.h>
#include <llvm/System/Path.h>
#include <llvm/System/Program.h>
#include <llvm/System/Signals.h>
#include <llvm/Target/TargetSelect.h>

#include <boost/scoped_ptr.hpp>

#include <fanged/cmdline/config.hxx>
#include <fanged/cmdline/grammar.hxx>
#include <fanged/cmdline/stderr_bound_printer.hxx>

namespace fanged {
namespace cmdline {

template<
  class Config      = native_cxx03,
  class ClParser    = module_loader_grammar<std::string::iterator>,
  class DiagPrinter = stderr_bound_printer
> class compiler; 

template<class Config, class ClParser, class DiagPrinter>
class compiler: public clang::CompilerInstance {
 private:
  boost::scoped_ptr<llvm::sys::Path> path;

 public:
  static  void backend_error_handler (
    void* user_data, std::string const& message
  ) {
    clang::Diagnostic& diags = *static_cast<clang::Diagnostic*>(user_data);
    diags.Report(clang::diag::err_fe_error_backend) << message;
    exit(1);
  }

  compiler (int const ac, char const** av, void* main_addr):
    // get the full, canonical path of our binary
    path(new llvm::sys::Path(
      llvm::sys::Path::GetMainExecutable(av[0], main_addr)
    ))
  {
    // set up default options
    getTargetOpts()             = Config::template get<tag::target>();
    getFrontendOpts()           = Config::template get<tag::frontend>();
    getAnalyzerOpts()           = Config::template get<tag::analyzer>();
    getCodeGenOpts()            = Config::template get<tag::code_generation>();
    getDependencyOutputOpts()   = Config::template get<tag::dependency_output>();
    getDiagnosticOpts()         = Config::template get<tag::diagnostic>();
    getHeaderSearchOpts()       = Config::template get<tag::header_search>();
    getPreprocessorOpts()       = Config::template get<tag::preprocessor>();
    getPreprocessorOutputOpts() = Config::template get<tag::preprocessor_output>();
    getLangOpts()               = Config::template get<tag::language>();
    
    // set up diagnostics
    DiagPrinter* diag_printer = new DiagPrinter(getDiagnosticOpts());
    diag_printer->setPrefix(path->getBasename());
    setDiagnostics(new clang::Diagnostic(diag_printer));
    
    llvm::install_fatal_error_handler(
      backend_error_handler, static_cast<void*>(&getDiagnostics())
    );

    // an instance of the command line parsing grammar
    ClParser grammar(*this);

    // process the command line arguments
    for (int i = 1; i < ac; ++i) {
      // create a temporary string, because karma::parse requires a mutable
      // iterator to work on 
      std::string arg(av[i]);

      std::string::iterator it = arg.begin();

      if (!qi::parse(it, arg.end(), grammar) || (it != arg.end()))
        llvm::report_fatal_error(
          std::string("couldn't parse command line argument \"")
              .append(av[i]).append("\"")
        );
    }
    
    // set up the LLVM threading context
    setLLVMContext(new llvm::LLVMContext());

    // create file i/o objects using the options we've set up 
    createSourceManager();
    createFileManager();

    // set up the target from the target options
    setTarget(
      clang::TargetInfo::CreateTargetInfo(getDiagnostics(), getTargetOpts()
    ));

    // create the preprocessor and AST context objects
    createPreprocessor();
    createASTContext();
  } 

  virtual ~compiler (void) {
    llvm::remove_fatal_error_handler();
  }
};

} // cmdline 
} // fanged

#endif // FANGED_CMDLINE_COMPILER_HXX