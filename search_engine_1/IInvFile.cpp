///////////////////////////////////////////////////////////////////////////////////////
//
// Author: Robert Luk
// Year: 2010
// Robert Luk (c) 2010
//
// Integrated Inverted Index Class Implmenetation:
// This software is made available only to students studying COMP433 (Information
// Retreieval). It should not be used or distributed without consent by the author.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <cmath>
#include <iostream>
#include <tuple>
#include <chrono>

#include "IInvFile.h"

using namespace std::chrono;

// struct _post {
//	int docid;
//	int freq;
// } post;

DocInfo docInfo[500000];

IInvFile::IInvFile()
{
	hsize = 0;
	htable = NULL;
	MaxDocid = 0;
	Files = NULL;
	result = NULL;
}

IInvFile::~IInvFile()
{
	Clear();
}

// Determine the hash value of the string s given the hash table size is h
// hash value is stored in hvalue
int IInvFile::hash(char *s, int h)
{
	char *t = s;
	hvalue = 3;
	if (s == NULL)
		return 0;

	while (*t != '\0')
	{
		hvalue = (31 * hvalue + 57 * ((int)(unsigned char)*t));
		if (hvalue < 0)
			hvalue = -1 * hvalue;
		hvalue = hvalue % h;
		t++;
	}

	return hvalue;
}

// Create a hash table of size h
// Initialize this hash table of pointers to NULL
void IInvFile::MakeHashTable(int h)
{
	if (h > 0)
	{
		hsize = h;
		htable = (hnode **)calloc(h, sizeof(hnode *));
		for (int i = 0; i < h; i++)
		{
			htable[i] = NULL;
		}
	}
}

// Delete the hash table entries and the hash table itself
void IInvFile::Clear()
{
	hnode *w;
	hnode *oldw;
	post *k;
	post *oldk;
	if ((hsize > 0) && (htable != NULL))
	{
		for (int i = 0; i < hsize; i++)
		{
			w = htable[i];
			while (w != NULL)
			{
				oldw = w;
				k = w->posting;
				while (k != NULL)
				{
					oldk = k;
					k = k->next;
					free(oldk);
				}
				free(w->stem);
				w = w->next;
				free(oldw);
			}
		}
		free(htable); // delete the entire hash table
		htable = NULL;
		hsize = 0;
	}
}

// Count the number of postings as the document frequency
int IInvFile::CountDF(post *p)
{
	int cnt = 0;
	post *r = p;
	while (r != NULL)
	{
		cnt++;
		r = r->next;
	}
	return cnt;
}

// Save the data into the file
void IInvFile::Save(char *f)
{
	FILE *fp = fopen(f, "wb"); // open the file for writing
	hnode *w;
	post *k;
	if ((hsize > 0) && (htable != NULL))
	{ // Is there a hash table?
		for (int i = 0; i < hsize; i++)
		{				   // For each hash table entry do
			w = htable[i]; // Go through each hash node
			while (w != NULL)
			{
				fprintf(fp, "%s %d:", w->stem, w->df);
				k = w->posting; // Go through each post node
				while (k != NULL)
				{
					fprintf(fp, "%d %d %d,", k->docid, k->freq, k->loc);
					k = k->next; // next post node
				}
				fprintf(fp, "\r\n");
				w = w->next; // next hash node
			}
		}
	}
	fclose(fp); // close the file
}

