#include "ophelib/ntl_conv.h"

#include <iostream>
#include <sstream>

namespace ophelib {
    void conv(ophelib::Integer & z, const NTL::RR & a) {
        /*
        ─╔══╗───╔═══╗
        ╔╬╣╠╝───║╔═╗║──────────╔╦╗
        ║║║║╔╗╔╗║╚══╦══╦═╦═╦╗─╔╣║║
        ╚╩╣║║╚╝║╚══╗║╔╗║╔╣╔╣║─║╠╩╝
        ─╔╣╠╣║║║║╚═╝║╚╝║║║║║╚═╝║
        ─╚══╩╩╩╝╚═══╩══╩╝╚╝╚═╗╔╝
        ───────────────────╔═╝║
        ───────────────────╚══╝
        */
        NTL::ZZ tmp;
        conv(tmp, a);
        std::ostringstream o(""); o << tmp;
        mpz_set_str(z.get_mpz_t(), o.str().c_str(), 10);
    }

    void conv(ophelib::Integer & z, const NTL::ZZ & a) {
        /*
        ─╔══╗───╔═══╗
        ╔╬╣╠╝───║╔═╗║──────────╔╦╗
        ║║║║╔╗╔╗║╚══╦══╦═╦═╦╗─╔╣║║
        ╚╩╣║║╚╝║╚══╗║╔╗║╔╣╔╣║─║╠╩╝
        ─╔╣╠╣║║║║╚═╝║╚╝║║║║║╚═╝║
        ─╚══╩╩╩╝╚═══╩══╩╝╚╝╚═╗╔╝
        ───────────────────╔═╝║
        ───────────────────╚══╝
        */
        std::ostringstream o(""); o << a;
        mpz_set_str(z.get_mpz_t(), o.str().c_str(), 10);
    }

    void conv(NTL::RR& z, const ophelib::Integer& a) {
        std::ostringstream o(""); o << a.to_string_();
        conv(z, o.str().c_str());
    }
}
