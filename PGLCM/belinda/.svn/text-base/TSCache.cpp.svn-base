// TSCache.cpp
// Made by Benjamin Negrevergne
// Started on  Fri Apr 17 12:03:47 2009

#include <iostream>
#include <cassert>

#include "TSCache.hpp"


pthread_t THREAD::_ids[100] = {0};
unsigned short int THREAD::_idx = 0;

#ifdef TRACE_EVENTS
eventData THREAD::_events[100]; 
const double THREAD::BEGIN = THREAD::getDate();
#endif

const int SPACE_CLOSED = -1; 
const int NO_MATCHES = -2; 

using namespace std; 

TupleSpace::TupleSpace(): 
  _nbTuples(0), 
  _workingInternal(0),
  _closed(false),
  _nbWorking(NUM_THREADS){

#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_init(&_spin, NULL); 
#else
  pthread_mutex_init(&_mutex, NULL); 
  pthread_cond_init(&_cond, NULL); 

  for (int i=0 ; i<NUM_L3_INTERNALS ; ++i) {
    _nbTuplesL3[i] = 0 ;
    _nbWorkingL3[i] = 0 ;
    pthread_mutex_init(&_mutexL3[i], NULL); 
    pthread_cond_init(&_condL3[i], NULL); 
  }


#endif
 
}

TupleSpace::~TupleSpace(){
#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_destroy(&_spin); 
#else
  pthread_mutex_destroy(&_mutex); 
  pthread_cond_destroy(&_cond); 

  for (int i=0 ; i<NUM_L3_INTERNALS ; ++i) {
    pthread_mutex_destroy(&_mutexL3[i]) ; 
    pthread_cond_destroy(&_condL3[i]) ; 
  }
#endif
}


void TupleSpace::put(void *tuple, unsigned int depth, unsigned int listID){
  //  assert(!_closed); 
  //  cout<<pthread_self()<<" puting in "<<listID<<endl;
  switch (depth) {
  case 0:
    _memInternal.put(tuple) ;
    tupleCountInc() ;
    //cout << "MEM tuple pushed by " << listID << " (" << _nbTuples << ")" << endl ;
    break ;
  case 1: 
    //    _L3Internals[mapThreadsTo3[listID]].put(tuple) ; // XXX 
    _internals[listID].put(tuple) ; 
    tupleL3CountInc(mapThreadsToL3[listID]) ;
    //cout << "L3 tuple pushed by " << listID << " (" << _nbTuplesL3[mapThreadsToL3[listID]] << ")" << endl ;
    // XXX handling L3 only in get ?
    break ;
  default:
    _internals[listID].put(tuple) ; // Never done in simple setting with depth limited to 1
    cerr << "?????" << endl ;
    break ;
  }

}

int TupleSpace::get(void *&tuple){
  //  cout<<"GET ON "<<internalIdx<<endl;
  /*  while(_nbTuples != 0) */
  THREAD::eventStart("wait"); 
  unsigned int internalIdx = THREAD::getMyID();
  unsigned int L3idx = mapThreadsToL3[internalIdx] ; // get the L3 associated with current thread
  unsigned int internalIdxInL3 = threadIdxInL3[internalIdx] ; // get position of core in L3

  // Simple case : try to get a tuple from one of the internal of the same L3
  while(__sync_fetch_and_or(&_nbTuplesL3[L3idx], 0)        // atomic or : _nbTuples in current L3 != 0 ???
	|| __sync_fetch_and_or(&_nbWorkingL3[L3idx], 0)){  // atomic or : are there working threads in current L3 ?   
                                                           // XXX can these be combined safely ???  
    //cout << internalIdx << " - in loop, #tuples in L3 = " << _nbTuplesL3[L3idx] << " #working in L3 = " << _nbWorkingL3[L3idx] << endl ;
    assert(internalIdx < NUM_INTERNALS); 
    if(! _internals[internalIdx].empty()){
      //      cout << internalIdx << " - internal not empty" << endl ;
      if( (tuple = _internals[internalIdx].iGet()) != NULL ){ // si son internal est pas vide il prend dedans
	/* Found a matching tuple, returns it */
	//cout << internalIdx << " - got tuple" << endl ;
	tupleL3CountDec(L3idx);
	//	THREAD::eventEnd(); 

	//	cout << internalIdx << " got L3 tuple (" << _nbTuplesL3[internalIdx] << ")" << endl ;
	return NULL; 
      }
    }
    //    cout << internalIdx << " - lookup next internal bank" << endl ;
    internalIdx = mapL3toThreads[L3idx][++internalIdxInL3 % NUM_THREADS % NUM_L3_CORES] ; // change internal but staying under the same L3
  }
  
  // No available tuples in L3 : get a tuple from memory

  //cout << "No internal tuples for : " << internalIdx << endl ;

  if(! _memInternal.empty()){
    if( (tuple = _memInternal.iGet()) != NULL ){ 
      /* Found a matching tuple, returns it */
      tupleCountDec();
      pthread_mutex_lock(&_mutexL3[L3idx]); 
      _nbWorkingL3[L3idx]++ ; // a tuple was taken : mark that a thread is working 
      //  std::cout<<_nbTuples<<std::endl;
      pthread_mutex_unlock(&_mutexL3[L3idx]); 
      //	THREAD::eventEnd(); 

      //cout << internalIdx << " got MEM tuple (" << _nbTuples << ")" << endl ;
      return NULL; 
    }
  }

  //cout << "No memory tuples for " << internalIdx << endl ;

  /* Reach here only if tupleSpace appear to be empty */

#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_lock(&_spin); 
#else
  pthread_mutex_lock(&_mutex); 
#endif
  //  cout<<"locked mode"<<endl;
  while(_nbTuples == 0){
    if(_closed){
#ifdef TUPLESPACE_USE_SPINLOCKS
      pthread_spin_unlock(&_spin); 
#else
      pthread_mutex_unlock(&_mutex); 
#endif
      //      THREAD::eventEnd(); 
      return SPACE_CLOSED;     
    }
#ifdef AUTO_TERMINATE
    if(--_nbWorking == 0){
      for(int i = 0; i < NUM_INTERNALS; i++){ //TODO CONTENTION POSSIBLE SUR LE PREMIER TS, faux de toute facon le mutex global est pris.
	_closed = true; 
#ifdef TUPLESPACE_USE_SPINLOCKS
	pthread_spin_unlock(&_spin); 
#else
	pthread_cond_broadcast(&_cond); 
	pthread_mutex_unlock(&_mutex); 
#endif
	THREAD::eventEnd(); 
	return SPACE_CLOSED;
      }
    }
#endif
#ifndef TUPLESPACE_USE_SPINLOCKS
    pthread_cond_wait(&_cond, &_mutex); 
#endif
  }  
  ++_nbWorking;
  for(int i = 0; i < NUM_INTERNALS; i++){ //TODO CONTENTION POSSIBLE SUR LE PREMIER TS, faux de toute facon le mutex global est pris.
    if( (tuple = _internals[i].iGet()) != NULL){
      _nbTuples--;
#ifdef TUPLESPACE_USE_SPINLOCKS
      pthread_spin_unlock(&_spin); 
#else
      pthread_mutex_unlock(&_mutex); 
#endif
      THREAD::eventEnd(); 
      return NULL;
    }
  }
#ifdef TUPLESPACE_USE_SPINLOCKS
      pthread_spin_unlock(&_spin); 
#else
      pthread_mutex_unlock(&_mutex); 
#endif
      THREAD::eventEnd(); 
      return SPACE_CLOSED; 
}


