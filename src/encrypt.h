#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <random>

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

    StringSource s(plainText, true,new StreamTransformationFilter(encryptor,new StringSink(cipherText)));

    return cipherText;
}

std::string aes256CBCDecrypt(const std::string& cipherText, const std::string& key, const std::string& iv) {
    std::string decryptedText;
    CBC_Mode<AES>::Decryption decryptor;
    SecByteBlock keyBytes(reinterpret_cast<const byte*>(key.data()), key.size());
    SecByteBlock ivBytes(reinterpret_cast<const byte*>(iv.data()), iv.size());
    decryptor.SetKeyWithIV(keyBytes, keyBytes.size(), ivBytes, ivBytes.size());

    StringSource s(cipherText, true,new StreamTransformationFilter(decryptor,new StringSink(decryptedText)));

    return decryptedText;
}
std::string hashString(const std::string& input) {
	// Initialize the EVP_MD_CTX
	EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
	const EVP_MD* md = EVP_sha256();

	// Initialize the digest
	EVP_DigestInit_ex(mdctx, md, NULL);

	// Update the context with the input data
	EVP_DigestUpdate(mdctx, input.c_str(), input.length());

	// Finalize the hash and store it in 'hash' (a 32-byte binary array)
	unsigned char hash[SHA256_DIGEST_LENGTH];
	unsigned int hashLen;

	EVP_DigestFinal_ex(mdctx, hash, &hashLen);

	// Clean up the context
	EVP_MD_CTX_free(mdctx);

	// Convert the binary hash to a hexadecimal string
	std::string hashedString;
	char hexBuffer[3]; // Two characters for each byte plus a null terminator

	for (unsigned int i = 0; i < hashLen; ++i) {
		snprintf(hexBuffer, sizeof(hexBuffer), "%02x", hash[i]);
		hashedString += hexBuffer;
	}

	return hashedString;
}

std::string generateSalt(int length) {
	// Define a character set from which to generate the salt
	const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// Initialize a random number generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

	// Generate the salt
	std::string salt;
	for (int i = 0; i < length; ++i) {
		salt += charset[distribution(gen)];
	}

	return salt;
}

bool checkmatch(std::string pass_not_hashed, std::string pass_hashed, std::string salt)
{
	if (hashString(pass_not_hashed + salt) == pass_hashed)
	{
		return true;
	}
	else { return false; }
}