// Load the data into RAM
void IInvFile::Load(char *f)
{
	FILE *fp = fopen(f, "rb");
	hnode *w;
	post *k;
	char c;
	bool next;
	int state = 0;
	int cnt;
	char line[1000];
	char stem[1000];
	int df;
	int i;
	int docid;
	int freq;
	int loc;
	ntiTotal = 0;

	if (fp == NULL)
	{
		printf("Aborted: file not found for <%s>\r\n", f);
		return;
	}
	if ((hsize > 0) && (htable != NULL))
	{
		i = 0;
		cnt = 0;
		do
		{
			next = true;
			if (fread(&c, 1, 1, fp) > 0)
			{ // read a character
				switch (state)
				{
				case 0:
					if (c != ':')
						line[i++] = c;
					else
					{
						line[i] = '\0';
						sscanf(line, "%s %d", stem, &df);
						ntiTotal += df;
						w = Find(stem);
						if (w == NULL)
						{
							w = MakeHnode(stem);
							w->df = df;
						}
						i = 0;
						state = 1;
						// printf("Read [%s,%d]\r\n",stem, df);
					}
					break;
				case 1:
					if (c == '\r')
						i = 0;
					else if (c == '\n')
					{
						cnt = 0;
						i = 0;
						state = 0;
					}
					else if (c == ',')
					{
						line[i] = '\0';
						cnt++;
						sscanf(line, "%d %d %d", &docid, &freq, &loc);
						k = w->posting; // push the data into the posting field
						w->posting = new post;
						w->posting->docid = docid;
						w->posting->freq = freq;
						w->posting->loc = loc;
						w->posting->next = k;
						if (MaxDocid < docid)
							MaxDocid = docid;
						// printf("[%d] %d %d,\r\n", cnt, docid, freq);
						i = 0;
					}
					else
						line[i++] = c;
					break;
				}
			}
			else
				next = false;
		} while (next == true);
	}
	else
		printf("Aborted: no hash table\r\n");
	fclose(fp);
	printf("Total number of term: %d\n", ntiTotal);
}

void IInvFile::MakeDocRec()
{

	if (MaxDocid > 0)
	{
		Files = (DocRec *)calloc(MaxDocid + 1, sizeof(DocRec));
		for (int i = 0; i <= MaxDocid; i++)
		{
			Files[i].TRECID = NULL;
			Files[i].len = 0.0;
			Files[i].iwflen = 0.0;
		}
	}
}

float IInvFile::GetIDF(int df)
{
	float idf;
	double N = 1 + (double)MaxDocid;
	idf = (float)log10(N / ((double)df)); // Compute IDF
	return idf;
}

float IInvFile::GetIWF(int df)
{
	float iwf;
	double N = 1 + (double)ntiTotal;
	iwf = (float)log10(N / (double)df); // Compute IWF
	return iwf;
}

// Compute documemt length
void IInvFile::DocLen(DocRec File[])
{
	float idf;
	float idf2;
	float iwf;
	float iwf2;
	hnode *w;
	post *k;

	if ((hsize > 0) && (htable != NULL))
	{ // if there is a hash table (for open hashing)
		for (int i = 0; i < hsize; i++)
		{				   // Loop through every entry in the hash table
			w = htable[i]; // Get the ith hash table entry
			while (w != NULL)
			{						 // Loop through each hash node (hnode) in the linked list
				idf = GetIDF(w->df); // Get the IDF value based on the document frequency, df
				idf2 = idf * idf;	 // square of IDF
				iwf = GetIWF(w->df); // Get the IWF value based on the document frequency, df
				iwf2 = iwf * iwf;	 // square of IWF
				k = w->posting;		 // Loop through each posting in the posting list
				while (k != NULL)
				{
					if (k->docid <= MaxDocid)
					{
						File[k->docid].len += idf2 * (float)(k->freq * k->freq);	// TF*IDF square
						File[k->docid].iwflen += iwf2 * (float)(k->freq * k->freq); // TF*IWF square
					}
					else
						printf("DocLen Error: Docid = %d > Max = %d\r\n", k->docid, MaxDocid);
					k = k->next; // next posting
				}
				w = w->next; // next hash node in the linked list
			}
		}
	}
	else
		printf("Doclen aborted: no hash table\r\n");
	for (int i = 0; i <= MaxDocid; i++)
	{
		File[i].len = (float)sqrt((double)File[i].len);
		File[i].iwflen = (float)sqrt((double)File[i].iwflen);
	}
}

