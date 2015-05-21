//
// TSCache.hpp
// 
// Made by Benjamin Negrevergne
// Login   <bengreve@confiance.imag.fr>
// 
// Started on  Fri Apr 17 11:59:24 2009 Benjamin Negrevergne
// Last update Wed Apr  7 12:48:08 2010 Alexandre Termier
//

#ifndef   	_TSCACHE_HPP_
#define   	_TSCACHE_HPP_

#include <iostream>

#include "Tuple.hpp"
#include "Internal.hpp"
#include "ThreadID.hpp"
#define AUTO_TERMINATE
const unsigned short int NUM_INTERNALS = NUM_INTERNALS_MACRO; 

extern const int SPACE_CLOSED; 
extern const int NO_MATCHES; 

extern const unsigned int NUM_THREADS; 
//#define TUPLESPACE_USE_SPINLOCKS

#define MACHINE_NEHALEM
//#define MACHINE_IDKONN

#ifdef MACHINE_IDKONN
#define NUM_L3_INTERNALS 4
#define NUM_L3_CORES 6
static const int mapL3toThreads[4][6] = {{0,12,4,16,8,20},{1,13,5,17,9,21},{2,14,6,18,10,22},{3,15,7,19,11,23}} ;
static const int threadIdxInL3 [24] = {0,0,0,0,2,2,2,2,4,4,4,4,1,1,1,1,3,3,3,3,5,5,5,5} ;
static const int mapThreadsToL3 [24] = {0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3} ; // standard bindings on idkonn
#endif
#ifdef MACHINE_NEHALEM
#define NUM_L3_INTERNALS 1
#define NUM_L3_CORES 4
static const int mapL3toThreads[1][4] = {{0,1,2,3}} ;
static const int threadIdxInL3 [4] = {0,1,2,3} ;
static const int mapThreadsToL3 [4] = {0,0,0,0} ; // only one L3 on the nehalem
#endif

/*
#if NUM_THREADS == 24
static const int mapThreadsToL3 [24] = {0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3} ; // standard bindings on idkonn
#endif

#if NUM_THREADS == 12
static const int mapThreadsToL3 [12] = {0,1,2,3,0,1,2,3,0,1,2,3} ; // standard bindings on idkonn
#endif

#if NUM_THREADS == 4
 #ifdef MACHINE_IDKONN
static const int mapThreadsToL3 [4] = {0,1,2,3} ; // standard binding on idkonn, use all physical processors
 #endif
 #ifdef MACHINE_NEHALEM
static const int mapThreadsToL3 [4] = {0,0,0,0} ; // only one L3 on the nehalem
 #endif
#endif
*/

typedef int TupleRef;

class TupleSpace{
public: 
  TupleSpace(); 
  ~TupleSpace();

 

  void put(void *tuple, unsigned int depth, unsigned int listID = THREAD::getMyID());
  void putTuple(const Tuple &t, unsigned int depth, unsigned int listID = THREAD::getMyID());


  int get(void *&tuple); 
  int getTuple(unsigned int typeID, const Tuple &tmpl, Tuple *&pTuple);  

  inline
  void tupleCountInc(); 

  inline
  void tupleCountDec(); 

  inline
  void tupleL3CountInc(unsigned int L3ID); 

  inline
  void tupleL3CountDec(unsigned int L3ID); 

  inline
  void workingL3Dec(); 

  inline void close();

private: 
  unsigned int _nbTuples;
  unsigned int _nbTuplesL3[NUM_L3_INTERNALS] ;
  unsigned int _nbWorkingL3[NUM_L3_INTERNALS] ;

  unsigned short int _workingInternal; 
  bool _closed;
  unsigned int _nbWorking; 
#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spinlock_t _spin; 
#else
  pthread_mutex_t _mutex; 
  pthread_cond_t _cond; 

  pthread_mutex_t _mutexL3[NUM_L3_INTERNALS]; 
  pthread_cond_t _condL3[NUM_L3_INTERNALS];   
#endif

  Internal _memInternal ; // This internal represents the memory, only gets initial tuples
  Internal _L3Internals[NUM_L3_INTERNALS] ; // Usefull ??? L3 might be only handled in get

  Internal _internals[NUM_INTERNALS];  
};

/*** INLINE DEFINITIONS ***/

void TupleSpace::tupleCountInc(){
//   if(__sync_fetch_and_add(&_nbTuples, 1) == 0){
//     pthread_cond_signal(&_cond); 
//   }

#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_lock(&_spin);
  _nbTuples++; 
  pthread_spin_unlock(&_spin);
#else
  pthread_mutex_lock(&_mutex); 
  if(_nbTuples++ == 0)
    pthread_cond_signal(&_cond); 
  pthread_mutex_unlock(&_mutex); 
#endif
}

void TupleSpace::tupleCountDec(){
  //  __sync_fetch_and_sub(&_nbTuples, 1); 
#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_lock(&_spin);
  _nbTuples--; 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_spin_unlock(&_spin);
#else
  pthread_mutex_lock(&_mutex); 
  _nbTuples--; 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_mutex_unlock(&_mutex); 
#endif
}

void TupleSpace::tupleL3CountInc(unsigned int L3idx){
//   if(__sync_fetch_and_add(&_nbTuples, 1) == 0){
//     pthread_cond_signal(&_cond); 
//   }

#ifdef TUPLESPACE_USE_SPINLOCKS // spin lock not updated for cache conscious...
  pthread_spin_lock(&_spin);
  _nbTuples++; 
  pthread_spin_unlock(&_spin);
#else
  pthread_mutex_lock(&_mutexL3[L3idx]); 
  if(_nbTuplesL3[L3idx]++ == 0)
    pthread_cond_signal(&_condL3[L3idx]); 
  pthread_mutex_unlock(&_mutexL3[L3idx]); 
#endif
}

void TupleSpace::tupleL3CountDec(unsigned int L3idx){
  //  __sync_fetch_and_sub(&_nbTuples, 1); 
#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_lock(&_spin);
  _nbTuples--; 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_spin_unlock(&_spin);
#else
  pthread_mutex_lock(&_mutexL3[L3idx]); 
  _nbTuplesL3[L3idx]--; 
  _nbWorkingL3[L3idx]++ ; // a tuple was taken : mark that a thread is working 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_mutex_unlock(&_mutexL3[L3idx]); 
#endif
}


void TupleSpace::workingL3Dec(){
  //  __sync_fetch_and_sub(&_nbTuples, 1); 
#ifdef TUPLESPACE_USE_SPINLOCKS
  pthread_spin_lock(&_spin);
  _nbTuples--; 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_spin_unlock(&_spin);
#else
  unsigned int internalIdx = THREAD::getMyID();
  unsigned int L3idx = mapThreadsToL3[internalIdx] ; // get the L3 associated with current thread
  //std::cout << "Work done reported by " << internalIdx << std::endl ;
  
  pthread_mutex_lock(&_mutexL3[L3idx]); 
  _nbWorkingL3[L3idx]-- ; 
  //  std::cout<<_nbTuples<<std::endl;
  pthread_mutex_unlock(&_mutexL3[L3idx]); 
#endif
}

void TupleSpace::close(){
  assert(!_closed); 
  _closed = true; 
#ifndef TUPLESPACE_USE_SPINLOCKS
  pthread_cond_broadcast(&_cond); 
#endif
}


#endif	    /* _TSCACHE_HPP_ */
