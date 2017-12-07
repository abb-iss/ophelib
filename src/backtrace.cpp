#include "ophelib/backtrace.h"

namespace ophelib {

    std::vector<std::string> split(const std::string &s, const std::string &delim) {
        std::stringstream strm(s);
        std::string line;
        std::vector<std::string> words;
        while (std::getline(strm, line)) {
            std::size_t prev = 0, pos;
            while ((pos = line.find_first_of(delim, prev)) != std::string::npos) {
                if (pos > prev)
                    words.push_back(line.substr(prev, pos - prev));
                prev = pos + 1;
            }
            if (prev < line.length())
                words.push_back(line.substr(prev, std::string::npos));
        }
        return words;
    }

    std::string demangle_line(std::string signature) {
        int status;

        /* contains three parts, before, inside and after parentheses */
        std::vector<std::string> split_line = split(signature, "()");
        /* contains two parts, name and offset */
        std::vector<std::string> split_location = split(split_line[1], "+");

        if(split_line.size() < 3) {
            split_line.push_back(split_line[1]);
            split_line[1] = "";
        }

        std::ostringstream o("");
        if(split_location.size() == 1) {
            o << split_line[0] << '(' << split_location[0] << ')' << split_line[2];
        } else {
            char *demangled = abi::__cxa_demangle(split_location[0].c_str(), 0, 0, &status);
            const std::string method_name = (demangled == NULL) ? split_location[0] : demangled;
            o << split_line[0] << '(' << method_name << '+' << split_location[1] << ')' << split_line[2];
        }

        return o.str();
    }

    std::string get_backtrace(void **backtrace, int backtrace_size, int skip) {
        char **messages = backtrace_symbols(backtrace, backtrace_size);

        if(skip > backtrace_size)
            skip = backtrace_size;

        std::ostringstream o("");
        for (int i = skip; i < backtrace_size && messages != NULL; i++) {
            o << "[bt]: (" << i << ") " << demangle_line(messages[i]) << std::endl;
        }
        free(messages);
        return o.str();
    }

    void crit_err_hdlr(int sig_num, siginfo_t *info, void *ucontext) {
        void *array[50];
        void *caller_address;
        int size;
        sig_ucontext_t *uc;

        uc = (sig_ucontext_t *) ucontext;

        /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
        caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
        caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture.
#endif

        fprintf(stderr, "--- signal %d (%s), address is %p from %p ---\n",
                sig_num, strsignal(sig_num), info->si_addr,
                (void *) caller_address);

        size = backtrace(array, 50);

        /* overwrite sigaction with caller's address */
        array[1] = caller_address;

        std::cerr << get_backtrace(array, size);
        exit(EXIT_FAILURE);
    }

    void register_segfault_handler() {
        struct sigaction sigact;

        sigact.sa_sigaction = crit_err_hdlr;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        if(sigaction(SIGSEGV, &sigact, (struct sigaction *) NULL) != 0) {
            fprintf(stderr, "error setting signal handler for %d (%s)\n",
                    SIGSEGV, strsignal(SIGSEGV));

            exit(EXIT_FAILURE);
        }

        if(sigaction(SIGABRT, &sigact, (struct sigaction *) NULL) != 0) {
            fprintf(stderr, "error setting signal handler for %d (%s)\n",
                    SIGABRT, strsignal(SIGABRT));

            exit(EXIT_FAILURE);
        }
    }
}