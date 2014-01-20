/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include <random>
#include <chrono>
#include <Common.h>
#include <secp256k1.h>
#include "Dagger.h"
#include "RLP.h"
#include "Trie.h"
#include "State.h"
using namespace std;
using namespace std::chrono;
using namespace eth;

// TODO: utilise the shared testdata.

int main()
{
	u256 root = 0;
	root = 0x87BF2B87 | (root << 32);
	root = 0x92D075DD | (root << 32);
	root = 0x7DB56BA2 | (root << 32);
	root = 0x0AFD74CC | (root << 32);
	root = 0x0C065DF8 | (root << 32);
	root = 0x04E22984 | (root << 32);
	root = 0xEF1F2DAC | (root << 32);
	root = 0xCCE2E07F | (root << 32);

	u256 xn = 0;
	xn = 0x8C00E04A | (xn << 32);
	xn = 0x3A469A8F | (xn << 32);
	xn = 0x3C2786B6 | (xn << 32);
	xn = 0x8F4EE7CC | (xn << 32);
	xn = 0x5E31AD10 | (xn << 32);
	xn = 0x7D10D4B8 | (xn << 32);
	xn = 0x25D88FCD | (xn << 32);
	xn = 0xC20D7CF1 | (xn << 32);
	xn = xn >> 23;
	xn = xn << 23;

	for (uint_fast32_t i = 0; i != 10; ++i)
	{
		Dagger dagger;
		auto s = steady_clock::now();

		uint_fast32_t c_numn = 1;
		for (uint_fast32_t n = 0; n != c_numn; ++n)
		{
			u256 nonce = xn | ((1 << 23) - 1 - n);
			u256 val = dagger.eval(root, nonce);

			if (n == (c_numn-1))
			{
				// update xn
				xn = val;
				xn = xn >> 23;
				xn = xn << 23;
			}

			//cout << hex << val;
			//if ((n % 100) == 0)
			{
				size_t us = max<size_t>(1, duration_cast<microseconds>(steady_clock::now() - s).count());
				cout << dec << dagger.m_numNodes << " nodes, ";
				cout << dec << (us/1000) << " ms, ";
				cout << dec << (size_t)((dagger.m_numNodes*2000) / us) << " kh/s, ";
				//cout << hex << nonce << "->" << val;
				cout << endl;
			}
		}
	}
	cout << endl;

	{
		Dagger dagger;

		{
			auto s = steady_clock::now();
			dagger.evalAll1(root, xn);
			size_t us = max<size_t>(1, duration_cast<microseconds>(steady_clock::now() - s).count());
			cout << "n to 2^21 : ";
			cout << dec << dagger.m_numNodes << " nodes, ";
			cout << dec << (us/1000) << " ms, ";
			cout << dec << (size_t)((dagger.m_numNodes*2000) / us) << " kh/s";
			cout << endl;
		}
		{
			auto s = steady_clock::now();
			dagger.evalAll2();
			size_t us = max<size_t>(1, duration_cast<microseconds>(steady_clock::now() - s).count());
			cout << "n to 2^22 : ";
			cout << dec << dagger.m_numNodes << " nodes, ";
			cout << dec << (us/1000) << " ms, ";
			cout << dec << (size_t)((dagger.m_numNodes*2000) / us) << " kh/s";
			cout << endl;
		}
		{
			auto s = steady_clock::now();
			dagger.evalAll3();
			size_t us = max<size_t>(1, duration_cast<microseconds>(steady_clock::now() - s).count());
			cout << "n to 2^23 : ";
			cout << dec << dagger.m_numNodes << " nodes, ";
			cout << dec << (us/1000) << " ms, ";
			cout << dec << (size_t)((dagger.m_numNodes*2000) / us) << " kh/s";
			cout << endl;
		}
	}

	return 0;
}

