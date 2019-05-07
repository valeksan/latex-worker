#include "qcrypter.h"

#include <QDebug>

void QCrypter::aes_enc(const QByteArray &input, QByteArray &output, CryptAesMode mode, int nKeyBits)
{
	//qDebug() << "CryptoPP::AES::MAX_KEYLENGTH = " << CryptoPP::AES::MAX_KEYLENGTH;
	std::string plain = input.toStdString();
	std::string ciphertext;
	// Hex decode symmetric key:
	CryptoPP::HexDecoder decoder;
	decoder.Put(reinterpret_cast<const byte *>(PRIVATE_KEY), static_cast<size_t>(nKeyBits/8*2));
	decoder.MessageEnd();
	CryptoPP::word64 size = decoder.MaxRetrievable();
	QByteArray decKey(static_cast<int>(size), '\x00');
	decoder.Get(reinterpret_cast<byte*>(decKey.data()), static_cast<size_t>(size));
	//qDebug() << "decodedKey: " << decKey.toHex(' ').toUpper();
	// Generate Cipher, Key, and CBC
	QByteArray keys(CryptoPP::AES::MAX_KEYLENGTH, '\x00');
	QByteArray ivs(CryptoPP::AES::BLOCKSIZE, '\x00');
	CryptoPP::StringSource( decKey.constData(), true,
							new CryptoPP::HashFilter(*(
														new CryptoPP::SHA256),
														new CryptoPP::ArraySink(reinterpret_cast<byte*>(keys.data()), CryptoPP::AES::MAX_KEYLENGTH))
							);
	switch (mode) {
	case CBC: {
			CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption Encryptor( reinterpret_cast<const byte*>(keys.constData()), static_cast<size_t>(keys.size()), reinterpret_cast<const byte*>(ivs.constData()) );
			CryptoPP::StringSource( plain, true, new CryptoPP::StreamTransformationFilter( Encryptor,
																						new CryptoPP::HexEncoder(new CryptoPP::StringSink( ciphertext ) ) ) );
			output = QByteArray::fromHex(QByteArray::fromStdString(ciphertext));
		}
		break;
	case CBC_CTS:
		{
			CryptoPP::CBC_CTS_Mode<CryptoPP::AES>::Encryption Encryptor( reinterpret_cast<const byte*>(keys.constData()), static_cast<size_t>(keys.size()), reinterpret_cast<const byte*>(ivs.constData()) );
			CryptoPP::StringSource( plain, true, new CryptoPP::StreamTransformationFilter( Encryptor,
																						new CryptoPP::HexEncoder(new CryptoPP::StringSink( ciphertext ) ) ) );
			output = QByteArray::fromHex(QByteArray::fromStdString(ciphertext));
		}
		break;
	case CTR:
		{
			CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption Encryptor( reinterpret_cast<const byte*>(keys.constData()), static_cast<size_t>(keys.size()), reinterpret_cast<const byte*>(ivs.constData()) );
			CryptoPP::StringSource( plain, true, new CryptoPP::StreamTransformationFilter( Encryptor,
																						new CryptoPP::HexEncoder(new CryptoPP::StringSink( ciphertext ) ) ) );
			output = QByteArray::fromHex(QByteArray::fromStdString(ciphertext));
		}
		break;
	case OFB:
		{
			CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption Encryptor( reinterpret_cast<const byte*>(keys.constData()), static_cast<size_t>(keys.size()), reinterpret_cast<const byte*>(ivs.constData()) );
			CryptoPP::StringSource( plain, true, new CryptoPP::StreamTransformationFilter( Encryptor,
																						new CryptoPP::HexEncoder(new CryptoPP::StringSink( ciphertext ) ) ) );
			output = QByteArray::fromHex(QByteArray::fromStdString(ciphertext));
		}
		break;
	case ECB:
		{
			CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption Encryptor( reinterpret_cast<const byte*>(keys.constData()), static_cast<size_t>(keys.size()), reinterpret_cast<const byte*>(ivs.constData()) );
			CryptoPP::StringSource( plain, true, new CryptoPP::StreamTransformationFilter( Encryptor,
																						new CryptoPP::HexEncoder(new CryptoPP::StringSink( ciphertext ) ) ) );
			output = QByteArray::fromHex(QByteArray::fromStdString(ciphertext));
		}
		break;
	}
}

