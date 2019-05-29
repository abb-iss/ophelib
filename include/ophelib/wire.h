#pragma once

#include "ophelib/integer.h"
#include "ophelib/paillier_base.h"
#include "ophelib/packing.h"
#include "ophelib/error.h"
#include "ophelib/vector.h"

#include "ophelib/schemas/integer_generated.h"
#include "ophelib/schemas/ciphertext_generated.h"
#include "ophelib/schemas/packed_ciphertext_generated.h"
#include "ophelib/schemas/vec_float_generated.h"
#include "ophelib/schemas/vec_integer_generated.h"
#include "ophelib/schemas/vec_ciphertext_generated.h"
#include "ophelib/schemas/vec_packed_ciphertext_generated.h"
#include "ophelib/schemas/mat_float_generated.h"
#include "ophelib/schemas/mat_integer_generated.h"
#include "ophelib/schemas/mat_ciphertext_generated.h"
#include "ophelib/schemas/public_key_generated.h"
#include "ophelib/schemas/private_key_generated.h"
#include "ophelib/schemas/key_pair_generated.h"

#include <fstream>
#include <limits.h>
#include <unistd.h>

namespace ophelib {

    /**
     * Contains all the wrapper classes used for serialization
     * ("over-the-wire") of our data structures. Most stuff in
     * here is generated automatically by
     * [flatc](http://google.github.io/flatbuffers/index.html)
     * from the definition files in `include/ophelib/wire`.
     */
    namespace Wire {

        /**
         * Some helper structs and types for serializing and deserializing Integers
         */
        namespace IntegerHelpers {
            /**
             * Helper struct used to extract the type value from a container type.
             */
            template<typename T>
            struct extract_value_type {
                typedef T value_type;
            };

            /**
             * Helper struct used to extract the type value from a container type
             * two levels deep.
             */
            template<template<typename> class X, typename T>
            struct extract_value_type<X<T>> {
                typedef T value_type;
            };

            /*
             * This code deduces the type of the
             * values in Wire::Integers data field. This
             * type is then available as scalar_type.
             */
            typedef typename std::result_of<decltype(&Wire::Integer::data)(Wire::Integer)>::type vector_type_p;
            typedef decltype(*std::declval<vector_type_p>()) vector_type_cr;
            typedef std::remove_const<std::remove_reference<vector_type_cr>::type>::type vector_type;
            typedef typename extract_value_type<vector_type>::value_type int_scalar_type;

            /* constants for exporting and importing Integers */
            const size_t EXPORT_NAILS = 0; // don't skip anything
            const size_t EXPORT_SIZE = sizeof(int_scalar_type);
            const int EXPORT_ENDIAN = -1; // least significant first -> little endian
            const int EXPORT_ORDER = -1;  // least significant first -> "little endian"
            const size_t EXPORT_NUM_BITS = 8 * EXPORT_SIZE - EXPORT_NAILS;
        }
    }

    /**
     * Helper function: get file size in bytes
     */
    size_t get_filesize(const std::string &fname);

    /**
     * Serialize a data structure and write to file.
     *
     * @param t data to serialize
     * @param fname name of file to create
     */
    template<typename T>
    void serialize_to_file(const T &t, const std::string &fname);

    /**
     * Read data from a file and deserialize it.
     *
     * @param fname name of file to read from
     */
    template<typename T>
    const T deserialize_from_file(const std::string &fname);

    /**
     * Read data from a file and deserialize it.
     *
     * @param fname name of file to read from
     */
    template<typename T>
    void deserialize_from_file(const std::string &fname, T &out);

    /**
     * Serialize Integer
     */
    flatbuffers::Offset<Wire::Integer> serialize(flatbuffers::FlatBufferBuilder &builder, const Integer &i);

    /**
     * Deserialize Integer from raw buffer
     */
    void deserialize(const void* buf, Integer &out);

    /**
     * Deserialize Integer
     */
    void deserialize(const Wire::Integer *i, Integer &out);

