#include "BinaryMatrix.h"
#include "Sort.h"
#include "TransactionSequence.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <map>

using namespace std;
using namespace nsPattern;

#define BINARYMATRIX nsPattern::BinaryMatrix
#define NULLVAL -9999.0

BINARYMATRIX::BinaryMatrix (void) throw ()
{
	m_size = 0;
	m_BimaryMatrix = NULL;
}

BINARYMATRIX::BinaryMatrix (const int size) throw ()
{
	if (size == 0)
	{
		m_size = 0;
		m_BimaryMatrix = NULL;
	}
	else
	{
		m_size = size;
		m_BimaryMatrix = new uint32_t [((m_size>>5)+1)*m_size];
		bzero(m_BimaryMatrix, ((m_size>>5)+1)*m_size*sizeof(uint32_t));
	}
}

BINARYMATRIX::BinaryMatrix (uint32_t * binaryMatrix, const int size) throw ()
{
	m_size = size;
	m_BimaryMatrix = new uint32_t [((m_size>>5)+1)*m_size];
    memcpy(m_BimaryMatrix, binaryMatrix, ((m_size>>5)+1)*m_size*sizeof(uint32_t));
}

BINARYMATRIX::BinaryMatrix (BinaryMatrix * bm) throw ()
{
    if (bm == NULL) {
        m_size = 0;
        m_BimaryMatrix = NULL;
    } else {
        m_size = bm->getSize();
        m_BimaryMatrix = new uint32_t [((m_size>>5)+1)*m_size];
        memcpy(m_BimaryMatrix, bm->m_BimaryMatrix, ((m_size>>5)+1)*m_size*sizeof(uint32_t));
    }
}

BINARYMATRIX::BinaryMatrix (const BinaryMatrix& bm) throw ()
{
	m_size = bm.getSize();
	m_BimaryMatrix = new uint32_t [((m_size>>5)+1)*m_size];
    memcpy(m_BimaryMatrix, bm.m_BimaryMatrix, ((m_size>>5)+1)*m_size*sizeof(uint32_t));
}

BINARYMATRIX::~BinaryMatrix (void) throw ()
{
	delete [] m_BimaryMatrix;
	m_BimaryMatrix = NULL;
}

void BINARYMATRIX::Initialize (const int size) throw ()
{
	m_size = size;
    bzero(m_BimaryMatrix, ((m_size>>5)+1)*m_size*sizeof(uint32_t));
}

void BINARYMATRIX::PrintInfo (void) throw ()
{
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
			cout << setw (6) << this->getValue(j,i);
		cout << endl;
	}
}

bool BINARYMATRIX::operator== (BinaryMatrix & bm) throw ()
{
	for (int i = 0; i < ((m_size>>5)+1)*m_size; i++)
        if (m_BimaryMatrix[i] != bm.m_BimaryMatrix[i])
            return false;
	return true;
}

ostream& nsPattern::operator<< (ostream & os, BINARYMATRIX * db) throw ()
{
	for (int i = 0; i < db->getSize(); i++)
	{
		for (int j = 0; j < db->getSize(); j++)
			os << setw (6) << db->getValue(j, i);
		os << endl;
	}
	return os;
}

int BINARYMATRIX::getActive() throw ()
{
	int count = 0;
    for (int i = 0; i < ((m_size>>5)+1)*m_size; i++) {
    	register unsigned int v = m_BimaryMatrix[i];//register
        v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
        unsigned int c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
        count = count + c;
    }
	return count;
}

int BINARYMATRIX::getMaxTrans() throw ()
{
	int maxCount = 0;
	int maxTrans = -1;
	int mSizeDec = ((m_size>>5)+1);
	for (int i = 0; i < m_size * mSizeDec; i = i + mSizeDec)
	{
		int count = 0;
		for (int j = i; j < (mSizeDec + i); j++)
		{	
			register unsigned int v = m_BimaryMatrix[j];//register
        		v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
        		v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
        		unsigned int c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
			count += c;
		}
		if (count > maxCount)
		{
			maxCount = count;
			maxTrans = i/mSizeDec;
		}
	}
	return maxTrans;
}

//max Child is a child that have the most value 1
int BINARYMATRIX::getMaxChild(int transaction) throw ()
{
	int maxCount = 0;
	int maxChild = -1;
	int mSizeDec = ((m_size>>5)+1);
	for (int i = 0; i < m_size; i++)
	{
		if  (m_BimaryMatrix[transaction*mSizeDec+(i>>5)] & (1 << i%32))
		{
			int child = i;
			int count = 0;
			for (int j = child * mSizeDec; j < (mSizeDec + child * mSizeDec); j++)
			{	
				register unsigned int v = m_BimaryMatrix[j];//register
        			v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
        			v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
        			unsigned int c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
				count += c;
			}
			if (count >= maxCount)
			{
				maxCount = count;
				maxChild = child;
			}
		}
	}
	return maxChild;
}

