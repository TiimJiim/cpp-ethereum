#pragma once

#pragma warning(push)
#pragma warning(disable:4244)
#include <sha3.h>
#pragma warning(pop)
#include "Common.h"

namespace eth
{

/// Functions are not re-entrant. If you want to multi-thread, then use different classes for each thread.
class Dagger
{
public:
	Dagger();
	~Dagger();
	
	static u256 bound(u256 const& _difficulty);
	
	static bool verify(u256 const& _root, u256 const& _nonce, u256 const& _difficulty);

	u256 eval(u256 const& _root, u256 const& _nonce);
	void evalAll1(u256 const& _root, u256 const& _xn);
	void evalAll2();
	void evalAll3();

	bool mine(u256& o_solution, u256 const& _root, u256 const& _difficulty, uint _msTimeout = 100);

private:
	h256 const& node(uint32_t _nonce);
	h256 const& nodeFast(uint32_t _nonce);

	u256 m_nonce;
	u256 m_root;
	u256 m_xn;

	h256* m_nodes;
	uint32_t* m_nodeSet;

	CryptoPP::SHA3_256 m_rootXnSHA;

public:
	uint_fast32_t m_numNodes;
};

}


