#include <utils/CryptoHelper.hpp>
#include <Geode/Geode.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base64_url.hpp>
#include <cppcodec/base64_url_unpadded.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

std::vector<uint8_t> crypto::base64Decode(std::string const& data, Base64Flags flags) {
    switch (flags) {
        case Base64Flags::None:
            return cppcodec::base64_rfc4648::decode(data);
        case Base64Flags::UrlSafe:
            return cppcodec::base64_url::decode(data);
        case Base64Flags::NoPaddingUrlSafe:
            return cppcodec::base64_url_unpadded::decode(data);
    }
}
std::string crypto::base64Encode(std::vector<uint8_t> const& data, Base64Flags flags) {
    switch (flags) {
        case Base64Flags::None:
            return cppcodec::base64_rfc4648::encode(data);
        case Base64Flags::UrlSafe:
            return cppcodec::base64_url::encode(data);
        case Base64Flags::NoPaddingUrlSafe:
            return cppcodec::base64_url_unpadded::encode(data);
    }
}

// std::vector<uint8_t> crypto::blake2bEncrypt(std::vector<uint8_t> const& data) {
//     auto const dataLen = data.size();
//     auto const outLen = 32;
//     auto out = std::vector<uint8_t>(outLen);
//     blake2b(out.data(), outLen, data.data(), dataLen, nullptr, 0);
//     return out;
// }

// std::vector<uint8_t> crypto::blake2bDecrypt(std::vector<uint8_t> const& data) {
//     auto const dataLen = data.size();
//     auto const outLen = 64;
//     auto out = std::vector<uint8_t>(outLen);
//     blake2b(out.data(), outLen, data.data(), dataLen, nullptr, 0);
//     return out;
// }

// std::pair<std::vector<uint8_t>, std::vector<uint8_t>> crypto::chacha20Poly1305Encrypt(std::vector<uint8_t> const& data, std::vector<uint8_t> const& key, std::vector<uint8_t> const& nonce) {
//     auto const dataLen = data.size();
//     auto const outLen = dataLen;
//     auto out = std::vector<uint8_t>(outLen);
//     auto tag = std::vector<uint8_t>(16);
//     auto aad = std::vector<uint8_t>();
//     ChaCha20_Poly1305::aead_encrypt(out.data(), tag.data(), data.data(), data.size(), aad.data(), aad.size(), key.data(), nonce.data());
//     return {out, tag};
// }

// std::pair<std::vector<uint8_t>, std::vector<uint8_t>> crypto::chacha20Poly1305Decrypt(std::vector<uint8_t> const& data, std::vector<uint8_t> const& key, std::vector<uint8_t> const& nonce) {
//     auto const dataLen = data.size();
//     auto const outLen = dataLen;
//     auto out = std::vector<uint8_t>(outLen);
//     auto tag = std::vector<uint8_t>(16);
//     auto aad = std::vector<uint8_t>();
//     ChaCha20_Poly1305::aead_decrypt(out.data(), tag.data(), data.data(), data.size(), aad.data(), aad.size(), key.data(), nonce.data());
//     return {out, tag};
// }