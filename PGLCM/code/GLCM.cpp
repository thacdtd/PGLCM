#if !defined __GLCM_CPP__
#define      __GLCM_CPP__

#define PARALLEL_PROCESS


#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <sys/time.h>
#include <sstream>
#include <iomanip>

#include "TransactionSequence.h"
#include "ItemSet.h"
#include "ItemSet.h"
#include "GLCM.hpp"
#include "DBMain.h"
#include "BinaryMatrix.h"
#include "TimeTracker.h"

using namespace std;
using namespace nsPattern;




#ifdef PARALLEL_PROCESS
#include <TupleSpace.hpp>
#include <Tuple.hpp>
#include <ThreadID.hpp>
#include <ostream>
TupleSpace tspace; 
const unsigned int NUM_THREADS = NUM_THREADS_MACRO;
#endif //PARALLEL_PROCESS






typedef struct
{
	int ind;
	int val;
} SortedObject;

int SortFunction (const void * a, const void * b)
{
	SortedObject * sa = (SortedObject *) a;
	SortedObject * sb = (SortedObject *) b;
	return (sa->val - sb->val);
}

bool funcSortTI (TransactionSequence TS1, TransactionSequence TS2) {
	  	  return TS1.getTransactionAt(1) < TS2.getTransactionAt(1);
};

struct SortLOPair
{
    bool operator()(pair<int,int> pair1, pair<int,int> pair2)
	{
		return pair1.second < pair2.second;
	};
};

GLCM::~GLCM(){} ;

/////////////////////////////////////////////////////////////////////////////////
//////////    Compute before Execute Algorithm   ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

vector < vector< pair<int,int> > > GLCM::buildG_1item(DBMain * DB)
{
	time_t t1, t2;

	t1 = time((time_t *)NULL);


	
	int * All_Transaction;

	int nbAtt = DB->GetAttribute();
	int nbCus = DB->GetCustomer();
	SortedObject * ValCopy = (SortedObject *) malloc (nbCus * sizeof (SortedObject));
	vector< pair<int,int> > tempPS;
	vector< vector< pair<int,int> > > vTS;
	for (int i = 0 ; i < 2 * nbAtt; i++)
	{
		All_Transaction = DB->GetDimension(i/2);
		vector <SortedObject *> tvTS;
		for (int j = 0 ; j < nbCus; j++)
		{
			ValCopy[j].ind = j;
			ValCopy[j].val = All_Transaction[j];
		}

		if (i % 2 == 0)//item increase
		{
			qsort (ValCopy, nbCus, sizeof (SortedObject), SortFunction);
			tempPS.clear();
			for (int k = 0; k < nbCus; k++)
			{
				pair<int,int> temppair = make_pair (ValCopy[k].ind,ValCopy[k].val);
				tempPS.push_back(temppair);
			}
		}
		else
		{
			vector< pair<int,int> > reversePS;
			reversePS.clear();
			vector< pair<int,int> >::iterator it;
			for (int i = tempPS.size() - 1; i >= 0 ; i--){
				reversePS.push_back(tempPS[i]);
			}
			tempPS = reversePS;
		}
		vTS.push_back(tempPS);
	}
	t2 = time((time_t *)NULL);
	free(ValCopy);
	return vTS;
};

vector<BinaryMatrix *> GLCM::constructBinaryMatrixAll(vector< vector< pair<int,int> > > G1Item)
{
	time_t t1, t2;

	t1 = time((time_t *)NULL);
	
	vector<BinaryMatrix *> vBM;
	vBM.clear();
	int G1ItemSize = G1Item.size();
	for (int i = 0; i < G1ItemSize; i++)
	{
		BinaryMatrix BM = BinaryMatrix(G1Item[i].size());
		BM.constructBinaryMatrix(i,G1Item);
		vBM.push_back(new BinaryMatrix(BM));
	}
	t2 = time((time_t *)NULL);
	return vBM;
}

