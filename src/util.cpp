#include "ophelib/util.h"

#include <sstream>
#include <unistd.h>

#ifndef OPHELIB_VERSION
#define OPHELIB_VERSION unknown-version
#endif

#ifndef OPHELIB_GIT_REF
#define OPHELIB_GIT_REF unknown-ref
#endif

namespace ophelib {
    const Integer nCr(const Integer n_, const Integer r_) {
        Integer n = n_, r;
        if(r_ > n_ / 2)
            r = n_ - r_; // because C(n, r) == C(n, n - r)
        else
            r = r_;
        Integer ret = 1;

        for(int i = 1; i <= r; i++) {
            ret *= n - r + i;
            ret /= i;
        }

        return ret;
    }

    long long nCr(const long &n_, const long &r_) {
        long n = n_, r;
        if(r_ > n_ / 2)
            r = n_ - r_; // because C(n, r) == C(n, n - r)
        else
            r = r_;
        long long ret = 1;

        for(int i = 1; i <= r; i++) {
            ret *= n - r + i;
            ret /= i;
        }

        return ret;
    }

    std::string temp_name() {
        std::string templ = P_tmpdir;
        templ += "/ophelibXXXXXX";
        char fname[PATH_MAX];
        strcpy(fname, templ.c_str());
        int fd = mkstemp(fname);
        close(fd);
        unlink(fname);
        return std::string(fname);
    }

    std::string ophelib_ref() {
         return std::string(STR(OPHELIB_GIT_REF));
    }

    std::string ophelib_version() {
        return std::string(STR(OPHELIB_VERSION));
    }

    bool ophelib_openmp_enabled() {
#ifdef OPHELIB_OPENMP_ENABLED
        return true;
#else
        return false;
#endif
    }

    StopWatch::StopWatch(std::string name_, int n_iter_)
            : name(name_),
              n_iter(n_iter_) {}

    void StopWatch::header() {
        std::cerr << "# t_s: total execution time, seconds" << std::endl;
        std::cerr << "# dt_ms: execution time divided by n_iter_, ms" << std::endl;
        std::cout << "name; n_iter_; t_s; dt_ms;" << std::endl;
    }

    void StopWatch::start() {
        t0 = std::chrono::high_resolution_clock::now();
    }

    long long StopWatch::stop(const bool print) {
        t1 = std::chrono::high_resolution_clock::now();
        d = std::chrono::duration_cast<std::chrono::microseconds>( t1 - t0 ).count();
        if(print) {
            auto d_s = ((double)d)/1e6;
            auto dt_ms = d_s * 1e3 / n_iter;
            std::cout << name << "; " << n_iter << "; " << d_s;
            std::cout << "; " << dt_ms << ";" << std::endl;
        }
        return d;
    }
}
