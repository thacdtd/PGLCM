#if !defined __BINARYMATRIX_H__
#define      __BINARYMATRIX_H__

#include "CException.h"
#include "TransactionSequence.h"

#include <stdint.h>
#include <vector>
#include <list>
#include <string.h>

namespace nsPattern
{
    class BinaryMatrix
    {
		protected :
			uint32_t * m_BimaryMatrix;
			int m_size;

		public :
			BinaryMatrix (void) throw ();
			BinaryMatrix (const int size) throw ();
			BinaryMatrix (uint32_t * binaryMatrix, const int size) throw ();
			BinaryMatrix (BinaryMatrix * bm) throw ();
            BinaryMatrix (const BinaryMatrix& bm) throw ();
			virtual ~BinaryMatrix (void) throw ();


			/***************************************/
			/*				Methods				   */
			/***************************************/

            BinaryMatrix& operator=(const BinaryMatrix& bm);
        
			uint32_t *	getBimaryMatrix	(void) throw ();
			void	setBimaryMatrix	(uint32_t * binaryMatrix) throw ();
			int		getSize			(void) const throw ();
			void	setSize			(int size) throw ();

			bool getValue (const int column, const int row) throw ();
			void setValue (const int column, const int row, const bool & Val) throw ();
			void Initialize (const int size)  throw ();
			void PrintInfo (void) throw ();
			uint32_t * GetDimension (const int & size) throw ();

			//get count matrix where equal to 1
			int getActive() throw ();//Frequency count

			//Normal Frequency
			int getMaxTrans() throw ();
			int getMaxChild(int transaction) throw ();
			bool checkZero(int transaction) throw ();


			//set to 0
			void setTo0(int k) throw ();

			//Check one Matrix included
			bool isInclude(BinaryMatrix * bm) throw ();
        
            BinaryMatrix& operator&= (BinaryMatrix& bm) throw ();

			void constructBinaryMatrix(int item, vector< vector< pair<int,int> > > & G1Item);

			//to write to file
			list<pair<int,int> > findPair() throw();
			list < pair<int,int> > buildLeastOrderMap() throw ();

			vector <int> buildLeastOrder_0() throw ();
			vector <int> buildGreatOrder_0() throw ();
			vector <int> findAllChildren(int transaction) throw ();
			vector <int> findAllDirectChildren(int transaction) throw ();

			bool operator== (BinaryMatrix & bm) throw ();


			//two 2 equal values
			BinaryMatrix& setContrastTo0(const BinaryMatrix& bm);

    }; // BINARYMATRIX

    inline BinaryMatrix& BinaryMatrix::operator=(const BinaryMatrix& bm) {
        if (m_size < bm.m_size) {
            delete [] m_BimaryMatrix;
            m_BimaryMatrix = new uint32_t [((bm.m_size>>5)+1)*bm.m_size];
        }
        m_size = bm.m_size;
        memcpy(m_BimaryMatrix, bm.m_BimaryMatrix, ((bm.m_size>>5)+1)*bm.m_size*sizeof(uint32_t));
        return *this;
    }

    inline uint32_t * BinaryMatrix::getBimaryMatrix (void) throw ()
    {
        return m_BimaryMatrix;
    }
    
    inline void BinaryMatrix::setBimaryMatrix (uint32_t * binaryMatrix) throw ()
    {
        m_BimaryMatrix = binaryMatrix;
    }
    
    inline bool BinaryMatrix::getValue (const int column, const int row) throw ()
    {
        return (m_BimaryMatrix[row*((m_size>>5)+1)+(column>>5)] & (1 << column%32));
    }
    
    inline void BinaryMatrix::setValue (const int column, const int row, const bool & Val) throw ()
    {
        if (Val)
            m_BimaryMatrix[row*((m_size>>5)+1)+(column>>5)] |= (1 << column%32);
        else
            m_BimaryMatrix[row*((m_size>>5)+1)+(column>>5)] &= ~(1 << column%32);
    }
    
    inline int BinaryMatrix::getSize (void) const throw ()
    {
        return m_size;
    }
    
    inline void BinaryMatrix::setSize (int size) throw ()
    {
        m_size = size;
    }
    
    inline uint32_t * BinaryMatrix::GetDimension (const int & size) throw ()
    {
        return m_BimaryMatrix + size*m_size;
    }
    
    inline BinaryMatrix& BinaryMatrix::operator&= (BinaryMatrix& bm) throw ()
    {
	int BMSize = ((bm.m_size>>5)+1)*bm.m_size;
        for (int i = 0; i < BMSize; i++)
            m_BimaryMatrix[i] &= bm.m_BimaryMatrix[i];
        return *this;
    }


   //2 equal values
   //if there are loop in BM, set 1 to 0
       inline BinaryMatrix& BinaryMatrix::setContrastTo0(const BinaryMatrix& bm) {
        if (m_size < bm.m_size) {
            delete [] m_BimaryMatrix;
            m_BimaryMatrix = new uint32_t [((bm.m_size>>5)+1)*bm.m_size];
        }
        m_size = bm.m_size;
        memcpy(m_BimaryMatrix, bm.m_BimaryMatrix, ((bm.m_size>>5)+1)*bm.m_size*sizeof(uint32_t));
	for (int i = 0; i < m_size; i++)
	{
		for (int j = 0; j < m_size; j++)
			if ((this->getValue(i,j) == 1) && (this->getValue(j,i) == 1)) this->setValue(j,i,0); 
	}
        return *this;
    }

    std::ostream& operator<< (std::ostream & os, nsPattern::BinaryMatrix * db) throw ();
} //nsPattern

#endif /* __BINARYMATRIX__H__ */
