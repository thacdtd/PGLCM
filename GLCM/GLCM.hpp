#if !defined __GLCM_H__
#define      __GLCM_H__

#include <string>
#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <ctime>

#include "TransactionSequence.h"
#include "ItemSet.h"
#include "ItemSet.h"
#include "DBMain.h"
#include "BinaryMatrix.h"

using namespace std;
using namespace nsPattern;

class GLCM
{

public:
	GLCM(){};
	~GLCM();

	vector < vector< pair<int,int> > > buildG_1item(DBMain * DB);

	//Construct BM for All DB
	vector<BinaryMatrix *> constructBinaryMatrixAll(vector< vector< pair<int,int> > > G1Item);

	bool CheckFrequent(BinaryMatrix * BM, int threshold, string freType, TransactionSequence & ts);

	//Cal F
	ItemSet calF(BinaryMatrix * BM, vector< vector< pair<int,int> > > G1Item, const vector<BinaryMatrix *> & vBM, string freType, int & resFre);

	//Cal G
	void calG(ItemSet is, const vector<BinaryMatrix *> & vBM, BinaryMatrix& res);
	BinaryMatrix * andMatrix(BinaryMatrix * bm1, BinaryMatrix * bm2);

	//GLCM Algo
	void setThreshold(int th);

	void findP_i_minus_1(ItemSet P, int i, ItemSet& res);
	int findCore_i(ItemSet P, vector<TransactionSequence> G1Item, vector<BinaryMatrix *> & vBM);

	void GLCMloop(ItemSet P, BinaryMatrix& res, vector< vector< pair<int,int> > > G1Item, const vector<BinaryMatrix *> & vBM, string freType, string so, std::ostream &output, int & resFre, bool printTSsFlag);
	void GLCMAlgo(DBMain * DB, float threshold, string freType, string so, std::ostream &output);

	//to write to file
	void write_to_file(string freType);
	list<TransactionSequence> buildTSList(BinaryMatrix BM);
	void buildTSListRecu(TransactionSequence& tTS, int transaction,BinaryMatrix BM, list<TransactionSequence>& tempLTS);
	bool checkContains(int t, vector<int> vInt);

	bool CompareTransaction(int transaction1, int transaction2, list<pair<int,int> > lPair);
	bool CheckListContains(TransactionSequence & P,list<TransactionSequence> & lTQ);

	void PrintaListTS(list<TransactionSequence> lTS);
	void write_TS_List_to_file(const char * filename);
	void Print_All_Closed_Pattern();
	void PrintaVectorTS(vector<TransactionSequence> vTS);
	void Print_An_Itemset(ItemSet IS);
	void PrintNoClosed();

	//Frequent
	void findMaxChain(int transaction, BinaryMatrix * BM, TransactionSequence & ts);
	void findMaxTranSeq(BinaryMatrix * BM, TransactionSequence & ts);

	//dump
	void dumpItemset(std::ostream &os, ItemSet itemset, int freq);
	void dumpTSs(std::ostream &os, list<TransactionSequence> TSl);

	//Test
	void reduceBM(BinaryMatrix & BM, int threshold, bool & change);
	void recursion_Chk_Freq(int trans, BinaryMatrix BM, vector<int> & freMap);
	void loop_Chk_Freq(BinaryMatrix BM, vector<int> & freMap);
	int frequentCount(vector<int> & freMap);



private:
	list<BinaryMatrix> BinaryMatrixList;
	list<ItemSet> ClosedItemSetList;
	list<TransactionSequence> MaxlengthTransactionSequenceList;
	int _threshold ;		/**< Minimum support threshold */
	int nbIS;
}; // GLCM

#endif /* __GLCM_H__ */
