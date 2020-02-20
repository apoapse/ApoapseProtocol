// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include "TypeDefs.hpp"
#include <array>
#include <vector>
#include <cryptopp/secblock.h>

static constexpr UInt32 sha256Length = 32;
static constexpr UInt32 sha384Length = 48;
static constexpr UInt32 sha512Length = 64;

// SHA-2
using hash_SHA256 = std::array<byte, sha256Length>;
using hash_SHA512 = std::array<byte, sha512Length>;

// SHA-3
using hash_SHA3_256 = std::array<byte, sha256Length>;
using hash_SHA3_384 = std::array<byte, sha384Length>;
using hash_SHA3_512 = std::array<byte, sha512Length>;

// General hash
using hashSecBytes = CryptoPP::SecBlock<byte>;

// Asymmetric keys
using PrivateKeyBytes = CryptoPP::SecBlock<byte>;
using PublicKeyBytes = std::vector<byte>;
using EncryptedPrivateKeyBytes = std::vector<byte>;

// AES-CBC
using IV = std::array<byte, 16>;