//Old Frequent
//Get max chain from 1 transaction.
void GLCM::findMaxChain(int transaction, BinaryMatrix * BM, TransactionSequence & ts)
{
	ts.addTransactionBack(transaction);
	if (BM->checkZero(transaction)) return;
	int maxChild = BM->getMaxChild(transaction);
	if (maxChild != -1) findMaxChain(maxChild, BM, ts);
}

//Find max Transaction Sequence of an Item Set
void GLCM::findMaxTranSeq(BinaryMatrix * BM, TransactionSequence & ts)
{
	ts.clear();
	if (BM->getSize() == 0) return;
	int maxTrans = 0;
	maxTrans = BM->getMaxTrans();
	findMaxChain(maxTrans,BM,ts);
}

//Check one Sequence Frequent or not
bool GLCM::CheckFrequent(BinaryMatrix * BM, int threshold, string freType, TransactionSequence & ts)
{
	if (BM->getSize() == 0) return false;
	if (freType == "nor")
	{
		BinaryMatrix * tBM = new BinaryMatrix(BM->getSize());
		tBM->setContrastTo0(BM);
		findMaxTranSeq(tBM, ts);
		delete tBM;
		return  ts.size() >= threshold;
	}
	else
	{
		return (BM->getActive()) >= threshold;
	}
}

///////////////////////////////////////////////////////////////////
/////////////////  CAL F   ////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ItemSet GLCM::calF(BinaryMatrix * BM, vector< vector< pair<int,int> > > G1Item, const vector<BinaryMatrix *> & vBM, string freType, int & resFre)
{

	ItemSet is = ItemSet();
	is.clear();
	int BMSize = BM->getSize();
	BinaryMatrix * calFBM = new BinaryMatrix(BMSize);
	calFBM->setContrastTo0(BM);
	if (freType == "gra")
		resFre = calFBM->getActive();
	else {
		vector<int> freMap;
		freMap.clear();
		for (int i = 0; i < BMSize; i++)
		{
			freMap.push_back(-1);
		}
		loop_Chk_Freq(calFBM,freMap);
		resFre = frequentCount(freMap);
	}
	delete calFBM;
	if (resFre < _threshold) return is;

	int G1ItemSize = G1Item.size();
	for (int i = 0; i < G1ItemSize; i++)
	{
		BinaryMatrix tBM = (*vBM[i]);
		tBM &= (*BM);
		if (tBM == (*BM)) is.addItem(i);
	}
	return is;
}

///////////////////////////////////////////////////////////////////
///////////////// CAL G  //////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void GLCM::calG(ItemSet is, const vector<BinaryMatrix *> & vBM, BinaryMatrix& res)
{
	int isSize = is.size();
	if (isSize == 0)
        return;

    res = *vBM[is.getItemAt(0)];

	for (int i = 1; i < isSize; i++)
        res &= *vBM[is.getItemAt(i)];
}