// Save document records
void IInvFile::LoadDocRec(char *f)
{
	FILE *fp;
	char line[10000];
	int i = 0;
	char str[1000];
	float doclen;
	float iwfdoclen;

	if ((MaxDocid > 0) && (Files != NULL))
	{
		printf("LoadDocRec error: already has document records\r\n");
		return;
	}

	fp = fopen(f, "rb");
	if (fp == NULL)
	{
		printf("LoadDocRec error: Cannot find file [%s]\r\n", f);
		return;
	}
	if (fgets(line, 10000, fp) != NULL)
	{
		sscanf(line, "%d", &MaxDocid);
		MaxDocid--;
		MakeDocRec();
		while (fgets(line, 10000, fp) != NULL)
		{
			sscanf(line, "%s %e %e", str, &doclen, &iwfdoclen);
			Files[i].TRECID = strdup(str);
			Files[i].len = doclen;
			Files[i].iwflen = iwfdoclen;
			if (i > MaxDocid)
				printf("LoadDocRec error: MaxDocid incorrect [%d,MaxDocid=%d]\r\n", i, MaxDocid);
			i++;
		}
	}
	fclose(fp);
}

// Save document records
void IInvFile::SaveDocRec(char *f)
{
	FILE *fp;
	if ((MaxDocid > 0) && (Files != NULL))
	{
		fp = fopen(f, "wb");
		fprintf(fp, "%d\r\n", MaxDocid + 1);
		for (int i = 0; i <= MaxDocid; i++)
		{
			if (Files[i].TRECID == NULL)
				fprintf(fp, "%d %e %e\r\n", i, Files[i].len, Files[i].iwflen);
			else
				fprintf(fp, "%s %e %e\r\n", Files[i].TRECID, Files[i].len, Files[i].iwflen);
		}
		fclose(fp);
	}
}

// Find the hnode structure of the string s
// If no such structure exist, return NULL
hnode *IInvFile::Find(char *s)
{
	hnode *r;
	int h = hash(s, hsize); // Get the hash value
	if (htable != NULL)
	{				   // there is a hashtable
		r = htable[h]; // Get the first pointer to the linked list
		while (r != NULL)
		{ // Loop through each hash node
			if (strcmp(r->stem, s) == 0)
				return r; // found the hash node with same stem
			else
				r = r->next; // next hash node
		}
		return r;
	}
	else
		printf("No hash table! \r\n");
	return NULL;
}

// Insert an hnode structure into the hash table
// Initialize the structure with the string s
hnode *IInvFile::MakeHnode(char *s)
{
	hnode *r = new hnode;
	r->stem = strdup(s);
	r->posting = NULL;
	r->df = 0;
	r->next = htable[hvalue];
	htable[hvalue] = r;
	return r;
}

// Find the posting that has the document id
// Assume postings are in reverse order of document ids (i.e., 5, 4, 3, ...)
post *IInvFile::FindPost(hnode *w, int docid)
{
	post *k = w->posting;

	if (k->docid == docid)
		return k;
	else
		return NULL;

	// while (k != NULL) {
	//	if (k->docid == docid) return k;
	//	else if (k->docid > docid) return NULL;
	//	else k = k->next;
	// }
	// return k;
}

// Add a posting
post *IInvFile::Add(char *s, int docid, int freq, int loc)
{
	hnode *w = Find(s); // Does the stem exist in the dictionary?
	post *k = NULL;

	if (w == NULL)
		w = MakeHnode(s); // If not exist, create a new hash node
	else
		k = FindPost(w, docid); // if exists, is the first posting the wanted one?

	if (k == NULL)
	{							   // no posting has the same docid
		k = w->posting;			   // push the data into the posting field
		w->posting = new post;	   // create a new posting record
		w->posting->docid = docid; // save the document id
		w->posting->freq = freq;   // save the term frequency
		w->posting->loc = loc;	   // save the location
		w->df += 1;				   // keep track of the document frequency
		w->posting->next = k;	   // push the data into the posting field
	}
	else
		k->freq += freq; // The posting exists, so add the freq to the freq field

	return k;
}

// Get next query term
char *IInvFile::GotoNextWord(char *s)
{
	char *q = s;
	if ((s == NULL) || (*s == '\0'))
		return NULL;
	while ((*q == ' ') && (*q == '\t'))
		q++;
	while ((*q != ' ') && (*q != '\0'))
		q++;
	if (*q == ' ')
	{
		*q = '\0';
		q++;
	}
	return q;
}

