#include "ophelib/wire.h"

using namespace std;
using namespace ophelib;

void help() {
    cerr << "Utility to dump serialized ophelib files to console." << endl;
    cerr << "Usage: ophelib_dump <type> <filename>" << endl;
    cerr << "    Where type is:" << endl;
    cerr << "    I  -> Integer" << endl;
    cerr << "    C  -> Ciphertext" << endl;
    cerr << "    P  -> PackedCiphertext" << endl;
    cerr << "    Vf -> Vec<float>" << endl;
    cerr << "    VI -> Vec<Integer>" << endl;
    cerr << "    VC -> Vec<Ciphertext>" << endl;
    cerr << "    VP -> Vec<PackedCiphertext>" << endl;
    cerr << "    Mf -> Mat<float>" << endl;
    cerr << "    MI -> Mat<Integer>" << endl;
    cerr << "    MC -> Mat<Ciphertext>" << endl;
    cerr << "    PK -> PublicKey" << endl;
    cerr << "    SK -> PrivateKey" << endl;
    cerr << "    KP -> KeyPair" << endl;
    exit(1);
}

int main (int argc, char **argv) {
    if(argc != 3)
        help();

    string type = argv[1];
    string file = argv[2];

    if (type == "I") {
        Integer x = deserialize_from_file<Integer>(file);
        cout << x.to_string(false) << endl;
    } else if (type == "C") {
        Ciphertext x = deserialize_from_file<Ciphertext>(file);
        cout << x.to_string(false) << endl;
    } else if (type == "P") {
        PackedCiphertext x = deserialize_from_file<PackedCiphertext>(file);
        cout << x.to_string(false) << endl;
    } else if (type == "Vf") {
        Vec<float> x = deserialize_from_file<Vec<float>>(file);
        cout << x << endl;
    } else if (type == "VI") {
        Vec<Integer> x = deserialize_from_file<Vec<Integer>>(file);
        cout << x << endl;
    } else if (type == "VC") {
        Vec<Ciphertext> x = deserialize_from_file<Vec<Ciphertext>>(file);
        cout << x << endl;
    } else if (type == "VP") {
        Vec<PackedCiphertext> x = deserialize_from_file<Vec<PackedCiphertext>>(file);
        cout << x << endl;
    } else if (type == "Mf") {
        Mat<float> x = deserialize_from_file<Mat<float>>(file);
        cout << x << endl;
    } else if (type == "MI") {
        Mat<Integer> x = deserialize_from_file<Mat<Integer>>(file);
        cout << x << endl;
    } else if (type == "MC") {
        Mat<Ciphertext> x = deserialize_from_file<Mat<Ciphertext>>(file);
        cout << x << endl;
    } else if (type == "PK") {
        PublicKey x = deserialize_from_file<PublicKey>(file);
        cout << x.to_string(false) << endl;
    } else if (type == "SK") {
        PrivateKey x = deserialize_from_file<PrivateKey>(file);
        cout << x.to_string(false) << endl;
    } else if (type == "KP") {
        KeyPair x = deserialize_from_file<KeyPair>(file);
        cout << x.to_string(false) << endl;
    } else {
        cerr << "Invalid type '" << type << "'!" << endl;
        help();
    }

    return 0;
}
