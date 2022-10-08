/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */
 
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#elif !defined(_MD5_H)
#define _MD5_H

#pragma once
#include "../../slang.h"
#include "../core/slang-string.h"
#include "../core/slang-list.h"

namespace Slang
{
    /* Any 32-bit or wider unsigned integer data type will do */
    typedef uint32_t MD5_u32plus;

    struct MD5Context
    {
        MD5_u32plus lo, hi;
        MD5_u32plus a, b, c, d;
        unsigned char buffer[64];
        MD5_u32plus block[16];
    };

    class MD5HashGen
    {
    public:
        void init(MD5Context* ctx);

        // Helper update function for raw values (e.g. ints, uints)
        template<typename T,
            typename = std::enable_if<std::is_enum<T>::value || std::is_arithmetic<T>::value>::type>
        void update(MD5Context* ctx, const T& val)
        {
            update(ctx, &val, sizeof(T));
        }
        // Helper update function for Slang::List
        template<typename T>
        void update(MD5Context* ctx, const List<T>& list)
        {
            update(ctx, list.getBuffer(), list.getCount());
        }
        // Helper update function for UnownedStringSlice
        void update(MD5Context* ctx, UnownedStringSlice string);
        // Helper update function for Slang::String
        void update(MD5Context* ctx, String str);
        // Helper update function for Checksums
        void update(MD5Context* ctx, const slang::Checksum& checksum);

        void finalize(MD5Context* ctx, slang::Checksum* result);

    private:
        static const void* body(MD5Context* ctx, const void* data, SlangInt size);
        void update(MD5Context* ctx, const void* data, SlangInt size);
    };
}
 
#endif