    /**
     * Serialize Ciphertext
     */
    flatbuffers::Offset<Wire::Ciphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Ciphertext &c);

    /**
     * Deserialize Ciphertext from raw buffer
     */
    void deserialize(const void* buf, Ciphertext &out);

    /**
     * Deserialize Ciphertext
     */
    void deserialize(const Wire::Ciphertext *c, Ciphertext &out);

    /**
     * Deserialize Ciphertext
     * @param n2_shared set in deserialized ciphertext
     */
    void deserialize(const Wire::Ciphertext *c, Ciphertext &out, std::shared_ptr<Integer> n2_shared);

    /**
     * Deserialize Ciphertext
     *
     * @TODO save a copy of n^2 into serialization format, so we don't have to do this (get n^2 from outside on deserializaion)
     *
     * @param n2_shared set in deserialized ciphertext
     * @param fast_mod set in deserialized ciphertext
     */
    void deserialize(const Wire::Ciphertext *c, Ciphertext &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod);

    /**
     * Serialize PackedCiphertext
     */
    flatbuffers::Offset<Wire::PackedCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const PackedCiphertext &p);

    /**
     * Deserialize PackedCiphertext from raw buffer
     */
    void deserialize(const void* buf, PackedCiphertext &out);

    /**
     * Deserialize PackedCiphertext
     */
    void deserialize(const Wire::PackedCiphertext *p, PackedCiphertext &out);

    /**
     * Serialize Vec<float>
     */
    flatbuffers::Offset<Wire::VecFloat> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<float> &v);

    /**
     * Deserialize Vec<float> from raw buffer
     */
    void deserialize(const void* buf, Vec<float> &out);

    /**
     * Deserialize Vec<float>
     */
    void deserialize(const Wire::VecFloat *v, Vec<float> &out);

    /**
     * Serialize Vec<Integer>
     */
    flatbuffers::Offset<Wire::VecInteger> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<Integer> &v);

    /**
     * Deserialize Vec<Integer> from raw buffer
     */
    void deserialize(const void* buf, Vec<Integer> &out);

    /**
     * Deserialize Vec<Integer>
     */
    void deserialize(const Wire::VecInteger *v, Vec<Integer> &out);

    /**
     * Serialize Vec<Ciphertext>
     */
    flatbuffers::Offset<Wire::VecCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<Ciphertext> &v);

    /**
     * Deserialize Vec<Ciphertext> from raw buffer
     */
    void deserialize(const void* buf, Vec<Ciphertext> &out);

    /**
     * Deserialize Vec<Ciphertext>
     */
    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out);

    /**
     * Deserialize Vec<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     */
    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared);

    /**
     * Deserialize Vec<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     * @param fast_mod set in all deserialized ciphertexts
     */
    void deserialize(const Wire::VecCiphertext *v, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod);

    /**
     * Deserialize Vec<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     */
    void deserialize(const void* buf, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared);

    /**
     * Deserialize Vec<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     * @param fast_mod set in all deserialized ciphertexts
     */
    void deserialize(const void* buf, Vec<Ciphertext> &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod);

    /**
     * Serialize Vec<PackedCiphertext>
     */
    flatbuffers::Offset<Wire::VecPackedCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Vec<PackedCiphertext> &v);

    /**
     * Deserialize Vec<PackedCiphertext> from raw buffer
     */
    void deserialize(const void* buf, Vec<PackedCiphertext> &out);

    /**
     * Deserialize Vec<PackedCiphertext>
     */
    void deserialize(const Wire::VecPackedCiphertext *v, Vec<PackedCiphertext> &out);

    /**
     * Serialize Mat<float>
     */
    flatbuffers::Offset<Wire::MatFloat> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<float> &m);

    /**
     * Deserialize Mat<float> from raw buffer
     */
    void deserialize(const void* buf, Mat<float> &out);

    /**
     * Deserialize Mat<float>
     */
    void deserialize(const Wire::MatFloat *m, Mat<float> &out);

    /**
     * Serialize Mat<Integer>
     */
    flatbuffers::Offset<Wire::MatInteger> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<Integer> &m);

    /**
     * Deserialize Mat<Integer> from raw buffer
     */
    void deserialize(const void* buf, Mat<Integer> &out);

    /**
     * Deserialize Mat<Integer>
     */
    void deserialize(const Wire::MatInteger *m, Mat<Integer> &out);

    /**
     * Serialize Mat<Ciphertext>
     */
    flatbuffers::Offset<Wire::MatCiphertext> serialize(flatbuffers::FlatBufferBuilder &builder, const Mat<Ciphertext> &mat);

    /**
     * Deserialize Mat<Ciphertext> from raw buffer
     */
    void deserialize(const void* buf, Mat<Ciphertext> &out);

    /**
     * Deserialize Mat<Ciphertext>
     */
    void deserialize(const Wire::MatCiphertext *mat, Mat<Ciphertext> &out);

    /**
     * Deserialize Mat<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     */
    void deserialize(const Wire::MatCiphertext *mat, Mat<Ciphertext> &out, std::shared_ptr<Integer> n2_shared);

    /**
     * Deserialize Mat<Ciphertext>
     * @param n2_shared set in all deserialized ciphertexts
     * @param fast_mod set in all deserialized ciphertexts
     */
    void deserialize(const Wire::MatCiphertext *mat, Mat<Ciphertext> &out, std::shared_ptr<Integer> n2_shared, std::shared_ptr<FastMod> fast_mod);

    /**
     * Serialize PublicKey
     */
    flatbuffers::Offset<Wire::PublicKey> serialize(flatbuffers::FlatBufferBuilder &builder, const PublicKey &p);

    /**
     * Deserialize PublicKey from raw buffer
     */
    void deserialize(const void* buf, PublicKey &out);

    /**
     * Deserialize PublicKey
     */
    void deserialize(const Wire::PublicKey *p, PublicKey &out);

    /**
     * Serialize PrivateKey
     */
    flatbuffers::Offset<Wire::PrivateKey> serialize(flatbuffers::FlatBufferBuilder &builder, const PrivateKey &p);

    /**
     * Deserialize PrivateKey from raw buffer
     */
    void deserialize(const void* buf, PrivateKey &out);

    /**
     * Deserialize PrivateKey
     */
    void deserialize(const Wire::PrivateKey *p, PrivateKey &out);

    /**
     * Serialize KeyPair
     */
    flatbuffers::Offset<Wire::KeyPair> serialize(flatbuffers::FlatBufferBuilder &builder, const KeyPair &p);

    /**
     * Deserialize KeyPair from raw buffer
     */
    void deserialize(const void* buf, KeyPair &out);

    /**
     * Deserialize KeyPair
     */
    void deserialize(const Wire::KeyPair *k, KeyPair &out);
}