// Combine partial retrieval results
void IInvFile::CombineResult(RetRec *r, post *kk, float idf)
{
	post *k = kk; // Get the pointer to the posting list
	int docid;

	while (k != NULL)
	{
		docid = k->docid;
		if (docid > MaxDocid)
			printf("CombineResult error: Docid = %d > MaxDocID = %d\r\n", k->docid, MaxDocid);
		r[docid].docid = docid;				  // make sure we store the document id
		r[docid].sim += idf * (float)k->freq; // add the partial dot product score
		k = k->next;						  // next posting
	}
}

// Comparison function used by qsort(.): see below
int compare(const void *aa, const void *bb)
{
	RetRec *a = (RetRec *)aa;
	RetRec *b = (RetRec *)bb;

	if (a->sim > b->sim)
		return -1;
	else if (a->sim < b->sim)
		return 1;
	else
		return 0;
}

// Print top N results
void IInvFile::PrintTop(RetRec *r, int top)
{
	int i = MaxDocid + 1;
	qsort(r, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
	i = 0;
	printf("Search Results:\r\n");
	while (i < top)
	{
		if ((r[i].docid == 0) && (r[i].sim == 0.0))
			return; // no more results; so exit
		printf("[%d]\t%d\t%e\r\n", i + 1, r[i].docid, r[i].sim);
		i++;
	}
}

// Perform retrieval
void IInvFile::Search(char *q)
{
	char *s = q;
	char *w;
	bool next = true;
	hnode *h;
	// Initialize the result set
	if (result != NULL)
		free(result);
	result = (RetRec *)calloc(MaxDocid + 1, sizeof(RetRec));

	do
	{
		w = s;				 // Do searching
		s = GotoNextWord(s); // Delimit the term
		if (s == NULL)
			next = false; // If no more terms, exit
		else
		{
			if (*s != '\0')
				*(s - 1) = '\0'; // If not the last term, delimit the term
			Stemming.Stem(w);	 // Stem the term w
			h = Find(w);		 // Find it in the integrated inverted index
			if (h != NULL)		 // Add the scores to the result set
				CombineResult(result, h->posting, GetIDF(h->df));
			else if (strlen(w) > 0)
				printf("Query term does not exist <%s>\r\n", w);
		}
	} while (next == true); // More query terms to handle?

	PrintTop(result, 10); // Print top 10 retrieved results
}

// Load file name information
void IInvFile::LoadFileInfo(char *f)
{
	char tmp[10000];
	int docIndex;
	int docLen;
	char status[10];
	char docName[20];
	char docPath[100];
	int nod = 0;

	FILE *fp = fopen(f, "rb");
	if (fp == NULL)
	{
		printf("Cannot open file \r\n");
		return;
	}

	// get input
	while (fgets(tmp, 10000, fp) != NULL)
	{
		// Get the index, the document length, status, file name, and file path
		sscanf(tmp, "%d %d %s %s %s", &docIndex, &docLen, status, docName, docPath);

		// Load the information
		docInfo[nod].docIndex = docIndex;
		docInfo[nod].docLen = docLen;
		strcpy(docInfo[nod].status, status);
		strcpy(docInfo[nod].docName, docName);
		strcpy(docInfo[nod].docPath, docPath);

		nod++; // number of document
	}
	fclose(fp);
}

// function for Retrieval models
tuple<FILE *, FILE *, bool, bool> IInvFile::GetQueryFile()
{
	bool next = true;
	char cmd[10000];
	FILE *queryFile;
	FILE *searchResult;
	bool skipBelowCode = false;
	printf("Type the query file name or \"_quit\" to exit\r\n");
	cin >> cmd;

	if (strcmp(cmd, "_quit") == 0)
	{
		next = false;
	}
	else
	{
		queryFile = fopen(cmd, "rb");
		if (queryFile == NULL)
		{
			printf("Aborted: file not found for <%s>\r\n", cmd);
			skipBelowCode = true;
		}

		searchResult = fopen("searchResult.txt", "wb");
		if (searchResult == NULL)
		{
			printf("Aborted: Cannot create search result file\r\n");
			fclose(queryFile);
			skipBelowCode = true;
		}
	}
	return std::make_tuple(queryFile, searchResult, next, skipBelowCode);
}

// Interactive retrieval model 1
void IInvFile::RetrievalModelOne()
{
	// Get Query File name
	bool next = true;

	const char *runIdentifier = "GROUP_1";
	FILE *queryFile;
	FILE *searchResult;

	do
	{
		bool skipBelowCode = false;
		tie(queryFile, searchResult, next, skipBelowCode) = GetQueryFile();
		if (skipBelowCode == true)
			continue;
		// Execution time: 1) Get the timepoint before the function is called
		auto start = high_resolution_clock::now();
		// get file content line by line
		char tmp[10000];
		int queryNumber;
		// char *str;
		string str;
		char documentIdentifier[1000];
		int rankNumber = 1;
		float similarityScore;

		while (fgets(tmp, 10000, queryFile) != NULL)
		{
			// Get the query number and the query
			str = tmp; // char[] to string

			// Remove index
			sscanf(tmp, "%d ", &queryNumber);			   // get index
			string query_no = std::to_string(queryNumber); // Int to string
			size_t found_query_no = str.find(query_no);	   // position of query_no
			if (!str.empty() && found_query_no != std::string::npos)
				str.erase(found_query_no, query_no.size()); // Remove
			// Remove \r
			size_t found_r = str.find('\r');
			if (!str.empty() && found_r != std::string::npos)
				str.erase(found_r);
			// Remove \n
			size_t found_n = str.find('\n');
			if (!str.empty() && found_n != std::string::npos)
				str.erase(found_n);
			// skip the front space character
			str.erase(0, 1);

			// Remove special character (e.g. , . ( ) -)
			// Accidentally done it by removing it. Can try try convert it to space character later
			const int no_of_special_char = 5;
			const char special_char[no_of_special_char] = {',', '.', '(', ')', '-'};
			for (int i = 0; i < no_of_special_char; i++)
			{
				bool have_special_char = str.find(special_char[i]) != std::string::npos;
				while (have_special_char == true)
				{ // If there is special char, remove it.
					size_t location = str.find(special_char[i]);
					if (!str.empty() && location != std::string::npos)
						str.erase(location, 1);
					have_special_char = str.find(special_char[i]) != std::string::npos; // Check whether there still have special char
				}
			}

			char *char_str = const_cast<char *>(str.c_str()); // string to char *

			// Search and retrieval data
			char *s = char_str;
			char *w;
			bool nextTerm = true;
			hnode *h;

			// Initialize the result set
			if (result != NULL)
				free(result);
			result = (RetRec *)calloc(MaxDocid + 1, sizeof(RetRec));

			// Count the query length
			int queryLen = 0;

			do
			{
				w = s;				 // Do searching
				s = GotoNextWord(s); // Delimit the term
				if (s == NULL)
					nextTerm = false; // If no more terms, exit
				else
				{
					if (*s != '\0')
						*(s - 1) = '\0'; // If not the last term, delimit the term
					Stemming.Stem(w);	 // Stem the term w
					h = Find(w);		 // Find it in the integrated inverted index
					queryLen++;
					if (h != NULL) // Add the scores to the result set
						CombineResult(result, h->posting, GetIDF(h->df));
					else if (strlen(w) > 0)
						printf("Query term does not exist <%s>\r\n", w);
				}
			} while (nextTerm == true); // More query terms to handle?

			// Normalize
			for (int i = 0; i < MaxDocid; i++)
			{
				float tmp;
				tmp = result[i].sim;
				if (i < 10)
					printf("[%d][%s]: tmp: %f, Files[i].len: %f, sqrt(querylength): %f", i, char_str, tmp, Files[i].len, (float)sqrt((double)queryLen));
				result[i].sim = (tmp / (float)Files[i].len) / (float)sqrt((double)queryLen); // hash result / Document length / Query length
				if (i < 10)
					printf(", result: %f\n", result[i].sim);
			}

			// write data to file
			// fprintf(searchResult, "%d Q0 %s %d %d %s", queryNumber, documentIdentifier, rankNumber, similarityScore, runIdentifier);
			int i = MaxDocid + 1;
			qsort(result, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
			i = 0;
			while (i < 1000)
			{

				if ((result[i].docid == 0) && (result[i].sim == 0.0))
					break; // no more results; so exit

				fprintf(searchResult, "%d 0 %s %d %4f %s\r\n", queryNumber, docInfo[result[i].docid].docName, i, result[i].sim, runIdentifier);

				// result[i].docid need to be fix
				i++;
			}
		}
		// Execution time: 2) Get the timepoint after the funciton is called
		auto stop = high_resolution_clock::now();
		// Execution time: 3) Get the difference in timepoints and cast it to required units
		auto duration = duration_cast<microseconds>(stop - start);
		printf("Searching time: %d microseconds\n", duration);
		fclose(queryFile);
		fclose(searchResult);
	} while (next == true);
}

// Interactive retrieval model 2
void IInvFile::RetrievalModelTwo()
{
	bool next = true;
	const char *runIdentifier = "GROUP_1";
	FILE *queryFile;
	FILE *searchResult;

	do
	{
		bool skipBelowCode = false;
		tie(queryFile, searchResult, next, skipBelowCode) = GetQueryFile();
		if (skipBelowCode == true)
			continue;
		// Execution time: 1) Get the timepoint before the function is called
		auto start = high_resolution_clock::now();

		// get file content line by line
		char tmp[10000];
		int queryNumber;
		// char *str;
		string str;
		char documentIdentifier[1000];
		int rankNumber = 1;
		float similarityScore;

		while (fgets(tmp, 10000, queryFile) != NULL)
		{
			// Get the query number and the query
			str = tmp; // char[] to string

			// Remove index
			sscanf(tmp, "%d ", &queryNumber);			   // get index
			string query_no = std::to_string(queryNumber); // Int to string
			size_t found_query_no = str.find(query_no);	   // position of query_no
			if (!str.empty() && found_query_no != std::string::npos)
				str.erase(found_query_no, query_no.size()); // Remove
			// Remove \r
			size_t found_r = str.find('\r');
			if (!str.empty() && found_r != std::string::npos)
				str.erase(found_r);
			// Remove \n
			size_t found_n = str.find('\n');
			if (!str.empty() && found_n != std::string::npos)
				str.erase(found_n);
			// skip the front space character
			str.erase(0, 1);

			// Remove special character (e.g. , . ( ) -)
			// Accidentally done it by removing it. Can try try convert it to space character later
			const int no_of_special_char = 5;
			const char special_char[no_of_special_char] = {',', '.', '(', ')', '-'};
			for (int i = 0; i < no_of_special_char; i++)
			{
				bool have_special_char = str.find(special_char[i]) != std::string::npos;
				while (have_special_char == true)
				{ // If there is special char, remove it.
					size_t location = str.find(special_char[i]);
					if (!str.empty() && location != std::string::npos)
						str.erase(location, 1);
					have_special_char = str.find(special_char[i]) != std::string::npos; // Check whether there still have special char
				}
			}

			char *char_str = const_cast<char *>(str.c_str()); // string to char *

			// Search and retrieval data
			char *s = char_str;
			char *w;
			bool nextTerm = true;
			hnode *h;

			// Initialize the result set
			if (result != NULL)
				free(result);
			result = (RetRec *)calloc(MaxDocid + 1, sizeof(RetRec));

			// Count the query length
			int queryLen = 0;

			do
			{
				// s == query
				// w == current stem in a query
				// When retrieving each stem to w, each stem would be removed from s.
				w = s;				 // Do searching
				s = GotoNextWord(s); // Delimit the term
				if (std::strspn(s, " ") == std::strlen(s))
					nextTerm = false; // If no more terms, exit
				else
				{
					if (*s != '\0')
						*(s - 1) = '\0'; // If not the last term, delimit the term
					Stemming.Stem(w);	 // Stem the term w
					h = Find(w);		 // Find it in the integrated inverted index
					queryLen++;

					// Phrasal query processing
					if (h != NULL)
					{
						bool passPhrasalQueryTest = false;
						const int range = 10;
						post *previousPosting = NULL;
						post *currentPosting = h->posting;
						// if s only contain " ", no stem in s anymore. continue.
						if (std::strspn(s, " ") == std::strlen(s))
							continue;
						string sStr = s;
						char *nextStem = const_cast<char *>(sStr.substr(0, sStr.find(" ")).c_str());
						Stemming.Stem(nextStem);
						hnode *nextStemHnode = Find(nextStem);
						if (nextStemHnode != NULL)
						{
							post *nextStemPosting = nextStemHnode->posting;
							while (currentPosting != NULL)
							{
								int stemPosition = currentPosting->loc;
								post *tempForNextStemPosting = nextStemPosting;
								while (tempForNextStemPosting != NULL)
								{
									if (currentPosting->docid != tempForNextStemPosting->docid)
									{
										tempForNextStemPosting = tempForNextStemPosting->next;
										continue;
									}

									if (tempForNextStemPosting->loc > stemPosition && tempForNextStemPosting->loc <= stemPosition + range)
									{
										passPhrasalQueryTest = true;
										break;
									}
									tempForNextStemPosting = tempForNextStemPosting->next;
								}
								// Remove posting in hnode if it did not pass the test
								if (passPhrasalQueryTest == false && previousPosting != NULL && currentPosting->next != NULL)
								{
									previousPosting->next = currentPosting->next;
								}
								previousPosting = currentPosting;
								currentPosting = currentPosting->next;
							}

							if (h != NULL) // Add the scores to the result set
								CombineResult(result, h->posting, GetIDF(h->df));
							else if (strlen(w) > 0)
								printf("Query term does not exist <%s>\r\n", w);
						}
					}
				}
			} while (nextTerm == true); // More query terms to handle?

			// Normalize
			for (int i = 0; i < MaxDocid; i++)
			{
				float tmp;
				tmp = result[i].sim;
				if (i < 10)
					printf("[%d][%s]: tmp: %f, Files[i].len: %f, sqrt(querylength): %f", i, char_str, tmp, Files[i].len, (float)sqrt((double)queryLen));
				result[i].sim = (tmp / (float)Files[i].len) / (float)sqrt((double)queryLen); // hash result / Document length / Query length
				if (i < 10)
					printf(", result: %f\n", result[i].sim);
			}

			// write data to file
			// fprintf(searchResult, "%d Q0 %s %d %d %s", queryNumber, documentIdentifier, rankNumber, similarityScore, runIdentifier);
			int i = MaxDocid + 1;
			qsort(result, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
			i = 0;
			while (i < 1000)
			{

				if ((result[i].docid == 0) && (result[i].sim == 0.0))
					break; // no more results; so exit

				fprintf(searchResult, "%d 0 %s %d %4f %s\r\n", queryNumber, docInfo[result[i].docid].docName, i, result[i].sim, runIdentifier);

				// result[i].docid need to be fix
				i++;
			}
		}
		// Execution time: 2) Get the timepoint after the funciton is called
		auto stop = high_resolution_clock::now();
		// Execution time: 3) Get the difference in timepoints and cast it to required units
		auto duration = duration_cast<microseconds>(stop - start);
		printf("Searching time: %d microseconds\n", duration);
		fclose(queryFile);
		fclose(searchResult);
	} while (next == true);
}
// Retrieval model 3: TF-IWF
void IInvFile::RetrievalModelThree()
{

	// Get Query File name
	bool next = true;

	const char *runIdentifier = "GROUP_3";
	FILE *queryFile;
	FILE *searchResult;

	do
	{
		bool skipBelowCode = false;
		tie(queryFile, searchResult, next, skipBelowCode) = GetQueryFile();
		if (skipBelowCode == true)
			continue;
		// Execution time: 1) Get the timepoint before the function is called
		auto start = high_resolution_clock::now();

		// get file content line by line
		char tmp[10000];
		int queryNumber;
		// char *str;
		string str;
		char documentIdentifier[1000];
		int rankNumber = 1;
		float similarityScore;

		while (fgets(tmp, 10000, queryFile) != NULL)
		{
			// Get the query number and the query
			str = tmp; // char[] to string

			// Remove index
			sscanf(tmp, "%d ", &queryNumber);			   // get index
			string query_no = std::to_string(queryNumber); // Int to string
			size_t found_query_no = str.find(query_no);	   // position of query_no
			if (!str.empty() && found_query_no != std::string::npos)
				str.erase(found_query_no, query_no.size()); // Remove
			// Remove \r
			size_t found_r = str.find('\r');
			if (!str.empty() && found_r != std::string::npos)
				str.erase(found_r);
			// Remove \n
			size_t found_n = str.find('\n');
			if (!str.empty() && found_n != std::string::npos)
				str.erase(found_n);
			// skip the front space character
			str.erase(0, 1);

			// Remove special character (e.g. , . ( ) -)
			// Accidentally done it by removing it. Can try try convert it to space character later
			const int no_of_special_char = 5;
			const char special_char[no_of_special_char] = {',', '.', '(', ')', '-'};
			for (int i = 0; i < no_of_special_char; i++)
			{
				bool have_special_char = str.find(special_char[i]) != std::string::npos;
				while (have_special_char == true)
				{ // If there is special char, remove it.
					size_t location = str.find(special_char[i]);
					if (!str.empty() && location != std::string::npos)
						str.erase(location, 1);
					have_special_char = str.find(special_char[i]) != std::string::npos; // Check whether there still have special char
				}
			}

			char *char_str = const_cast<char *>(str.c_str()); // string to char *

			// Search and retrieval data
			char *s = char_str;
			char *w;
			bool nextTerm = true;
			hnode *h;

			// Initialize the result set
			if (result != NULL)
				free(result);
			result = (RetRec *)calloc(MaxDocid + 1, sizeof(RetRec));

			// Count the query length
			int queryLen = 0;

			do
			{
				w = s;				 // Do searching
				s = GotoNextWord(s); // Delimit the term
				if (s == NULL)
					nextTerm = false; // If no more terms, exit
				else
				{
					if (*s != '\0')
						*(s - 1) = '\0'; // If not the last term, delimit the term
					Stemming.Stem(w);	 // Stem the term w
					h = Find(w);		 // Find it in the integrated inverted index
					queryLen++;
					if (h != NULL) // Add the scores to the result set
						CombineResult(result, h->posting, GetIWF(h->df));
					else if (strlen(w) > 0)
						printf("Query term does not exist <%s>\r\n", w);
				}
			} while (nextTerm == true); // More query terms to handle?

			// Normalize
			for (int i = 0; i < MaxDocid; i++)
			{
				float tmp;
				tmp = result[i].sim;
				if (i < 10)
					printf("[%d][%s]: tmp: %f, Files[i].len: %f, sqrt(querylength): %f", i, char_str, tmp, Files[i].len, (float)sqrt((double)queryLen));
				result[i].sim = (tmp / (float)Files[i].len) / (float)sqrt((double)queryLen); // hash result / Document length / Query length
				if (i < 10)
					printf(", result: %f\n", result[i].sim);
			}

			// write data to file
			// fprintf(searchResult, "%d Q0 %s %d %d %s", queryNumber, documentIdentifier, rankNumber, similarityScore, runIdentifier);
			int i = MaxDocid + 1;
			qsort(result, MaxDocid + 1, sizeof(RetRec), compare); // qsort is a C function: sort results
			i = 0;
			while (i < 1000)
			{

				if ((result[i].docid == 0) && (result[i].sim == 0.0))
					break; // no more results; so exit

				fprintf(searchResult, "%d 0 %s %d %4f %s\r\n", queryNumber, docInfo[result[i].docid].docName, i, result[i].sim, runIdentifier);

				// result[i].docid need to be fix
				i++;
			}
		}
		// Execution time: 2) Get the timepoint after the funciton is called
		auto stop = high_resolution_clock::now();
		// Execution time: 3) Get the difference in timepoints and cast it to required units
		auto duration = duration_cast<microseconds>(stop - start);
		printf("Searching time: %d microseconds\n", duration);
		fclose(queryFile);
		fclose(searchResult);
	} while (next == true);
}