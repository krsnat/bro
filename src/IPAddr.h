// See the file "COPYING" in the main distribution directory for copyright.

#ifndef IPADDR_H
#define IPADDR_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include "BroString.h"
#include "util.h"

typedef in_addr in4_addr;

/**
 * Class storing both IPv4 and IPv6 addresses.
 */
class IPAddr
{
public:
	/**
	 * Address family.
	 */
	enum Family { IPv4, IPv6 };

	/**
	 * Byte order.
	 */
	enum ByteOrder { Host, Network };

	/**
	 * Constructs the unspecified IPv6 address (all 128 bits zeroed).
	 */
	IPAddr()
		{
		memset(in6.s6_addr, 0, sizeof(in6.s6_addr));
		}

	/**
	 * Constructs an address instance from an IPv4 address.
	 *
	 * @param in6 The IPv6 address.
	 */
	IPAddr(const in4_addr& in4)
		{
		memcpy(in6.s6_addr, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&in6.s6_addr[12], &in4.s_addr, sizeof(in4.s_addr));
		}

	/**
	 * Constructs an address instance from an IPv6 address.
	 *
	 * @param in6 The IPv6 address.
	 */
	IPAddr(const in6_addr& arg_in6) : in6(arg_in6) { }

	/**
	 * Constructs an address instance from a string representation.
	 *
	 * @param s String containing an IP address as either a dotted IPv4
	 * address or a hex IPv6 address.
	 */
	IPAddr(const std::string& s)
		{
		Init(s);
		}

	/**
	 * Constructs an address instance from a string representation.
	 *
	 * @param s ASCIIZ string containing an IP address as either a
	 * dotted IPv4 address or a hex IPv6 address.
	 */
	IPAddr(const char* s)
		{
		Init(s);
		}

	/**
	 * Constructs an address instance from a string representation.
	 *
	 * @param s String containing an IP address as either a dotted IPv4
	 * address or a hex IPv6 address.
	 */
	IPAddr(const BroString& s)
		{
		Init(s.CheckString());
		}

	/**
	 * Constructs an address instance from a raw byte representation.
	 *
	 * @param family The address family.
	 *
	 * @param bytes A pointer to the raw byte representation. This must point
	 * to 4 bytes if \a family is IPv4, and to 16 bytes if \a family is
	 * IPv6.
	 *
	 * @param order Indicates whether the raw representation pointed to
	 * by \a bytes is stored in network or host order.
	 */
	IPAddr(Family family, const uint32_t* bytes, ByteOrder order);

	/**
	 * Copy constructor.
	 */
	IPAddr(const IPAddr& other) : in6(other.in6) { };

	/**
	 * Destructor.
	 */
	~IPAddr() { };

	/**
	 * Returns the address' family.
	 */
	Family family() const
		{
		if ( memcmp(in6.s6_addr, v4_mapped_prefix, 12) == 0 )
			return IPv4;
		else
			return IPv6;
		}

	/**
	 * Returns true if the address represents a loopback device.
	 */
	bool IsLoopback() const;

	/**
	 * Returns true if the address represents a multicast address.
	 */
	bool IsMulticast() const
		{
		if ( family() == IPv4 )
			return in6.s6_addr[12] == 224;
		else
			return in6.s6_addr[0] == 0xff;
		}

	/**
	 * Returns true if the address represents a broadcast address.
	 */
	bool IsBroadcast() const
		{
		if ( family() == IPv4 )
			return ((in6.s6_addr[12] == 0xff) && (in6.s6_addr[13] == 0xff)
				&& (in6.s6_addr[14] == 0xff) && (in6.s6_addr[15] == 0xff));
		else
			return false;
		}

	/**
	 * Retrieves the raw byte representation of the address.
	 *
	 * @param bytes The pointer to which \a bytes points will be set to
	 * the address of the raw representation in network-byte order.
	 * The return value indicates how many 32-bit words are valid starting at
	 * that address. The pointer will be valid as long as the address instance
	 * exists.
	 *
	 * @return The number of 32-bit words the raw representation uses. This
	 * will be 1 for an IPv4 address and 4 for an IPv6 address.
	 */
	int GetBytes(const uint32_t* const * bytes) const
		{
		if ( family() == IPv4 )
			{
			*bytes = (uint32_t*) &in6.s6_addr[12];
			return 1;
			}
		else
			{
			*bytes = (uint32_t*) in6.s6_addr;
			return 4;
			}
		}