bool BINARYMATRIX::checkZero(int transaction) throw ()
{
	int mSizeDec = ((m_size>>5)+1);
    for (int i = (transaction * mSizeDec); i < (((transaction +1) * mSizeDec)); i++) {
    	if (m_BimaryMatrix[i])
            return false;
    }
	return true;
}

void BINARYMATRIX::setTo0(int k) throw ()
{
	for (int i = 0 ; i < m_size; i++)
	{
        m_BimaryMatrix[i*((m_size>>5)+1)+(k>>5)] &= ~(1 << k%32);
        m_BimaryMatrix[k*((m_size>>5)+1)+(i>>5)] &= ~(1 << i%32);
	}
}

bool BINARYMATRIX::isInclude(BinaryMatrix * bm) throw ()
{
	int mSizeDec = ((m_size>>5)+1);

    for (int i = 0; i < m_size; i++)
	{
    	int iDec = (i>>5);
    	int iMod = (1 << i%32);
		for (int j = 0; j < m_size; j++)
            if ( (bm->m_BimaryMatrix[j*mSizeDec+iDec] & iMod) &&
                ( ( m_BimaryMatrix[i*mSizeDec+(j>>5)] ^ bm->m_BimaryMatrix[i*mSizeDec+(j>>5)] ) & (1 << j%32) ) )//!(m_BimaryMatrix[j*mSizeDec+iDec] & iMod))
                return false;
	}
	return true;
}

void BINARYMATRIX::constructBinaryMatrix(int item, vector< vector< pair<int,int> > > & G1Item)
{
	vector< pair<int,int> > ts = G1Item[item];
	int tsSize = ts.size();
	for (int i = 0; i < tsSize; i++)
	{
		for (int j = 0; j < tsSize; j++)
		{
			int row = ts[i].first;
			int col = ts[j].first;
			if ((ts[i].second != NULLVAL) && (ts[j].second != NULLVAL))
			{			
				this->setValue(col, row, (j > i));
				if ((ts[i].second == ts[j].second) && (i !=j))	
				{
					this->setValue(col, row, 1);
					this->setValue(row, col, 1);
				}		
			}
		}
	}
}

//to write to file
list<pair <int,int> > BINARYMATRIX::findPair() throw()
{
	list<pair <int,int> > lTSBM;
	lTSBM.clear();
	pair<int,int> pBM;
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
		{
			if(this->getValue(j,i))
			{
				pBM = make_pair(i,j);
				lTSBM.push_back(pBM);
			}
		}
	}
	return lTSBM;
}

list < pair<int,int> > BINARYMATRIX::buildLeastOrderMap() throw ()
{
	list < pair<int,int> > lMBM;
	lMBM.clear();
	pair<int,int> mBM;
	for (int i = 0; i < m_size; i++)
	{
		int count = 0;
		for (int j = 0; j < m_size; j++)
		{
			if(this->getValue(i,j))
			{
				count++;
			}
		}
		mBM = make_pair (i,count);
		lMBM.push_back(mBM);
	}
	return lMBM;
}

//Node has no father
vector <int> BINARYMATRIX::buildLeastOrder_0() throw ()
{
	vector <int> vMBM;
	vMBM.clear();
	for (int i = 0; i < m_size; i++)
	{
		int count = 0;
		for (int j = 0; j < m_size; j++)
		{
			if(this->getValue(i,j))
			{
				count++;
			}
		}
		if (count == 0)
		{
			cout << "LO" << i << endl;
			vMBM.push_back(i);
		}
	}
	return vMBM;
}

//Node has no child
vector <int> BINARYMATRIX::buildGreatOrder_0() throw ()
{
	vector <int> vMBM;
	vMBM.clear();
	for (int i = 0; i < m_size; i++)
	{
		if (m_BimaryMatrix[i] == 0)
			vMBM.push_back(i);
	}
	return vMBM;
}

//find all children of a transaction
vector <int> BINARYMATRIX::findAllChildren(int transaction) throw ()
{
	vector <int> tVInt;
	tVInt.clear();
	for (int i = 0; i< m_size; i++) {
		if (this->getValue(i,transaction))
			tVInt.push_back(i);
	}
	return tVInt;
}

//find all direct children of a transaction
vector <int> BINARYMATRIX::findAllDirectChildren(int transaction) throw ()
{
	vector <int> tVDInt;
	tVDInt.clear();
	vector <int> tVInt = findAllChildren(transaction);
	if (tVInt.size() == 0) return tVDInt;
	for (int i = 0; i < (signed)tVInt.size(); i++)
	{
		vector <int> tVCInt = findAllChildren(tVInt[i]);
		if ((tVInt.size() == (tVCInt.size() + 1)))
		{
			cout << " DC " << tVInt[i] << endl;
			tVDInt.push_back(tVInt[i]);
		}
	}
	return tVDInt;
}
#undef DBMAIN
