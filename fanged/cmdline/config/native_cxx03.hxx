//===-----------------------------------------------------------*- C++ -*-===//
// (C) Copyright 2010 Bryce Lelbach
//
// Use, modification and distribution of this software is subject to the Boost
// Software License, Version 1.0.
//
// Relative to repository root: /credit/BOOST_LICENSE_1_0.rst
// Online: http://www.boost.org/LICENSE_1_0.txt
//===----------------------------------------------------------------------===//

#if !defined(FANGED_CMDLINE_NATIVE_CXX03_HXX)
#define FANGED_CMDLINE_NATIVE_CXX03_HXX

#include <fanged/config_tags.hxx>

namespace fanged {
namespace cmdline {

struct native_cxx03 {
 public:
  template<class Tag> static typename Tag::type get (void) {
    return typename Tag::type();
  }
};

template<> tag::target::type
native_cxx03::get<tag::target> (void) {
  tag::target::type opt;

  opt.Triple = llvm::sys::getHostTriple();

  return opt;
}

template<> tag::diagnostic::type
native_cxx03::get<tag::diagnostic> (void) {
  tag::diagnostic::type opt;

  opt.ShowColors = true;
  opt.ShowOverloads = true;

  return opt;
}

template<> tag::language::type
native_cxx03::get<tag::language> (void) {
  tag::language::type opt;

  opt.CPlusPlus = true;
  opt.Bool = true;
  opt.RTTI = true;
  opt.Exceptions = true;
  opt.AccessControl = true;

  return opt;
}

} // cmdline 
} // fanged

#endif // FANGED_CMDLINE_NATIVE_CXX03_HXX
