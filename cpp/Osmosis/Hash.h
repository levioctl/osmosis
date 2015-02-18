#ifndef __OSMOSIS_HASH_H__
#define __OSMOSIS_HASH_H__

#include <openssl/md5.h>
#include <openssl/sha.h>
#include "Osmosis/Tongue.h"
#include "Common/Error.h"
#include "Osmosis/Hex.h"

namespace Osmosis
{

class Hash
{
public:
	Hash( const struct Tongue::Hash & raw ) :
		_raw( raw )
	{
		ASSERT( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) or
				_raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 ) );
	}

	Hash( const char *hex, unsigned length )
	{
		Hex::toBuffer( hex, length, _raw.hash, sizeof( _raw.hash ) );
		if ( length == MD5_DIGEST_LENGTH * 2 )
			_raw.hashAlgorithm = static_cast< unsigned >( Tongue::HashAlgorithm::MD5 );
		else if ( length == SHA_DIGEST_LENGTH * 2 )
			_raw.hashAlgorithm = static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 );
		else
			THROW( Error, "Hash hex strings can be only 32 characters for MD5, or 40 for SHA1, but "
					"string " << std::string( hex, length ) << " is " << length );
	}

	Hash( const std::string & hex ) : Hash( hex.c_str(), hex.size() ) {}

	boost::filesystem::path relativeFilename() const
	{
		char name[ 64 ];
		snprintf( name, sizeof( name ), "%02x", _raw.hash[ 0 ] );
		boost::filesystem::path result( name );
		snprintf( name, sizeof( name ), "%02x", _raw.hash[ 1 ] );
		result /= name;
		Hex::fromBuffer( _raw.hash + 2, bytesCount() - 2, name, sizeof( name ) );
		result /= name;
		return result;
	}

	Tongue::HashAlgorithm algorithm() const
	{
		return static_cast< Tongue::HashAlgorithm >( _raw.hashAlgorithm );
	}

	const unsigned char * bytes() const
	{
		return _raw.hash;
	}

	unsigned bytesCount() const
	{
		ASSERT( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) or
				_raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 ) );
		return ( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) ) ? 16 : 20;
	}

	friend std::ostream & operator<<( std::ostream & os, const Hash & hash )
	{
		char buffer[ 64 ];
		Hex::fromBuffer( hash._raw.hash, hash.bytesCount(), buffer, sizeof( buffer ) );
		os << buffer;
		return os;
	}

	const Tongue::Hash & raw() const
	{
		return _raw;
	}

	bool operator == ( const Hash & other ) const
	{
		return _raw.hashAlgorithm == other._raw.hashAlgorithm and
			memcmp( _raw.hash, other._raw.hash, bytesCount() ) == 0;
	}

	bool operator != ( const Hash & other ) const
	{
		return not operator == ( other );
	}

	bool operator < ( const Hash & other ) const
	{
		return value() < other.value();
	}

private:
	struct Tongue::Hash _raw;

	unsigned long value() const
	{
		return * reinterpret_cast< const unsigned long * >( & _raw );
	}
};

} // namespace Osmosis

namespace std
{

template<> struct hash< Osmosis::Hash >
{
	std::size_t operator()( const Osmosis::Hash & hash ) const
	{
		return * reinterpret_cast< const std::size_t * >( hash.bytes() );
	}
};

} // namespace std

#endif // __OSMOSIS_HASH_H__
