#pragma once

#include "OPHELib/integer.h"

#include <iostream>
#include <exception>
#include <sstream>

namespace OPHELib {

#ifdef OPHELIB_ENABLE_EXCEPTIONS

    class BaseException: public std::runtime_error {
        const std::string file;
        const int line;
    public:
        BaseException(const std::string message, const std::string file, const int line);
        virtual const char* what() const throw();
    };

    class MathException: public BaseException {

    public:
        MathException(const std::string &message, const std::string &file, const int line)
                : BaseException(message, file, line) { }
    };

    class NotImplementedException: public BaseException {

    public:
        NotImplementedException(const std::string &file, const int line)
                : BaseException("not implemented!", file, line) { }
    };

    class DimensionMismatchException: public BaseException {

    public:
        DimensionMismatchException(const std::string &file, const int line)
                : BaseException("dimension mismatch!", file, line) { }
    };

    #define error_exit(MESSAGE) throw OPHELib::BaseException(MESSAGE, __FILE__, __LINE__)
    #define math_error_exit(MESSAGE) throw OPHELib::MathException(MESSAGE, __FILE__, __LINE__)
    #define not_implemented() throw OPHELib::NotImplementedException(__FILE__, __LINE__)
    #define dimension_mismatch() throw OPHELib::DimensionMismatchException(__FILE__, __LINE__)

#else

    #define REQUIRE_THROWS_AS(x, y)

    void error_exit_impl(const std::string message, const std::string file, const int line);

    #define error_exit(MESSAGE) OPHELib::error_exit_impl(MESSAGE, __FILE__, __LINE__)
    #define math_error_exit(MESSAGE) OPHELib::error_exit_impl(MESSAGE, __FILE__, __LINE__)
    #define not_implemented() OPHELib::error_exit_impl("not implemented!", __FILE__, __LINE__)
    #define dimension_mismatch() OPHELib::error_exit_impl("dimension mismatch!", __FILE__, __LINE__)

#endif // OPHELIB_ENABLE_EXCEPTIONS
} // namespace OPHELib