	/**
	 * Retrieves a copy of the IPv6 raw byte representation of the address.
	 * If the internal address is IPv4, then the copied bytes use the
	 * IPv4 to IPv6 address mapping to return a full 16 bytes.
	 *
	 * @param bytes The pointer to a memory location in which the
	 * raw bytes of the address are to be copied in network byte-order.
	 */
	void CopyIPv6(uint32_t* bytes) const
		{
		memcpy(bytes, in6.s6_addr, sizeof(in6.s6_addr));
		}

	/**
	 * Masks out lower bits of the address.
	 *
	 * @param top_bits_to_keep The number of bits \a not to mask out,
	 * counting from the highest order bit. The value is always
	 * interpreted relative to the IPv6 bit width, even if the address
	 * is IPv4. That means if compute ``192.168.1.2/16``, you need to
	 * pass in 112 (i.e., 96 + 16). The value must be in the range from
	 * 0 to 128.
	 */
	void Mask(int top_bits_to_keep);

	/**
	 * Masks out top bits of the address.
	 *
	 * @param top_bits_to_chop The number of bits to mask out, counting
	 * from the highest order bit.  The value is always interpreted relative
	 * to the IPv6 bit width, even if the address is IPv4.  So to mask out
	 * the first 16 bits of an IPv4 address, pass in 112 (i.e., 96 + 16).
	 * The value must be in the range from 0 to 128.
	 */
	void ReverseMask(int top_bits_to_chop);

	/**
	 * Assignment operator.
	 */
	IPAddr& operator=(const IPAddr& other)
		{
		// No self-assignment check here because it's correct without it and
		// makes the common case faster.
		in6 = other.in6;
		return *this;
		}

	/**
	 * Returns a string representation of the address. IPv4 addresses
	 * will be returned in dotted representation, IPv6 addresses in
	 * compressed hex.
	 */
	string AsString() const;

	/**
	 * Returns a string representation of the address. This returns the
	 * same as AsString().
	 */
	operator std::string() const { return AsString(); }

	/**
	 * Comparison operator for IP address.
	 */
	friend bool operator==(const IPAddr& addr1, const IPAddr& addr2)
		{
		return memcmp(&addr1.in6, &addr2.in6, sizeof(in6_addr)) == 0;
		}

	friend bool operator!=(const IPAddr& addr1, const IPAddr& addr2)
		{
		return ! (addr1 == addr2);
		}

	/**
	 * Comparison operator IP addresses. This defines a well-defined order for
	 * IP addresses. However, the order does not necessarily correspond to
	 * their numerical values.
	 */
	friend bool operator<(const IPAddr& addr1, const IPAddr& addr2)
		{
		return memcmp(&addr1.in6, &addr2.in6, sizeof(in6_addr)) < 0;
		}

	unsigned int MemoryAllocation() const { return padded_sizeof(*this); }

private:
	/**
	 * Initializes an address instance from a string representation.
	 *
	 * @param s String containing an IP address as either a dotted IPv4
	 * address or a hex IPv6 address.
	 */
	void Init(const std::string& s);

	in6_addr in6; // IPv6 or v4-to-v6-mapped address

	static const uint8_t v4_mapped_prefix[12]; // top 96 bits of v4-mapped-addr
};

inline IPAddr::IPAddr(Family family, const uint32_t* bytes, ByteOrder order)
	{
	if ( family == IPv4 )
		{
		memcpy(in6.s6_addr, v4_mapped_prefix, sizeof(v4_mapped_prefix));
		memcpy(&in6.s6_addr[12], bytes, sizeof(uint32_t));

		if ( order == Host )
			{
			uint32_t* p = (uint32_t*) &in6.s6_addr[12];
			*p = htonl(*p);
			}
		}

	else
		{
		memcpy(in6.s6_addr, bytes, sizeof(in6.s6_addr));

		if ( order == Host )
			{
			for ( unsigned int i = 0; i < 4; ++ i)
				{
				uint32_t* p = (uint32_t*) &in6.s6_addr[i*4];
				*p = htonl(*p);
				}
			}
		}
	}

