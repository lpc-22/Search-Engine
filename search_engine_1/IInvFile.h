///////////////////////////////////////////////////////////////////////////////////////
//
// Author: Robert Luk
// Year: 2010
// Robert Luk (c) 2010
//
// Integrated Inverted Index Declaration:
// This software is made available only to students studying COMP433 (Information
// Retreieval). It should not be used or distributed without consent by the author.
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef _IINVFILE_
#define _IINVFIEL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stem.h"

// Posting structure
typedef struct _post
{
	int docid;			// document id
	int freq;			// term frequency
	int loc;			// Position
	struct _post *next; // next pointer
} post;

// Hash node structure
typedef struct _hnode
{
	char *stem;			 // stem
	post *posting;		 // pointer to postings
	int df;				 // document frequency
	struct _hnode *next; // next pointer
} hnode;

// Document related information
typedef struct _DocRec
{
	float iwflen;
	float len;
	char *TRECID;
} DocRec;

// Retrieval record
typedef struct _RetRec
{
	int docid;
	float sim;
} RetRec;

// Retriecal file name information
typedef struct _DocInfo
{
	int docIndex;
	int docLen;
	char status[10];
	char docName[20];
	char docPath[100];
} DocInfo;

// Integrated Inverted Index Class
class IInvFile
{
public:
	IInvFile();			 // Constructor
	virtual ~IInvFile(); // Deconstructor

	// Hashing related functions and values
	int hsize;				   // hash table size
	int hvalue;				   // current hash value
	int hash(char *s, int h);  // compute the hash value of s
	hnode **htable;			   // hash table pointer
	void MakeHashTable(int h); // Create a hash table
	void Clear();			   // Clear the hash table entries and postings
	hnode *Find(char *s);	   // Find the hash node that has the same stem as s
	hnode *MakeHnode(char *s); // Create a new hash node

	// TF-IWF
	int ntiTotal; // Store the total number of nti

	// File information
	int MaxDocid;  // the number of files indexed
	DocRec *Files; // An array to store information about each file

	// Inverted File processing
	post *FindPost(hnode *w, int docid); // Got the lastest posting?
	float GetIDF(int df);				 // Compute the IDF value using df
	float GetIWF(int df);				 // Compute the IWF value using df
	post *Add(char *s, int docid, int f, int loc); // Add a posting into the integrated inverted index
	int CountDF(post *p);						   // Obsolete (count df from posting list)
	void Save(char *f);							   // Save the integrated inverted index to the file f
	void Load(char *f);							   // Load the file data into the integrated inverted index

	// Document length
	void MakeDocRec();			// allocate document records
	void DocLen(DocRec File[]); // Compute document lengths
	void SaveDocRec(char *f);	// Save document record information
	void LoadDocRec(char *f);	// Load document record information

	// Retrieval
	void LoadFileInfo(char *f);						   // Load file name information
	RetRec *result;									   // Retrieval result set pointer
	void PrintTop(RetRec *r, int N);				   // Print the top N retrieved documents
	void CombineResult(RetRec *r, post *p, float idf); // Combine the partial retrieval results
	stemmer Stemming;								   // Stemmer
	char *GotoNextWord(char *s);					   // Delimit the next query term
	void Search(char *q);							   // Search one query
	tuple<FILE *, FILE *, bool, bool> GetQueryFile();  // Function for retrieval models
	void RetrievalModelOne();						   // Interactive retrieval model 1
	void RetrievalModelTwo();						   // Interactive retrieval model 2
	void RetrievalModelThree();						   // Interactive retrieval model 3
};

#endif // _IINVFILE_
