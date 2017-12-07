#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <cxxabi.h>
#include <execinfo.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include <vector>

namespace ophelib {

    /** @file */

    /* This structure mirrors the one found in /usr/include/asm/ucontext.h */
    typedef struct _sig_ucontext {
        unsigned long uc_flags;
        struct ucontext *uc_link;
        stack_t uc_stack;
        struct sigcontext uc_mcontext;
        sigset_t uc_sigmask;
    } sig_ucontext_t;

    /**
     * Split by any of the characters in delim
     * @param s String to split
     * @param delim Delimiters
     * @return Vector of splits
     */
    std::vector<std::string> str_split(const std::string &s, const std::string &delim);

    /**
     * Demangle a GCC-style mangled C++ function signature
     */
    std::string demangle_line(std::string signature);

    /**
     * Per default, skip 1st frame as it points here.
     * @param backtrace to read from
     * @param backtrace_size number of elements in backtrace
     * @param skip how many initial frames to skip
     * @return stringified backtrace
     */
    std::string get_backtrace(void **backtrace, int backtrace_size, int skip = 1);

    /**
     * Our segfault handler. You don't have to use it directly,
     * but can use the helper function register_segfault_handler.
     */
    void crit_err_hdlr(int sig_num, siginfo_t *info, void *ucontext);

    /**
     * If you want to see a nice stack trace when a segfault occurs,
     * just #include "ophelib/backtrace.h" and call this function in
     * your main() or whereever before running your code.
     * Works only with GCC (maybe also in Clang?).
     */
    void register_segfault_handler();
}