void QCrypter::aes_dec(const QByteArray &input, QByteArray &output, CryptAesMode mode, int nKeyBits)
{
	std::string plain;
	std::string encrypted = input.toHex().toStdString();
	// Hex decode symmetric key:
	CryptoPP::HexDecoder decoder;
	decoder.Put( reinterpret_cast<const unsigned char *>(QByteArray(PRIVATE_KEY).constData()), static_cast<size_t>(nKeyBits/8*2)/*32*2*/ );
	decoder.MessageEnd();
	CryptoPP::word64 size = decoder.MaxRetrievable();
	char *decodedKey = new char[size];
	memset(decodedKey, 0x00, static_cast<size_t>(size));
	decoder.Get(reinterpret_cast<unsigned char *>(decodedKey), static_cast<size_t>(size));
	// Generate Cipher, Key, and CBC
	byte key[ CryptoPP::AES::MAX_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( key, 0x00, CryptoPP::AES::MAX_KEYLENGTH );
	CryptoPP::StringSource( reinterpret_cast<const char *>(decodedKey), true,
							new CryptoPP::HashFilter(*(
													new CryptoPP::SHA256),
													new CryptoPP::ArraySink(key, CryptoPP::AES::MAX_KEYLENGTH))
							);
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );
	try {
		CryptoPP::CTR_Mode/*CFB_Mode*//*CBC_Mode*/<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
		CryptoPP::StringSource( encrypted,
								true,
								new CryptoPP::HexDecoder(
									new CryptoPP::StreamTransformationFilter( Decryptor,
																			new CryptoPP::StringSink( plain )
																			)
									)
								);

		switch (mode) {
		case CBC: {
				CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
				CryptoPP::StringSource( encrypted,
										true,
										new CryptoPP::HexDecoder(
											new CryptoPP::StreamTransformationFilter( Decryptor,
																					new CryptoPP::StringSink( plain )
																					)
											)
										);
				output = QByteArray::fromStdString(plain);
			}
			break;
		case CBC_CTS:
			{
				CryptoPP::CBC_CTS_Mode<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
				CryptoPP::StringSource( encrypted,
										true,
										new CryptoPP::HexDecoder(
											new CryptoPP::StreamTransformationFilter( Decryptor,
																					new CryptoPP::StringSink( plain )
																					)
											)
										);
				output = QByteArray::fromStdString(plain);
			}
			break;
		case CTR:
			{
				CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
				CryptoPP::StringSource( encrypted,
										true,
										new CryptoPP::HexDecoder(
											new CryptoPP::StreamTransformationFilter( Decryptor,
																					new CryptoPP::StringSink( plain )
																					)
											)
										);
				output = QByteArray::fromStdString(plain);
			}
			break;
		case OFB:
			{
				CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
				CryptoPP::StringSource( encrypted,
										true,
										new CryptoPP::HexDecoder(
											new CryptoPP::StreamTransformationFilter( Decryptor,
																					new CryptoPP::StringSink( plain )
																					)
											)
										);
				output = QByteArray::fromStdString(plain);
			}
			break;
		case ECB:
			{
				CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption Decryptor( key, sizeof(key), iv );
				CryptoPP::StringSource( encrypted,
										true,
										new CryptoPP::HexDecoder(
											new CryptoPP::StreamTransformationFilter( Decryptor,
																					new CryptoPP::StringSink( plain )
																					)
											)
										);
				output = QByteArray::fromStdString(plain);
			}
			break;
		}
	}
	catch (CryptoPP::Exception &e) {
		// ...
		qDebug() << e.what();
	}
//	catch (...) {
//		// ...
//	}
}

void QCrypter::sha1(const QByteArray &input, QByteArray &output)
{
	CryptoPP::SHA1 sha1;
	std::string hash = "";
	std::string source = input.toStdString();
	CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
	output = QByteArray::fromStdString(hash);
}

//void QCrypter::rsa_sign_enc(QString privateKeyPath, const QByteArray &input, QByteArray &output)
//{
//	std::string signature;
//	std::string message = input.toStdString();
//	CryptoPP::AutoSeededRandomPool rng;
//	CryptoPP::ByteQueue bytes;

//	//Read private key
//	CryptoPP::FileSource inputKey(privateKeyPath.toStdString().c_str(), true/*, new CryptoPP::Base64Decoder*/);
//	inputKey.TransferTo(bytes);
//	bytes.MessageEnd();
//	CryptoPP::RSA::PrivateKey rsaPrivate;
//	rsaPrivate.Load(bytes);

//	//Sign message
//	CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(rsaPrivate);
//	CryptoPP::SecByteBlock signatureRaw(signer.SignatureLength());
//	signer.SignMessage(rng,
//					   reinterpret_cast<byte const*>(message.data()),
//					   message.size(),
//					   signatureRaw);

//	// hex encode signature
//	CryptoPP::HexEncoder encoder;
//	encoder.Attach(new CryptoPP::StringSink(signature));
//	encoder.Put(signatureRaw.data(), signatureRaw.size());
//	encoder.MessageEnd();

//	//Save result signature
//	CryptoPP::word64 size = encoder.MaxRetrievable();
//	signature.resize(size);
//	encoder.Get(reinterpret_cast<byte*>(&signature[0]), signature.size());
////	CryptoPP::StringSource ss1(message, true,
////		new CryptoPP::SignerFilter(rng, signer,
////			new CryptoPP::StringSink(signature)
////	   ) // SignerFilter
////	); // StringSource
//	output = QByteArray::fromStdString(signature);
//}

//void QCrypter::rsa_read_private_key(QString privateKeyPath, QByteArray &output/*, bool base64_decode*/)
//{
//	CryptoPP::ByteQueue bytes;
//	std::string privateKey;

//	//Read private key
//	CryptoPP::FileSource inputKey(privateKeyPath.toStdString().c_str(), true, NULL/*, new CryptoPP::Base64Decoder()*/);
//	inputKey.TransferTo(bytes);
//	bytes.MessageEnd();
////	CryptoPP::RSA::PrivateKey rsaPrivate;
////	rsaPrivate.Load(bytes);
////	rsaPrivate.Save(CryptoPP::HexEncoder(
////						new CryptoPP::StringSink(privateKey)).Ref());
//	CryptoPP::word64 size = bytes.MaxRetrievable();
//	privateKey.resize(size, '\x00');
//	bytes.Get(reinterpret_cast<byte*>(&privateKey[0]), privateKey.size());
//	output = QByteArray::fromStdString(privateKey);
//}
