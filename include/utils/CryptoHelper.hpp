#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <utility>

namespace tulip::editor {
    namespace crypto {
        enum class Base64Flags {
            None = 0,
            UrlSafe = 1,
            NoPaddingUrlSafe = 2,
        };

        std::vector<uint8_t> base64Decode(std::string const& data, Base64Flags flags = Base64Flags::None);
        std::string base64Encode(std::vector<uint8_t> const& data, Base64Flags flags = Base64Flags::None);
        
        std::vector<uint8_t> blake2bEncrypt(std::vector<uint8_t> const& data);
        std::vector<uint8_t> blake2bDecrypt(std::vector<uint8_t> const& data);

        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> chacha20Poly1305Encrypt(std::vector<uint8_t> const& data, std::vector<uint8_t> const& key, std::vector<uint8_t> const& nonce);
        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> chacha20Poly1305Decrypt(std::vector<uint8_t> const& data, std::vector<uint8_t> const& key, std::vector<uint8_t> const& nonce);
    };
}