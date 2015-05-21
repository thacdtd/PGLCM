//
// main.cpp
//  
// Made by Trong Dinh Thac DO
// Login   <dot@imag.fr>
// 
// Started on  Thu Apr 1 12:00:00 2010 Thac DO
//
#include <fstream>
std::ostream *output; 

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <math.h>

#include "GLCM.hpp"
#include "TransactionSequence.h"
#include "DBMain.h"
#include "BinaryMatrix.h"
#include "TimeTracker.h"

using namespace std;

typedef list<TransactionSequence> LISTTS;
typedef list<int> LISTINT;
typedef GLCM GLCM;

/**
 * Load an ascii file (SPADE sample)
 * File format :
 * 	- first line display items name (must be integers)
 * 	- then each line display items values in the right order
 **/
DBMain * LoadBase (char * fileName) throw ()
{
	ifstream f (fileName);
	string line = "";
	int NbAttr = 1;
	DBMain * mainDB;

	// Looking for the number of attribute
	getline (f, line);
	for (unsigned int i = 0; i < line.length (); i++)
		if (line[i] == ' ')
			NbAttr++;

	// Looking for the number of customers
	int NbCust = 0;
	string LINE = "";
	for (;getline (f, line); NbCust++);

	//mainDB->Initialize (NbCust, NbAttr);
	f.close ();

	// We read again the file, treating its
	ifstream file (fileName);
	line = "";
	getline (file, line); // first line is useless

	vector <string> vecStr;
	string test = "";
	for (unsigned int i = 0; i < line.length (); i++)
	{
		if (line[i] == ' ')
		{
			vecStr.push_back (test);
			test.clear ();
		}
		else
			test +=line[i];
	}
	vecStr.push_back (test);
	int * att = new int [vecStr.size ()];
	for (unsigned int i = 0; i < vecStr.size (); i++)
	{
		att[i] = atoi (vecStr[i].c_str ());
	}

	// Now, loading transaction values
	int nbLine = 0;
	int ** values = new int * [vecStr.size ()];
	for (unsigned int i = 0; i < vecStr.size (); i++)
		values[i] = new int [NbCust];

	LINE = "";
	for (;getline (file, LINE); nbLine++)
	{
		vecStr.clear ();
		string test = "";
		for (unsigned int i = 0; i < LINE.length (); i++)
		{
			if ((LINE[i] == ' ') || (LINE[i] == '\t'))
			{
				vecStr.push_back (test);
				test.clear ();
			}
			else test +=LINE[i];
		}
		vecStr.push_back (test);

		for (unsigned int i = 0; i < vecStr.size (); i++)
		{
			values[i][nbLine] = atof(vecStr[i].c_str()) * 1000;
		}
		LINE = "";
	}
	mainDB = new DBMain(values,NbCust,NbAttr);
	file.close ();

	cout << "File loaded" << endl;

	for (unsigned int i = 0; i < vecStr.size (); i++)
		delete [] values[i];
	delete [] values;
	delete [] att;
	values = NULL;

	return mainDB;
}

int main(int argc, char** argv){

std::ofstream outputFile; 

  /* Deals with optional argument (ie. output file) */ 
  switch (argc){
  case (4):
    output = &std::cout; 
    cerr<<"No output file, using standard output."<<endl; 
    break;       
  case (5): 
    outputFile.open(argv[4]); 
    if(outputFile.fail()){
      std::cerr<<"Error opening file : "<<argv[4]<<"."<<std::endl;
      exit(EXIT_FAILURE); 
    }
    output = &outputFile; 
    break; 
  default :
    cout << "Usage = " << argv[0] << "<nor/gra/norwTSs> <input file name> <absolute support threshold> <output file>" << endl ;
		cout << "nor : Normal frequent definition" << endl;
		cout << "gra : GRAAK frequent definition" << endl;
		cout << "norwTSs : Normal frequent definition with computing transaction sequences" << endl;
    exit(EXIT_FAILURE);     
  }

		cout<<"\n     -----Debut du programm------       " << endl;

		// Recover arguments
		string freType = argv[1] ;
		char* inputFileName = argv[2] ;
		float threshold = atof(argv[3]) ;
		string so = "n";//argv[4] ;


		DBMain * dbmain = LoadBase(inputFileName);

        TimeTracker tt;
        tt.Start();
        
		GLCM glcm = GLCM();
		glcm.GLCMAlgo(dbmain,threshold,freType,so,*output);
        
        double ts = tt.Stop();
		cout<<"------------FIN----------"<<endl;
		glcm.PrintNoClosed();
        cout << "Total: " << setw(5) << ts << " sec" << endl;

        ostringstream ost;
        ost << getpid ();
        string cmd="ps -p "+ost.str ()+" -o rss";
        cout << "Total Memory usage : " << endl;
        //system(cmd.c_str());
        delete dbmain;

	return 0;
}
