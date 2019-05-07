#ifndef QCRYPTER_H
#define QCRYPTER_H

#include <QByteArray>
#include <QString>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

//#if defined(CRYPTOPP_CXX11_NULLPTR) && !defined(NULLPTR)
//# define NULLPTR nullptr
//#elif !defined(NULLPTR)
//# define NULLPTR NULL
//#endif // CRYPTOPP_CXX11_NULLPTR

#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/filters.h>
#include <crypto++/hex.h>
#include <crypto++/sha.h>
#include <crypto++/rsa.h>
#include <crypto++/cryptlib.h>

#include <crypto++/osrng.h>
#include <crypto++/files.h>
#include <crypto++/base64.h>

//#include <crypto++/pem.h>

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef PRIVATE_KEY
#define PRIVATE_KEY "729308A8E815F6A40000000A00000000729308A8E815F6A4000000C000000000"
#endif

class QCrypter
{
public:
	enum CryptAesMode{
		CBC = 0,
		CBC_CTS,
		CTR,
		OFB,
		ECB
	};

	static void aes_enc(const QByteArray &input, QByteArray& output, CryptAesMode mode = QCrypter::CBC, int nKeyBits = 256);
	static void aes_dec(const QByteArray &input, QByteArray& output, CryptAesMode mode = QCrypter::CBC, int nKeyBits = 256);
	static void sha1(const QByteArray &input, QByteArray& output);
//	static void rsa_sign_enc(QString privateKeyPath, const QByteArray &input, QByteArray& output);
//	static void rsa_read_private_key(QString privateKeyPath, QByteArray& output);

};

#endif // QCRYPTER_H
