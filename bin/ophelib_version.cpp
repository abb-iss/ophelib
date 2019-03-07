#include "ophelib/util.h"

using namespace std;
using namespace ophelib;

int main (int argc, char **argv) {
    cout << "Version: " << ophelib_version() << endl;
    cout << "Git ref: " << ophelib_ref() << endl;
    cout << " OpenMP: " << (ophelib_openmp_enabled() ? "'yes'" : "'no'") << endl;

    return 0;
}
