#include "ophelib/integer.h"
#include "ophelib/wire.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace ophelib {
    size_t get_filesize(const std::string &fname) {
        struct stat st;
        if(stat(fname.c_str(), &st) != 0)
            error_exit("cannot stat file " + fname);
        return (size_t) st.st_size;
    }

    template<typename T>
    void serialize_to_file(const T &t, const std::string &fname) {
        std::ofstream file(fname);
        if(!file.is_open())
            error_exit("cannot open file :" + fname);

        #ifdef DEBUG_SER
        std::cout << "serialize_to_file<" << typeid(T).name() << ">()" << std::endl;
        #endif

        /// @TODO maybe use buffer identifier
        //Wire::FinishCiphertextBuffer(builder, x);

        flatbuffers::FlatBufferBuilder builder;
        auto x = serialize(builder, t);
        builder.Finish(x);

        file.write((const char *) builder.GetBufferPointer(), builder.GetSize());
        if(file.bad())
            error_exit("could not write to file " + fname);
        file.close();
    }

    template void serialize_to_file(const Integer &t, const std::string &fname);
    template void serialize_to_file(const Ciphertext &t, const std::string &fname);
    template void serialize_to_file(const PackedCiphertext &t, const std::string &fname);
    template void serialize_to_file(const Vec<float> &t, const std::string &fname);
    template void serialize_to_file(const Vec<Integer> &t, const std::string &fname);
    template void serialize_to_file(const Vec<Ciphertext> &t, const std::string &fname);
    template void serialize_to_file(const Vec<PackedCiphertext> &t, const std::string &fname);
    template void serialize_to_file(const Mat<float> &t, const std::string &fname);
    template void serialize_to_file(const Mat<Integer> &t, const std::string &fname);
    template void serialize_to_file(const Mat<Ciphertext> &t, const std::string &fname);
    template void serialize_to_file(const PublicKey &t, const std::string &fname);
    template void serialize_to_file(const PrivateKey &t, const std::string &fname);
    template void serialize_to_file(const KeyPair &t, const std::string &fname);

    template<typename T>
    void deserialize_from_file(const std::string &fname, T &out) {
        const int fd = open(fname.c_str(), O_RDONLY, 0);
        if(fd == -1)
            error_exit("cannot open file " + fname);

        #ifdef DEBUG_SER
        std::cout << "deserialize_from_file<" << typeid(T).name() << ">()" << std::endl;
        #endif

        /* mmap */
        const auto filesize = get_filesize(fname);
        void* mapped = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
        if(mapped == MAP_FAILED)
            error_exit("cannot mmap file " + fname);

        /// @TODO maybe use buffer identifier
        //if(!Wire::CiphertextBufferHasIdentifier(mapped))
        //    error_exit("no identifier found!");

        /* call deserialization implementation */
        deserialize(mapped, out);

        /* cleanup */
        const int rc = munmap(mapped, filesize);
        if(rc != 0)
            error_exit("error munmapping file " + fname);
        close(fd);
    }

    template void deserialize_from_file(const std::string &fname, Integer &out);
    template void deserialize_from_file(const std::string &fname, Ciphertext &out);
    template void deserialize_from_file(const std::string &fname, PackedCiphertext &out);
    template void deserialize_from_file(const std::string &fname, Vec<float> &out);
    template void deserialize_from_file(const std::string &fname, Vec<Integer> &out);
    template void deserialize_from_file(const std::string &fname, Vec<Ciphertext> &out);
    template void deserialize_from_file(const std::string &fname, Vec<PackedCiphertext> &out);
    template void deserialize_from_file(const std::string &fname, Mat<float> &out);
    template void deserialize_from_file(const std::string &fname, Mat<Integer> &out);
    template void deserialize_from_file(const std::string &fname, Mat<Ciphertext> &out);
    template void deserialize_from_file(const std::string &fname, PublicKey &out);
    template void deserialize_from_file(const std::string &fname, PrivateKey &out);
    template void deserialize_from_file(const std::string &fname, KeyPair &out);

    template<typename T>
    const T deserialize_from_file(const std::string &fname) {
        T ret;
        deserialize_from_file(fname, ret);

        return ret;
    }

    template const Integer deserialize_from_file(const std::string &fname);
    template const Ciphertext deserialize_from_file(const std::string &fname);
    template const PackedCiphertext deserialize_from_file(const std::string &fname);
    template const Vec<float> deserialize_from_file(const std::string &fname);
    template const Vec<Integer> deserialize_from_file(const std::string &fname);
    template const Vec<Ciphertext> deserialize_from_file(const std::string &fname);
    template const Vec<PackedCiphertext> deserialize_from_file(const std::string &fname);
    template const Mat<float> deserialize_from_file(const std::string &fname);
    template const Mat<Integer> deserialize_from_file(const std::string &fname);
    template const Mat<Ciphertext> deserialize_from_file(const std::string &fname);
    template const PublicKey deserialize_from_file(const std::string &fname);
    template const PrivateKey deserialize_from_file(const std::string &fname);
    template const KeyPair deserialize_from_file(const std::string &fname);

    flatbuffers::Offset<Wire::Integer> serialize(flatbuffers::FlatBufferBuilder &builder, const ophelib::Integer &i) {
        using namespace Wire::IntegerHelpers;

        const auto n_bits = mpz_sizeinbase(i.get_mpz_t(), 2);
        const auto count = (n_bits + EXPORT_NUM_BITS-1) / EXPORT_NUM_BITS;

        #ifdef DEBUG_SER
        std::cout << "serialize<Integer>(" << i.to_string(false) << ")" << std::endl;
        std::cout << "EXPORT_SIZE="<< EXPORT_SIZE << std::endl;
        std::cout << "EXPORT_NUM_BITS="<< EXPORT_NUM_BITS << std::endl;
        std::cout << "count="<< count << std::endl;
        std::cout << "n_bits="<< n_bits << std::endl;
        std::cout << "i.size_bits() = "<< i.size_bits() << std::endl;
        std::cout << "vector_type_p = " << typeid(vector_type_p).name() << std::endl;
        std::cout << "vector_type_cr = " << typeid(vector_type_cr).name() << std::endl;
        std::cout << "vector_type = " << typeid(vector_type).name() << std::endl;
        std::cout << "int_scalar_type = " << typeid(int_scalar_type).name() << std::endl;
        #endif

        std::vector<int_scalar_type> data;
        data.resize(count, 0);
        size_t countp;
        mpz_export(data.data(), &countp, EXPORT_ORDER, EXPORT_SIZE, EXPORT_ENDIAN, EXPORT_NAILS, i.get_mpz_t());
        if(countp != count && i != 0 && countp != 0)
            error_exit("unexpected size");
        auto data_vec = builder.CreateVector(data.data(), count);
        return Wire::CreateInteger(builder, n_bits, i < 0, data_vec);
    }

    void deserialize(const void* buf, Integer &out) {
        deserialize(Wire::GetInteger(buf), out);
    }

    void deserialize(const Wire::Integer *i, Integer &out) {
        using namespace Wire::IntegerHelpers;

        const auto n_bits = i->n_bits();
        const auto count = (n_bits + EXPORT_NUM_BITS-1) / EXPORT_NUM_BITS;

        const auto data = i->data();
        mpz_import(out.get_mpz_t(), count, EXPORT_ORDER, EXPORT_SIZE, EXPORT_ENDIAN, EXPORT_NAILS, data->data());
        if(i->neg())
            mpz_neg(out.get_mpz_t(), out.get_mpz_t());

        #ifdef DEBUG_SER
        std::cout << "deserialize(const Wire::Integer *i, Integer &out)" << std::endl;
        std::cout << "EXPORT_SIZE="<< EXPORT_SIZE << std::endl;
        std::cout << "EXPORT_NUM_BITS="<< EXPORT_NUM_BITS << std::endl;
        std::cout << "count="<< count << std::endl;
        std::cout << "n_bits="<< n_bits << std::endl;
        if(out.size_bits() != n_bits)
            error_exit("invalid size imported!");
        #endif
    }

    flatbuffers::Offset<Wire::Ciphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Ciphertext &c) {
        return Wire::CreateCiphertext(builder, serialize(builder, c.data));
    }

    void deserialize(const void* buf, Ciphertext &out) {
        deserialize(Wire::GetCiphertext(buf), out);
    }

    void deserialize(const Wire::Ciphertext *c, Ciphertext &out) {
        deserialize(c->data(), out.data);
    }

    void deserialize(const Wire::Ciphertext *c, Ciphertext &out, std::shared_ptr<Integer> n2_shared) {
        if(!n2_shared)
            error_exit("got a null pointer");
        deserialize(c->data(), out.data);
        out.n2_shared = n2_shared;
    }

    void deserialize(const Wire::Ciphertext *c, Ciphertext &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod) {
        if(!n2_shared)
            error_exit("got a null pointer");
        if(!fast_mod)
            error_exit("got a null pointer");
        deserialize(c->data(), out.data);
        out.n2_shared = n2_shared;
        out.fast_mod = fast_mod;
    }

    flatbuffers::Offset<Wire::PackedCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const PackedCiphertext &p) {
        return Wire::CreatePackedCiphertext(builder,
                                            p.n_plaintexts,
                                            p.plaintext_bits,
                                            serialize(builder, p.data));
    }

    void deserialize(const void* buf, PackedCiphertext &out) {
        deserialize(Wire::GetPackedCiphertext(buf), out);
    }

    void deserialize(const Wire::PackedCiphertext *p, PackedCiphertext &out) {
        deserialize(p->data(), out.data);
        out.n_plaintexts = p->n_plaintexts();
        out.plaintext_bits = p->plaintext_bits();
    }

    flatbuffers::Offset<Wire::VecFloat> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<float> &v) {
        const auto n = v.length();
        const auto vec = builder.CreateVector(v.data(), n);
        return Wire::CreateVecFloat(builder, n, vec);
    }

    void deserialize(const void* buf, Vec<float> &out) {
        deserialize(Wire::GetVecFloat(buf), out);
    }

    void deserialize(const Wire::VecFloat *v, Vec<float> &out) {
        const auto n = v->length();
        if(v->length() != v->data()->size())
            error_exit("invalid import dimensions!");

        out.SetLength(n);
        memcpy(out.data(), v->data()->data(), n * sizeof(float));
    }

    flatbuffers::Offset<Wire::VecInteger> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<Integer> &v) {
        const auto n = v.length();

        std::vector< flatbuffers::Offset<Wire::Integer> > vec;
        vec.reserve(n);
        for(auto i: v) {
            vec.push_back(serialize(builder, i));
        }
        return Wire::CreateVecInteger(builder, n, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Vec<Integer> &out) {
        deserialize(Wire::GetVecInteger(buf), out);
    }

    void deserialize(const Wire::VecInteger *v, Vec<Integer> &out) {
        const auto n = v->length();
        const auto vec = v->data();
        if(v->length() != vec->size())
            error_exit("invalid import dimensions!");

        out.SetLength(n);
        for(unsigned long i = 0; i < n; i++) {
            deserialize(vec->Get(i), out[i]);
        }
    }

    flatbuffers::Offset<Wire::VecCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<Ciphertext> &v) {
        const auto n = v.length();

        std::vector< flatbuffers::Offset<Wire::Ciphertext> > vec;
        vec.reserve(n);
        for(auto i: v) {
            vec.push_back(serialize(builder, i));
        }
        return Wire::CreateVecCiphertext(builder, n, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Vec<Ciphertext> &out) {
        deserialize(Wire::GetVecCiphertext(buf), out);
    }

    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out) {
        const auto n = v->length();
        const auto vec = v->data();
        if(v->length() != vec->size())
            error_exit("invalid import dimensions!");

        out.SetLength(n);
        for(unsigned long i = 0; i < n; i++) {
            deserialize(vec->Get(i), out[i]);
        }
    }

    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared) {
        if(!n2_shared)
            error_exit("got a null pointer");
        deserialize(v, out);
        const auto n = v->length();
        for(unsigned long i = 0; i < n; i++) {
            out[i].n2_shared = n2_shared;
        }
    }

    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod) {
        if(!n2_shared)
            error_exit("got a null pointer");
        if(!fast_mod)
            error_exit("got a null pointer");
        deserialize(v, out);
        const auto n = v->length();
        for(unsigned long i = 0; i < n; i++) {
            out[i].n2_shared = n2_shared;
            out[i].fast_mod = fast_mod;
        }
    }

    void deserialize(const void* buf, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared) {
        return deserialize(Wire::GetVecCiphertext(buf), out, n2_shared);
    }

    void deserialize(const void* buf, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod) {
        return deserialize(Wire::GetVecCiphertext(buf), out, n2_shared, fast_mod);
    }

    flatbuffers::Offset<Wire::VecPackedCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<PackedCiphertext> &v) {
        const auto n = v.length();

        std::vector< flatbuffers::Offset<Wire::PackedCiphertext> > vec;
        vec.reserve(n);
        for(auto i: v) {
            vec.push_back(serialize(builder, i));
        }
        return Wire::CreateVecPackedCiphertext(builder, n, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Vec<PackedCiphertext> &out) {
        deserialize(Wire::GetVecPackedCiphertext(buf), out);
    }

    void deserialize(const Wire::VecPackedCiphertext *v, Vec<PackedCiphertext> &out) {
        const auto n = v->length();
        const auto vec = v->data();
        if(v->length() != vec->size())
            error_exit("invalid import dimensions!");

        out.SetLength(n);
        for(unsigned long i = 0; i < n; i++) {
            deserialize(vec->Get(i), out[i]);
        }
    }

    flatbuffers::Offset<Wire::MatFloat> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<float> &mat) {
        const auto n = mat.NumRows();
        const auto m = mat.NumCols();

        std::vector< flatbuffers::Offset<Wire::VecFloat> > vec;
        vec.reserve(n);
        for(long i = 0; i < n; i++) {
            vec.push_back(serialize(builder, mat[i]));
        }
        return Wire::CreateMatFloat(builder, n, m, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Mat<float> &out) {
        deserialize(Wire::GetMatFloat(buf), out);
    }

    void deserialize(const Wire::MatFloat *mat, Mat<float> &out) {
        const auto n = mat->n_rows();
        const auto m = mat->n_cols();
        const auto vec = mat->data();

        if(mat->n_rows() != mat->data()->size())
            error_exit("invalid import dimensions!");

        out.SetDims(n, m);
        for(unsigned long i = 0; i < n; i++) {
            const auto row = vec->Get(i);
            if(row->length() != m)
                error_exit("invalid import dimensions!");
            deserialize(row, out[i]);
        }
    }

    flatbuffers::Offset<Wire::MatInteger> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<Integer> &mat) {
        const auto n = mat.NumRows();
        const auto m = mat.NumCols();

        std::vector< flatbuffers::Offset<Wire::VecInteger> > vec;
        vec.reserve(n);
        for(long i = 0; i < n; i++) {
            vec.push_back(serialize(builder, mat[i]));
        }
        return Wire::CreateMatInteger(builder, n, m, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Mat<Integer> &out) {
        deserialize(Wire::GetMatInteger(buf), out);
    }

    void deserialize(const Wire::MatInteger *mat, Mat<Integer> &out) {
        const auto n = mat->n_rows();
        const auto m = mat->n_cols();
        const auto vec = mat->data();

        if(mat->n_rows() != mat->data()->size())
            error_exit("invalid import dimensions!");

        out.SetDims(n, m);
        for(unsigned long i = 0; i < n; i++) {
            const auto row = vec->Get(i);
            if(row->length() != m)
                error_exit("invalid import dimensions!");
            deserialize(row, out[i]);
        }
    }

    flatbuffers::Offset<Wire::MatCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<Ciphertext> &mat) {
        const auto n = mat.NumRows();
        const auto m = mat.NumCols();

        std::vector< flatbuffers::Offset<Wire::VecCiphertext> > vec;
        vec.reserve(n);
        for(long i = 0; i < n; i++) {
            vec.push_back(serialize(builder, mat[i]));
        }
        return Wire::CreateMatCiphertext(builder, n, m, builder.CreateVector(vec));
    }

    void deserialize(const void* buf, Mat<Ciphertext> &out) {
        deserialize(Wire::GetMatCiphertext(buf), out);
    }

    void deserialize(const Wire::MatCiphertext *mat, Mat<Ciphertext> &out) {
        const auto n = mat->n_rows();
        const auto m = mat->n_cols();
        const auto vec = mat->data();

        if(mat->n_rows() != mat->data()->size())
            error_exit("invalid import dimensions!");

        out.SetDims(n, m);
        for(unsigned long i = 0; i < n; i++) {
            const auto row = vec->Get(i);
            if(row->length() != m)
                error_exit("invalid import dimensions!");
            deserialize(row, out[i]);
        }
    }

    flatbuffers::Offset<Wire::PublicKey> serialize(flatbuffers::FlatBufferBuilder &builder, const PublicKey &p) {
        return Wire::CreatePublicKey(builder,
                                     p.key_size_bits,
                                     serialize(builder, p.n),
                                     serialize(builder, p.g));
    }

    void deserialize(const void* buf, PublicKey &out) {
        deserialize(Wire::GetPublicKey(buf), out);
    }

    void deserialize(const Wire::PublicKey *p, PublicKey &out) {
        deserialize(p->n(), out.n);
        deserialize(p->g(), out.g);
        out.key_size_bits = p->key_size_bits();
    }

    flatbuffers::Offset<Wire::PrivateKey> serialize(flatbuffers::FlatBufferBuilder &builder, const PrivateKey &p) {
        return Wire::CreatePrivateKey(builder,
                                      p.a_bits,
                                      p.key_size_bits,
                                      serialize(builder, p.p),
                                      serialize(builder, p.q),
                                      serialize(builder, p.a));
    }

    void deserialize(const void* buf, PrivateKey &out) {
        deserialize(Wire::GetPrivateKey(buf), out);
    }

    void deserialize(const Wire::PrivateKey *p, PrivateKey &out) {
        deserialize(p->p(), out.p);
        deserialize(p->q(), out.q);
        deserialize(p->a(), out.a);
        out.a_bits = p->a_bits();
        out.key_size_bits = p->key_size_bits();
    }

    flatbuffers::Offset<Wire::KeyPair> serialize(flatbuffers::FlatBufferBuilder &builder, const KeyPair &k) {
        return Wire::CreateKeyPair(builder,
                                   serialize(builder, k.pub),
                                   serialize(builder, k.priv));
    }

    void deserialize(const void* buf, KeyPair &out) {
        deserialize(Wire::GetKeyPair(buf), out);
    }

    void deserialize(const Wire::KeyPair *k, KeyPair &out) {
        deserialize(k->pub(), out.pub);
        deserialize(k->priv(), out.priv);
    }
}
