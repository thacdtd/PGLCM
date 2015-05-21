GLCM/PGLCM
	GLCM/PGLCM are the first directly algorithms for mining gradual closed frequent itemsets. They inherit the ppc-extension from LCM, which is the fastest algorithm for mining frequent patterns at this time. 

I. GLCM
	GLCM is the sequential version of the algorithm. It has the same strong advantages of LCM: linear time complexity and constant memory consumption. 
	1. Download
		<<<<< link download >>>>>>>

	2.  Compilation
		Inside the archive's root directory.
		$> make

	3. Run
		./glcm <nor/gra/norwTSs>  <input file name>  <absolute support threshold>  <output file>

		nor : Normal frequent definition
		gra : GRAAK frequent definition
		norwTSs : Normal frequent definition with computing transaction sequences

		<absolute support threshold> is in the format 0.abc... eg. 0.1 means the results have greater than or equal to 10% of total transactions.
	
	4. Input/Output format
		Input file
		Dataset must be a single ASCII file. The number of rows are the total attributes. Each line is a transaction. First line is the attributes counter (not a transaction). eg. 
		--- file temp.data ---
		1 2 3
		22 1200 4
		24 2300 2
		30 2700 3
		28 3000 1
		-----------------------
		is a valid dataset.
		Dataset has 3 attributes and 4 transactions. First line represents that dataset has 3 attributes. 

		Output file
		glcm output files have the following format : 
each line is a gradual frequent closed itemset.
frequency is stored at the end of the line into brackets. 
		eg. running glcm on temp.data dataset, with the following command:
			./glcm nor temp.data 0.1 out_temp
		will generate the following file 
		--- file out_temp.data ---
			1+ 2+  (3)
			1+ 2+ 3+  (2)
			1+ 2+ 3-  (3)
			1+ 2- 3+  (2)
			1+ 3+  (2)
			2+ 3-  (3)


II. PGLCM
	PGLCM is the parallel implementation. This parallel implementation can take advantages of the strength of recent multi-process computer. It scales well with the number of available cores for complex datasets, where such computing power is really needed. 
	1. Download
		<<<<< link download >>>>>>>

	2.  Compilation
		Inside the archive's root directory:
		$> make dep
		$> make mt
		This will produce the program for 1 .. 24 threads marked from pglcm1 to pglcm24. eg.
		pglcm4 stands for the PGLCM algorithm that run in 4 threads. 

	3. Run
		./pglcm<nthreads> <nor/gra/norwTSs>  <input file name>  <absolute support threshold>  <output file>

		replace <nthreads> by the number of threads
		nor : Normal frequent definition
		gra : GRAAK frequent definition
		norwTSs : Normal frequent definition with computing transaction sequences

		<absolute support threshold> is in the format 0.abc... eg. 0.1 means the results have greater than or equal to 10% of total transactions.
	
	4. Input/Output format
		Input file
		Dataset must be a single ASCII file. The number of rows are the total attributes. Each line is a transaction. First line is the attributes counter (not a transaction). eg. 
		--- file temp.data ---
		1 2 3
		22 1200 4
		24 2300 2
		30 2700 3
		28 3000 1
		-----------------------
		is a valid dataset.
		Dataset has 3 attributes and 4 transactions. First line represents that dataset has 3 attributes. 

		Output file
		pglcm output files have the following format : 
			Each line is a gradual frequent closed itemset.
			Frequency is stored at the end of the line into brackets. 
			The number of output files depend on the number of threads. Each file store the result that is produce by the thread it represents for. They will be named as out_”n” (“n” is a character from a to z depends on the number of threads)
		eg. running pglcm on temp.data dataset, with the following command:
			./pglcm2 nor temp.data 0.1
		will generate the 2 files: out_a and out_b 
			--- file out_a ---
				1+ 2+ 3+  (2)
			--- file out_b ---
				2+ 3-  (3)
				1+ 2+  (3)
				1+ 2+ 3-  (3)
				1+ 3+  (2)
				1+ 2- 3+  (2)

			notice that the results may vary from the execution.
			eg. Run ./pglcm2 nor temp.data 0.1 again we will have 2 files: out_a and out_b 
			--- file out_a ---
				2+ 3-  (3)
				1+ 2- 3+  (2)
			
			--- file out_b ---
				1+ 3+  (2)
				1+ 2+  (3)
				1+ 2+ 3-  (3)
				1+ 2+ 3+  (2)

			but the total final result will always the same.


Reference publications
	Trong Dinh Thac Do, Anne Laurent, Alexandre Termier: PGLCM: Efficient Parallel Mining of Closed Frequent Gradual Itemsets, ICDM (International Conference on Data Mining), pages 138–147, 2010.
	LCM ver. 2: Effcient Mining Algorithms for Frequent/Closed/Maximal Itemsets Takeaki Uno1, Masashi Kiyomi1, Hiroki Arimura2 

Questions and bug reporting 