inline bool IPAddr::IsLoopback() const
	{
	if ( family() == IPv4 )
		return in6.s6_addr[12] == 127;

	else
		return ((in6.s6_addr[0] == 0) && (in6.s6_addr[1] == 0)
			&& (in6.s6_addr[2] == 0) && (in6.s6_addr[3] == 0)
			&& (in6.s6_addr[4] == 0) && (in6.s6_addr[5] == 0)
			&& (in6.s6_addr[6] == 0) && (in6.s6_addr[7] == 0)
			&& (in6.s6_addr[8] == 0) && (in6.s6_addr[9] == 0)
			&& (in6.s6_addr[10] == 0) && (in6.s6_addr[11] == 0)
			&& (in6.s6_addr[12] == 0) && (in6.s6_addr[13] == 0)
			&& (in6.s6_addr[14] == 0) && (in6.s6_addr[15] == 1));
	}

/**
 * Class storing both IPv4 and IPv6 prefixes
 * (i.e., \c 192.168.1.1/16 and \c FD00::/8.
 */
class IPPrefix
{
public:
	/**
	 * Constructs a prefix instance from an IPv4 address and a prefix
	 * length.
	 *
	 * @param in4 The IPv4 address.
	 *
	 * @param length The prefix length in the range from 0 to 32.
	 */
	IPPrefix(const in4_addr& in4, uint8_t length);

	/**
	 * Constructs a prefix instance from an IPv6 address and a prefix
	 * length.
	 *
	 * @param in6 The IPv6 address.
	 *
	 * @param length The prefix length in the range from 0 to 128.
	 */
	IPPrefix(const in6_addr& in6, uint8_t length);

	/**
	 * Constructs a prefix instance from an IPAddr object and prefix length.
	 *
	 * @param addr The IP address.
	 *
	 * @param length The prefix length in the range from 0 to 128
	 */
	IPPrefix(const IPAddr& addr, uint8_t length);

	/**
	 * Constructs a prefix instance from IP string representation and length.
	 *
	 * @param s String containing an IP address as either a dotted IPv4
	 * address or a hex IPv6 address.
	 *
	 * @param length The prefix length in the range from 0 to 128
	 */
	IPPrefix(const std::string& s, uint8_t length);

	/**
	 * Copy constructor.
	 */
	IPPrefix(const IPPrefix& other)
		: prefix(other.prefix), length(other.length) { }

	/**
	 * Destructor.
	 */
	~IPPrefix() { }

	/**
	 * Returns the prefix in the form of an IP address. The address will
	 * have all bits not part of the prefixed set to zero.
	 */
	const IPAddr& Prefix() const { return prefix; }

	/**
	 * Returns the bit length of the prefix, relative to the 32 bits
	 * of an IPv4 prefix or relative to the 128 bits of an IPv6 prefix.
	 */
	uint8_t Length() const
		{
		return prefix.family() == IPAddr::IPv4 ? length - 96 : length;
		}

	/**
	 * Returns the bit length of the prefix always relative to a full
	 * 128 bits of an IPv6 prefix (or IPv4 mapped to IPv6).
	 */
	uint8_t LengthIPv6() const { return length; }

	/**
	 * Assignment operator.
	 */
	IPPrefix& operator=(const IPPrefix& other)
		{
		// No self-assignment check here because it's correct without it and
		// makes the common case faster.
		prefix = other.Prefix();
		length = other.Length();
		return *this;
		}

	/**
	 * Returns a string representation of the prefix. IPv4 addresses
	 * will be returned in dotted representation, IPv6 addresses in
	 * compressed hex.
	 */
	string AsString() const;

	operator std::string() const	{ return AsString(); }

	unsigned int MemoryAllocation() const { return padded_sizeof(*this); }

	/**
	 * Comparison operator for IP prefix.
	 */
	friend bool operator==(const IPPrefix& net1, const IPPrefix& net2)
		{
		return net1.Prefix() == net2.Prefix() && net1.Length() == net2.Length();
		}

	/**
	 * Comparison operator IP prefixes. This defines a well-defined order for
	 * IP prefix. However, the order does not necessarily corresponding to their
	 * numerical values.
	 */
	friend bool operator<(const IPPrefix& net1, const IPPrefix& net2)
		{
		if ( net1.Prefix() < net2.Prefix() )
			return true;

		else if ( net1.Prefix() == net2.Prefix() )
			return net1.Length() < net2.Length();

		else
			return false;
		}

private:
	IPAddr prefix;	// We store it as an address with the non-prefix bits masked out via Mask().
	uint8_t length;	// The bit length of the prefix relative to full IPv6 addr.
};

#endif