BinaryMatrix * GLCM::andMatrix(BinaryMatrix * bm1, BinaryMatrix * bm2)
{
	BinaryMatrix * tBM = new BinaryMatrix(bm1->getSize());
	for (int i = 0; i < bm1->getSize(); i++)
	{
		for (int j = 0; j < bm1->getSize(); j++)
		{
			if ((bm1->getValue(j,i) == 1) && (bm2->getValue(j,i) == 1)) tBM->setValue(j,i,1);
			else tBM->setValue(j,i,0);
		}
	}
	return tBM;
}
///////////////////////////////////////////////////////////////////
//////////////  threshold   ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void GLCM::setThreshold(int th)
{
	_threshold = th;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// dump
//////////////////////////////////////////////////////////////////////////////////////////////

void GLCM::dumpItemset(std::ostream &os, ItemSet itemset, int freq){

	//os << "[ ";
	for (int i = 0 ; i < itemset.size() ; i++)
	{
		if ((itemset.getItemAt(i) % 2) == 0) os << itemset.getItemAt(i)/2 +1 << "+ ";
		else os << itemset.getItemAt(i)/2 +1 << "- ";
	}
	//os << "] ";
	os<<" ("<<freq<<")\n";
}

void GLCM::dumpTSs(std::ostream &os, list<TransactionSequence> TSl)
{
	os << "\t{ ";
	list<TransactionSequence>::iterator it;
	for (it = TSl.begin (); it != TSl.end (); it++)
	{
		os << " " << *it << " ";
	}
	os << "} \n"; 
}

//////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////

void GLCM::recursion_Chk_Freq(int trans, BinaryMatrix BM, vector<int> & freMap)
{
	int BMSize = BM.getSize();
	if (BM.checkZero(trans)) freMap[trans] = 1;
		for(int j = 0; j < BMSize; j++)
		{
			if (BM.getValue(j,trans))
			{
				if (freMap[j] == -1) recursion_Chk_Freq(j,BM,freMap);
				freMap[trans] = (freMap[trans]>freMap[j] + 1)?freMap[trans]:(freMap[j] + 1);
			}
		}
}

void GLCM::loop_Chk_Freq(BinaryMatrix BM, vector<int> & freMap)
{
	int freMapSize = freMap.size();
	for(int i = 0; i < freMapSize; i++)
	{
		recursion_Chk_Freq(i,BM,freMap);
	}
}

int GLCM::frequentCount(vector<int> & freMap)
{
	int maxFreq = 0;
	int freMapSize = freMap.size();
	for(int i = 0; i < freMapSize; i++)
	{
		if (freMap[i] > maxFreq) maxFreq = freMap[i]; 
	}
	return maxFreq;
}




/////////////////////////////////////////////////////////////////////////////
//////////  Parallel Process  ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifdef PARALLEL_PROCESS
void * processTupleThread(int id){
  THREAD::registerThread();

    /* Open a file for the current thread output */
  char outputName[] ="out_a"; 
  outputName[4]+=id; 
#ifdef MEMORY_OUTPUT
  std::ostringstream output;
#else
  std::ofstream output;
  output.open(outputName); 
#endif

  /* Retreives a Tuple */
  static const Tuple templateTuple("?????????"); 
  for(;;){
    Tuple *t; 
    int r = tspace.getTuple(1, templateTuple, t); 
    if(r != 1){
      if(r == NO_MATCHES)
	continue;
      if(r == SPACE_CLOSED)
	break; 
    }

    ItemSet * P = (t->getValue<ItemSet *>(0));
    BinaryMatrix * res = (t->getValue<BinaryMatrix *>(1));
    vector< vector< pair<int,int> > > * G1Item = (t->getValue<vector< vector< pair<int,int> > > *>(2));    
    vector<BinaryMatrix *> * vBM = (t->getValue<vector<BinaryMatrix *> *>(3));
    string * freType = (t->getValue<string *>(4));
	string * so = (t->getValue<string *>(5));
	int resFre = t->getValue<int>(6);
	bool * printTSsFlag = (t->getValue<bool *>(7));
	int depth = t->getValue<int>(8);

    GLCM glc = GLCM();
    glc.GLCMloop(*P, *res, *G1Item, *vBM, *freType, *so, output, resFre, *printTSsFlag, depth); 

delete P;
delete res;
delete G1Item;
delete vBM;
delete freType;
delete so;
delete printTSsFlag;

    delete t; 
}
}
#endif // parallel process




////////////////////////////////////////////////////////////////////////////////////
//////////   GLCM
////////////////////////////////////////////////////////////////////////////////////
void GLCM::GLCMloop(ItemSet P, BinaryMatrix& res, vector< vector< pair<int,int> > > G1Item, const vector<BinaryMatrix *> & vBM, string freType, string so , std::ostream &output, int & resFre, bool printTSsFlag, int depth)
{	
	if ((P.size() != 0) && (resFre < _threshold))
	{
		return;
	}
	if (P.size() != 0)
	{
		if (resFre != G1Item[0].size()){
			dumpItemset(output,P,resFre);
			nbIS++;
			if (printTSsFlag){
				list<TransactionSequence> TSl;
				TSl.clear();
				TSl = buildTSList(res);
				dumpTSs(output,TSl);
			}
		}
	}


	int k = 0;
	int core_i = -1;
	if (P.size() != 0) core_i = P.getCore_i();

	if ((core_i % 2 != 0) || core_i == -1) k = core_i;
	else k = core_i + 1;
	
    	BinaryMatrix calGBM(vBM[0]->getSize());
	int G1ItemSize = G1Item.size();


#ifndef PARALLEL_PROCESS
	for (int i = k + 1; i < G1ItemSize; i++)
	{
		ItemSet P1 = P.addItemwCore_i(i);
		if (P != P1)
		{
			calGBM = (*vBM[i]);
			if (res.getSize() != 0)
				calGBM &= res;
			int resourceFre = 0;
			ItemSet Q = calF(&calGBM,G1Item,vBM,freType,resourceFre);
			if (Q.size() != 0)
			{
				Q.setCore_i(P1.getCore_i());
				if (Q.prefixEqual(P1))
				{
					GLCMloop(Q, calGBM, G1Item, vBM, freType, so, output, resourceFre, printTSsFlag, depth + 1);
				}
			}
		}
	}
#else
if (depth <= 4){
	for (int i = k + 1; i < G1ItemSize; i++)
	{
		ItemSet P1 = P.addItemwCore_i(i);
		if (P != P1)
		{
			calGBM = (*vBM[i]);
			if (res.getSize() != 0)
				calGBM &= res;
			int resourceFre = 0;
			ItemSet Q = calF(&calGBM,G1Item,vBM,freType,resourceFre);
			if (Q.size() != 0)
			{
				Q.setCore_i(P1.getCore_i());
				if (Q.prefixEqual(P1))
				{
					ItemSet * pQ = new ItemSet(Q);
					BinaryMatrix * pcalGBM = new BinaryMatrix(calGBM);
					vector< vector< pair<int,int> > > * pG1Item = new vector< vector< pair<int,int> > >(G1Item);
					vector<BinaryMatrix *> * pvBM = new vector<BinaryMatrix *>(vBM);
					string * pfreType = new string(freType);
					string * pso = new string(so);
					bool * pprintTSsFlag =  new bool(printTSsFlag);


					Tuple t("ppppppipi", (char*) pQ, (char*) pcalGBM, (char*) pG1Item, (char*) pvBM, (char*) pfreType, (char*) pso, resourceFre, (char*) pprintTSsFlag, depth + 1); 
    					tspace.putTuple(t, 0);
				}
			}
		}
	}
}
else{
	for (int i = k + 1; i < G1ItemSize; i++)
	{
		ItemSet P1 = P.addItemwCore_i(i);
		if (P != P1)
		{
			calGBM = (*vBM[i]);
			if (res.getSize() != 0)
				calGBM &= res;
			int resourceFre = 0;
			ItemSet Q = calF(&calGBM,G1Item,vBM,freType,resourceFre);
			if (Q.size() != 0)
			{
				Q.setCore_i(P1.getCore_i());
				if (Q.prefixEqual(P1))
				{
					GLCMloop(Q, calGBM, G1Item, vBM, freType, so, output, resourceFre, printTSsFlag, depth + 1);
				}
			}
		}
	}
}
#endif
	
}

void GLCM::GLCMAlgo( DBMain * DB, float threshold, string freType,string so, std::ostream &output)
{
    TimeTracker tt1;
    tt1.Start();
	int depth = 1;

	nbIS = 0;
	ItemSet P = ItemSet();
	P.clear();
	ClosedItemSetList.clear();
	BinaryMatrixList.clear();
	vector< vector< pair<int,int> > > G1Item = buildG_1item(DB);
	cout << "There are " << G1Item[0].size() << " customers, and " <<  G1Item.size()/2 << " attributes." << endl << endl;
	if ((freType == "nor") || (freType == "norwTSs"))
	{
		_threshold = (int) ceil(threshold * G1Item[0].size());
	}
	else if (freType == "gra")
	{
		_threshold = (int) ceil(((threshold * G1Item[0].size() * (G1Item[0].size() - 1)))/2);
	}
	vector<BinaryMatrix *> vBM = constructBinaryMatrixAll(G1Item);
	BinaryMatrix BM = BinaryMatrix();
	cout << "Begin GLCM" << endl;
	int G1ItemSized2 = G1Item.size()/2;



#ifdef PARALLEL_PROCESS
	
	for (int i = 0; i < G1ItemSized2; i++)
	{
		cout << "Item Sets start with: " << i + 1 << "+" << endl;
		ItemSet P1 = P.addItemwCore_i(i*2);
		BM = (*vBM[i*2]);

		int resFre = 0;
		int BMSize = BM.getSize();
		BinaryMatrix * calFBM = new BinaryMatrix(BMSize);
		calFBM->setContrastTo0(BM);
		if (freType == "gra")
			resFre = calFBM->getActive();
		else {
			vector<int> freMap;
			freMap.clear();
			for (int i = 0; i < BMSize; i++)
			{
				freMap.push_back(-1);
			}
			loop_Chk_Freq(calFBM,freMap);
			resFre = frequentCount(freMap);
		}
		delete calFBM;
		bool printTSsFlag = false;
		if (freType == "norwTSs") printTSsFlag = true;

		Tuple t("ppppppipi", (char*) new ItemSet(P1), (char*) new BinaryMatrix(BM), (char*) new vector< vector< pair<int,int> > >(G1Item), (char*) new vector<BinaryMatrix *>(vBM), (char*) new string(freType), (char*) new string(so), resFre, (char*) new bool(printTSsFlag), depth); 
		tspace.putTuple(t, 0);
	}
	

/* Run the threads */
  pthread_t *tids = new pthread_t[NUM_THREADS];
cout << NUM_THREADS << endl;
  for(int i = 0; i < NUM_THREADS; i++){
    if(pthread_create(&tids[i], NULL, (void*(*)(void*))processTupleThread, (void *)i)){
      perror("pthread_create ");
      exit(EXIT_FAILURE); 
    }
}

  for(int i = 0; i < NUM_THREADS; i++)
    pthread_join(tids[i], NULL);
	tspace.close();
#else 
	for (int i = 0; i < G1ItemSized2; i++)
	{
		cout << "Item Sets start with: " << i + 1 << "+" << endl;
		ItemSet P1 = P.addItemwCore_i(i*2);
		BM = (*vBM[i*2]);

		int resFre = 0;
		int BMSize = BM.getSize();
		BinaryMatrix * calFBM = new BinaryMatrix(BMSize);
		calFBM->setContrastTo0(BM);
		if (freType == "gra")
			resFre = calFBM->getActive();
		else {
			vector<int> freMap;
			freMap.clear();
			for (int i = 0; i < BMSize; i++)
			{
				freMap.push_back(-1);
			}
			loop_Chk_Freq(calFBM,freMap);
			resFre = frequentCount(freMap);
		}
		delete calFBM;
		bool printTSsFlag = false;
		if (freType == "norwTSs") printTSsFlag = true;
		GLCMloop(P1, BM, G1Item, vBM, freType, so, output, resFre, printTSsFlag, depth);
	}
#endif //PARALLEL_PROCESS

	double ts1 = tt1.Stop();
	cout << endl << "Execution took " << setw(5) << ts1 << " sec" << endl;

	vector<BinaryMatrix*>::const_iterator posVBM ;
	for (posVBM = vBM.begin() ; posVBM != vBM.end() ; ++posVBM)
	  delete *posVBM ;
	  vBM.clear() ;

	ostringstream ost1;
	ost1 << getpid ();
	string cmd1="ps -p "+ost1.str ()+" -o rss";
	cout << "Execution Memory usage : " << endl;
	//system(cmd1.c_str());
	cout << endl;
}

void GLCM::write_to_file(string freType)
{
	TimeTracker tt2;
	tt2.Start();
	//print to file
	string st = "OUT/out";
	time_t tim=time(NULL);
	tm *now=localtime(&tim);
	stringstream ssm,ssd,ssh,ssmi,sss;//create a stringstream
	ssm << now->tm_mon + 1;//add number to the stream
	ssd << now->tm_mday;
	ssh << now->tm_hour;
	ssmi << now->tm_min;
	sss << now->tm_sec;
	st = st + ssm.str() + ssd.str() + ssh.str() + ssmi.str() + sss.str();

	char * filename = (char*) st.c_str();

	ofstream outf(filename);
	outf << ClosedItemSetList.size() <<" closed gradual frequent Item sets" << endl;

	list<ItemSet>::iterator it;
	for (it = ClosedItemSetList.begin (); it != ClosedItemSetList.end (); it++)
	{
		outf << "[ ";
		ItemSet itSet = *it;
		for (int i = 0 ; i < itSet.size() ; i++)
		{
			if ((itSet.getItemAt(i) % 2) == 0) outf << itSet.getItemAt(i)/2 +1 << "+ ";
			else outf << itSet.getItemAt(i)/2 +1 << "- ";
		}
		outf << "]" << endl;
	}

	outf.close();
	cout << "All Item sets have been written to file: " << st << endl;

	double ts2 = tt2.Stop();
	cout << endl << "Writing took " << setw(5) << ts2 << " sec" << endl;

	ostringstream ost1;
	ost1 << getpid ();
	string cmd1="ps -p "+ost1.str ()+" -o rss";
	cout << "Memory usage at writing step: " << endl;
	//system(cmd1.c_str());
	cout << endl;
}

//check 1 int contains in a vector or not
bool GLCM::checkContains(int t, vector<int> vInt)
{
	for (int i = 0; i < (signed)vInt.size(); i++)
	{
		if (vInt[i] == t) return true;
	}
	return false;
}

void GLCM::buildTSListRecu(TransactionSequence& tTS, int transaction,BinaryMatrix BM, list<TransactionSequence>& tempLTS)
{
	tTS.addTransactionBack(transaction);
	vector<int> vChild = BM.findAllDirectChildren(transaction);
	if (vChild.size() == 0)
	{
		tempLTS.push_back(tTS);
		tTS.clear();
		return;
	}
	else
	{
		for(int i = 0; i < (signed)vChild.size(); i++)
		{
			buildTSListRecu(tTS,vChild[i],BM,tempLTS);
		}
	}
}

list<TransactionSequence> GLCM::buildTSList(BinaryMatrix BM)
{
	list<pair<int,int> > lPair = BM.findPair();
	list<pair<int,int> > lLOM = BM.buildLeastOrderMap();
	lLOM.sort(SortLOPair());
	list<TransactionSequence> tlTS;
	tlTS.clear();

	list<pair<int,int> >::iterator itlLOM = lLOM.begin();
	do {
		pair<int,int> pLOM = *itlLOM;
		if (pLOM.second == 0)
		{
			TransactionSequence tempTS = TransactionSequence(pLOM.first);
			tlTS.push_back(tempTS);
			//do next node
			itlLOM++;
		}
		else
		{
			//Add Transaction in the same group (have the same LeastOrder) to list
			//use another iterator
			pLOM = *itlLOM;
			int tempLO = pLOM.second;
			list<TransactionSequence> tempLTS;
			tempLTS.clear();
			do
			{
				//use a temp list of TransactionSequence to store

				//Pop all list elements
				list<TransactionSequence>::iterator tempit;
				for (tempit = tlTS.begin(); tempit != tlTS.end(); tempit++)
				{
					TransactionSequence itempTS = *tempit;
					bool iC = CompareTransaction(itempTS.getTransactionAt(itempTS.size() - 1),pLOM.first,lPair);
					// if last transaction greater than new Transaction (that need to be added) -> add new Transaction to Sequence and push it back to list
					if (iC)
						//add new Transaction to Transaction Sequence and push to a temp list
						itempTS.addTransactionBack(pLOM.first);

					if (!CheckListContains(itempTS,tempLTS))
					{
						tempLTS.push_back(itempTS);
					}
				}
				itlLOM++;
				pLOM = *itlLOM;
			}
			while (pLOM.second == tempLO);
			tlTS = tempLTS;
		}
	}while(itlLOM != lLOM.end());
	return tlTS;
}

bool GLCM::CompareTransaction(int transaction1, int transaction2, list<pair<int,int> > lPair)
{
	if (lPair.size() != 0)
	{
		list<pair<int,int> >::iterator it;
		for (it = lPair.begin (); it != lPair.end (); it++)
		{
			pair<int,int> pair = *it;
			if ((pair.first == transaction1) && (pair.second== transaction2)) return true;
		}
	}
	return false;
}

bool GLCM::CheckListContains(TransactionSequence & P,list<TransactionSequence> & lTQ)
{
	list<TransactionSequence>::iterator itt;
	for (itt = lTQ.begin (); itt != lTQ.end (); itt++)
	{
		TransactionSequence ts = *itt;
		if(ts.contains(P)) return true;
	}
	return false;
}

void GLCM::write_TS_List_to_file(const char * filename)
{
	ofstream outf(filename);
	outf << MaxlengthTransactionSequenceList.size() <<" longest path" << endl;

	list<TransactionSequence>::iterator it;
	for (it = MaxlengthTransactionSequenceList.begin (); it != MaxlengthTransactionSequenceList.end (); it++)
	{
		outf << *it << " --> size: " << it->size() << endl;
	}
	outf.close();
}

void GLCM::PrintaVectorTS(vector<TransactionSequence> vTS)
{
	if (vTS.size()>0)
	{
		//list<TransactionSequence>::iterator it;
		for (int i = 0; i < (signed)vTS.size(); i++)
		{
			cout  << vTS[i] << endl;
		}
	}
}

void GLCM::PrintaListTS(list<TransactionSequence> lTS)
{
	list<TransactionSequence>::iterator it;
	for (it = lTS.begin (); it != lTS.end (); it++)
	{
		cout << *it << endl;
	}
}

void GLCM::Print_All_Closed_Pattern()
{
	list<ItemSet>::iterator it;
	cout << "Print all Closed Item set" << endl;
	for (it = ClosedItemSetList.begin (); it != ClosedItemSetList.end (); it++)
	{
		ItemSet itSet = *it;
		for (int i = 0 ; i < itSet.size() ; i++)
		{
			if ((itSet.getItemAt(i) % 2) == 0) cout << itSet.getItemAt(i)/2 +1 << "+ ";
			else cout << itSet.getItemAt(i)/2 +1 << "- ";
		}
		cout << endl;
	}
}

void GLCM::Print_An_Itemset(ItemSet IS)
{
	list<ItemSet>::iterator it;
	cout << " [";
	for (int i = 0; i < IS.size(); i++)
	{
		if ((IS.getItemAt(i) % 2) == 0) cout << IS.getItemAt(i)/2 +1 << "+";
					else cout << IS.getItemAt(i)/2 +1 << "-";
	}
	cout << "]" << endl;
}

void GLCM::PrintNoClosed()
{
	cout << "No of Closed Gradual Frequent Item Set: " <<  nbIS << endl;
}


#endif /* __GLCM_CPP__ */
