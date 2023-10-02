#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>

using namespace CryptoPP;

std::string generateRandomKey(size_t keySize) {
    AutoSeededRandomPool rng;
    const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string key;

    for (size_t i = 0; i < keySize; ++i) {
        key += charset[rng.GenerateWord32(0, charset.size() - 1)];
    }

    return key;
}

std::string aes256CBCEncrypt(const std::string& plainText, const std::string& key, const std::string& iv) {
    std::string cipherText;
    CBC_Mode<AES>::Encryption encryptor;
    SecByteBlock keyBytes(reinterpret_cast<const byte*>(key.data()), key.size());
    SecByteBlock ivBytes(reinterpret_cast<const byte*>(iv.data()), iv.size());
    encryptor.SetKeyWithIV(keyBytes, keyBytes.size(), ivBytes, ivBytes.size());

    StringSource(plainText, true,
        new StreamTransformationFilter(encryptor,
            new StringSink(cipherText)
        )
    );

    return cipherText;
}

std::string aes256CBCDecrypt(const std::string& cipherText, const std::string& key, const std::string& iv) {
    std::string decryptedText;
    CBC_Mode<AES>::Decryption decryptor;
    SecByteBlock keyBytes(reinterpret_cast<const byte*>(key.data()), key.size());
    SecByteBlock ivBytes(reinterpret_cast<const byte*>(iv.data()), iv.size());
    decryptor.SetKeyWithIV(keyBytes, keyBytes.size(), ivBytes, ivBytes.size());

    StringSource(cipherText, true,
        new StreamTransformationFilter(decryptor,
            new StringSink(decryptedText)
        )
    );

    return decryptedText;
}