void TupleSpace::putTuple(const Tuple &t, unsigned int depth, unsigned int typeID){
  put(new Tuple(t), depth, typeID); 
}

int TupleSpace::getTuple(unsigned int typeID, const Tuple &tmpl, Tuple *&pTuple){
  int r; 
  void *t;
  if( (r = get(t)) != 0)
    return r;
  pTuple = (Tuple*)t;
  return typeID; 

//   int dataIdx = tmpl._elements.size();
//   Internal *internal = &_internals[_workingInternal++];
//   internal->getAccess();
//   while(internal->waitTuple()){
// //     TupleSpaceInternal::iterator entry = _data.lower_bound(typeID); 
// //     TupleSpaceInternal::iterator uBound = _data.upper_bound(typeID); 
// //    while(entry  != uBound){

//     for(internal_t::iterator entry = internal->_tuples.begin();
// 	entry != internal->_tuples.end(); ++entry){

//       //assert(entry->first == typeID);
//       Tuple *t = static_cast<Tuple*>(*entry);
//       if(tmpl.matches(*t)){
// 	internal->_tuples.erase(entry); /* Remove tuple from tuplespace */
// 	internal->releaseAccess(); 
// 	pTuple = t;
// #ifndef NDEBUG
// #ifdef CHECK_INTEGRITY
// 	t->checkMyIntegrity(); 
// #endif
// #endif 
// 	return typeID; 
//       }
//     }
//     internal->releaseAccess();
//     return NO_MATCHES; 
//   }
//   internal->releaseAccess();
//   return SPACE_CLOSED;
}


#ifdef TEST_TUPLE

static void thread(TupleSpace *ts){
  THREAD::registerThread();
  Tuple *t; 
  static const Tuple tmpl("???"); 
  while( (ts->getTuple(1u, tmpl, t)) >= 0){
    t->checkMyIntegrity();
//     cout<<pthread_self()<<" ";
//     cout<<t->getValue<int>(0)<<" ";
//     cout<<t->getValue<int>(1)<<" ";
//     cout<<t->getValue<int>(2)<<endl;
    if(t->getValue<int>(2) % 2 == 0)
      ts->putTuple(Tuple("iii", 1, 1, 1)); 
    
    delete t; 
  }
  cerr<<"return wrong value"<<endl;
 
}

//const unsigned int NUM_THREADS=2;
int main(){


  TupleSpace ts; 

  cout<<"TupleSpace created"<<endl;
  pthread_t tids[NUM_THREADS];
  

  for(int i = 0; i < 100000; i++){
    //    ts.put((void*)0xdeadbeef);
    ts.putTuple(Tuple("iii", 3, 14, i), i%NUM_INTERNALS); 
  }


  for(int i = 0; i < NUM_THREADS; i++){
    cout<<"creating thread 1"<<endl;
    pthread_create(&tids[i], NULL, (void*(*)(void*))thread, &ts); 
  }  
  
  //  ts.close();

  for(int i = 0; i < NUM_THREADS; i++)
    pthread_join(tids[i], NULL);
}
#endif
