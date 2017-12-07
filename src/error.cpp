#include "ophelib/error.h"
#ifdef OPHELIB_ENABLE_EXCEPTIONS
    #include "ophelib/backtrace.h"
#endif

namespace ophelib {

#ifdef OPHELIB_ENABLE_EXCEPTIONS

    BaseException::BaseException(const std::string message_, const std::string file_, const int line_)
            : runtime_error(message_),
              file(file_),
              line(line_) { }

    const char *BaseException::what() const throw() {
        std::ostringstream o("");

        o << "Error: ";
        o << std::runtime_error::what();
        o << "; in " << file << ":" << line << std::endl;

        const size_t bt_size_max = 50;
        void *bt[bt_size_max];
        int size = backtrace(bt, bt_size_max);

        /* skip the first frames as they point to some libc internals */
        o << get_backtrace(bt, size, 5);

        std::string *cpy = new std::string(o.str());
        return cpy->c_str();
    }
#else

    void error_exit_impl(const std::string message, const std::string file, const int line) {
        std::cerr << "Error: " << message;
        std::cerr << "; in " << file << ":" << line << std::endl;
        exit(1);
    }

#endif // OPHELIB_ENABLE_EXCEPTIONS
} // namespace ophelib
