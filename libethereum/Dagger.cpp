#include <chrono>
#pragma warning(push)
#pragma warning(disable:4244)
#include <sha3.h>
#pragma warning(pop)
#include "Common.h"
#include "Dagger.h"
using namespace std;
using namespace std::chrono;

/* Implement revised dagger algorithm as described by Vitalik Buterin
D(data,xn,0) = sha3(data)
D(data,xn,n) =
    with v = sha3(data + xn + n)
         L = 3 if n < 2^21 or n > 2^22 else 11
         a[k] = floor(v/n^k) mod n for 0 <= k < L
    sha3(data ++ xn ++ D(data,xn,a[0]) ++ D(data,xn,a[1]) ++ ... ++ D(data,xn,a[L-1])
Challenge: find xn and n where 2^22 < n < 2^23 such that D(data,xn,n) * diff <= 2^256
*/

namespace eth
{

Dagger::Dagger()
{
	m_nodes = new h256[1 << 23];
	m_nodeSet = new uint32_t[(1 << 23) / 32];
}

Dagger::~Dagger()
{
	delete [] m_nodes;
	delete [] m_nodeSet;
}

u256 Dagger::bound(u256 const& _difficulty)
{
	return (u256)((bigint(1) << 256) / _difficulty);
}

bool Dagger::verify(u256 const& _root, u256 const& _nonce, u256 const& _difficulty)
{
	return Dagger().eval(_root, _nonce) < bound(_difficulty);
}

bool Dagger::mine(u256& o_solution, u256 const& _root, u256 const& _difficulty, uint _msTimeout)
{
/* todo, test this
	// restart search if root has changed
	if (m_root != _root)
	{
		m_root = _root;
		m_nonce = 1 << 22;
	}

	// compute bound
	u256 const b = bound(_difficulty);

	// evaluate until we run out of time
	for (auto startTime = steady_clock::now(); (steady_clock::now() - startTime) < milliseconds(_msTimeout); )
	{
		
		if (eval(_root, m_nonce) < b)
		{
			o_solution = m_nonce;
			return true;
		}

		if (((uint32_t)++m_nonce & ((1 << 23) - 1)) < (1 << 22))
		{
			m_nonce += 1 << 22;
		}
	}
*/
	return false;
}

template <class _T>
inline void update(_T& _sha, h256 const& _value)
{
	_sha.Update(_value.data(), 32);
}

template <class _T>
inline void update(_T& _sha, u256 const& _value)
{
	const auto data = _value.backend().limbs();
	static_assert(sizeof(_value.backend().limbs()[0]) == 4, "Expecting 4 byte words in u256");

	h256 beval;
	for (uint_fast32_t i = 0; i != 8; ++i)
	{
		uint32_t word = data[7 - i];
		beval[i*4 + 0] = (byte)(word >> 24);
		beval[i*4 + 1] = (byte)(word >> 16);
		beval[i*4 + 2] = (byte)(word >> 8);
		beval[i*4 + 3] = (byte)(word);
	}

	update(_sha, beval);
}

template <class _T>
inline void get(h256& o_ret, _T& _sha)
{
	_sha.TruncatedFinal(&o_ret[0], 32);
}

template <class _T>
inline h256 get(_T& _sha)
{
	h256 ret;
	_sha.TruncatedFinal(&ret[0], 32);
	return ret;
}

h256 const& Dagger::node(uint32_t _n)
{
	uint_fast32_t bit = 1 << (_n % 32);
	uint_fast32_t word = _n / 32;

	if (!(m_nodeSet[word] & bit))
	{
		// compute v
		CryptoPP::SHA3_256 sha;
		update(sha, m_root + m_xn + _n);
		u256 v = get(sha);

		sha = m_rootXnSHA;
		
		uint_fast32_t const numk = _n >= (1 << 21) && _n < (1 << 22) ? 11u : 3u;
		for (uint_fast32_t k = 0; k != numk; ++k)
		{
			u256 v1 = v / _n;
			uint32_t pk = (uint32_t)(v - v1*_n);
			v = v1;
			update(sha, node(pk));
		}
		get(m_nodes[_n], sha);

		m_nodeSet[word] |= bit;
		m_numNodes += 1;
	}
	return m_nodes[_n];
}

h256 const& Dagger::nodeFast(uint32_t _n)
{
	// compute v
	CryptoPP::SHA3_256 sha;
	update(sha, m_root + m_xn + _n);
	u256 v = get(sha);

	sha = m_rootXnSHA;

	uint_fast32_t const numk = _n >= (1 << 21) && _n < (1 << 22) ? 11u : 3u;
	for (uint_fast32_t k = 0; k != numk; ++k)
	{
		u256 v1 = v / _n;
		uint32_t pk = (uint32_t)(v - v1*_n);
		v = v1;
		update(sha, m_nodes[pk]);
	}
	get(m_nodes[_n], sha);

	m_numNodes += 1;
	return m_nodes[_n];
}

u256 Dagger::eval(u256 const& _root, u256 const& _nonce)
{
	m_numNodes = 0;

	u256 xn = _nonce >> 23;
	if (m_root != _root || m_xn != xn)
	{
		m_xn = xn;
		m_root = _root;
		memset(m_nodeSet, 0, (1 << 23) / 8);

		m_rootXnSHA.Restart();
		update(m_rootXnSHA, m_root);
		update(m_rootXnSHA, m_xn);

		m_nodes[0] = _root;
		m_nodeSet[0] = 1;
	}

	return node((uint32_t)_nonce % (1 << 23));
}

void Dagger::evalAll1(u256 const& _root, u256 const& _xn)
{
	m_numNodes = 0;

	if (m_root != _root || m_xn != _xn)
	{
		m_xn = _xn;
		m_root = _root;
		m_nodes[0] = _root;

		m_rootXnSHA.Restart();
		update(m_rootXnSHA, m_root);
		update(m_rootXnSHA, m_xn);
	}

	for (uint_fast32_t n = 1; n != (1 << 21); ++n)
	{
		nodeFast(n);
	}
}

void Dagger::evalAll2()
{
	m_numNodes = 0;

	for (uint_fast32_t n = (1 << 21); n != (1 << 22); ++n)
	{
		nodeFast(n);
	}
}

void Dagger::evalAll3()
{
	m_numNodes = 0;

	for (uint_fast32_t n = (1 << 22); n != (1 << 23); ++n)
	{
		nodeFast(n);
	}
}

}
