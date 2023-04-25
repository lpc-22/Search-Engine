///////////////////////////////////////////////////////////////////////////////////////
//
// Author: Robert Luk
// Year: 2010
// Robert Luk (c) 2010
//
// Interactive Retrieval using the Integrated Inverted Index Class:
// This software is made available only to students studying COMP433 (Information
// Retreieval). It should not be used or distributed without consent by the author.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <chrono>

#include "IInvFile.h"

using namespace std::chrono;

// Integrated Inverted Index (see lecture notes on Implementation)
IInvFile InvFile;

int main()
{
	// Execution time: 1) Get the timepoint before the function is called
	auto start = high_resolution_clock::now();

	// Initialize the Hash Table
	InvFile.MakeHashTable(13023973);

	printf("Loading Inverted File\r\n");
	InvFile.Load("InvFile.txt");

	printf("Load Document Lengths\r\n");
	InvFile.LoadDocRec("InvFile.doc");

	printf("Load file name information\r\n");
	;
	InvFile.LoadFileInfo("file.txt");

	// Execution time: 2) Get the timepoint after the funciton is called
	auto stop = high_resolution_clock::now();
	// Execution time: 3) Get the difference in timepoints and cast it to required units
	auto duration = duration_cast<microseconds>(stop - start);
	printf("Model loading time: %d microseconds\n", duration);

	// Start interactive retrieval
	bool exit = false;
	string searchEngine;
	while (exit == false)
	{
		cout << "Please choose search engine:\n1. Base search engine\n2.Search engine with query processing\n3.Search engine with TF-IWF\nType exit to exit the program.\n";
		cin >> searchEngine;
		if (searchEngine == "1")
			InvFile.RetrievalModelOne();
		else if (searchEngine == "2")
			InvFile.RetrievalModelTwo();
		else if (searchEngine == "3")
			InvFile.RetrievalModelThree();
		else if (searchEngine == "exit")
			exit = true;
	}
	return 0;
}
