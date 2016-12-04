/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Name                     :Isaac Styles
// Department Name : Computer and Information Sciences
// File Name                :hw4.cc
// Purpose                  :Read file. Create seven threads which queue Product_Records sequentially. Then write results to file.
// Author			        : Isaac Styles, styles@goldmail.etsu.edu
// Create Date	            :Mar 11, 2016
//
//-----------------------------------------------------------------------------------------------------------
//
// Modified Date	: Mar 28, 2016
// Modified By		: Isaac Styles
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define DEBUG
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "product_record.h"
#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include "SafeQueue.h"
using namespace std;
//Function prototypes
void* station0(void*);
void* station1(void*);
void* station2(void*);
void* station3(void*);
void* station4(void*);
void* writeFileT(void*);
void* readFileT(void*);
int readRecordFromFile(fstream&, product_record&);
int writeRecordToFile(fstream&, product_record&);
void displayRecord(product_record&);

//Global data
SafeQueue<product_record> queues[6];                     //for communication between threads

int main(int argc, char** argv)
{
    char infile [40];
    char outfile [40];
    pthread_t children [7];                             //array of child thread numbers
    int errorCode = 0;                                  //exit code

    strcpy(infile, argv[1]);
    strcpy(outfile, argv[2]);
    if(argc == 3)                                       //if proper number of command arguments
    {
        for(int child = 0; child<7; child++)            //create 7 threads
        {
            int initError = -1;                         // !== 0 -> failed to create thread
            switch (child)
            {
            case 0:
                initError = pthread_create(&children[child], NULL, readFileT, (void*)&infile);
                break;
            case 1:
                initError =pthread_create(&children[child], NULL, station0, NULL);
                break;
            case 2:
                initError =pthread_create(&children[child], NULL, station1, NULL);
                break;
            case 3:
                initError =pthread_create(&children[child], NULL, station2, NULL);
                break;
            case 4:
                initError =pthread_create(&children[child], NULL, station3, NULL);
                break;
            case 5:
                initError =pthread_create(&children[child], NULL, station4, NULL);
                break;
            case 6:
                initError =pthread_create(&children[child], NULL, writeFileT, (void*)&outfile);
                break;
            }
            if (initError != 0)
            {
                cout << "Could not create thread " << child << endl;
                errorCode--;
            }
        }
        for (int child = 0; child < 7 ; child++)
        {
            pthread_join(children[child], NULL);        //wait on threads
        }
    }
    else{                                               //incorrect arguments
        cout << "Please use command arguments for infile and outfile." << endl;
    }
    return errorCode;
}
/// <summary>
/// Reads input file and writes to Station0 queue, then writes all-done record
/// </summary>
void* readFileT(void *argv)
{
    char* a = (char*)argv;                        //copy command params
    int errorCode = 0;                              //error code for write process
    product_record pr;                              //holder for current record
    fstream fin (a,fstream::in);			        //input file
    errorCode = readRecordFromFile(fin, pr);        //get a record from file
    while (errorCode == 0)                          //while more records in file
    {
        queues[0].enqueue(pr);                         //write record to station 0 pipe
        errorCode = readRecordFromFile(fin, pr);    //get another record
    }
    pr = product_record();                          //initialize terminate record
    pr.idnumber= -1;
    queues[0].enqueue(pr);                         //write record to station 0 pipe

    fin.close();                                    //close infile filestream
    pthread_exit(0);
}
/// <summary>
/// Compute tax for a product_record, then depending on idnumber, pipe the record to either station 1 or 2
/// </summary>
void* station0(void*)
{
    size_t processed = 0;                                           //number of records processed
    product_record pr;
    do
    {
        pr = queues[0].dequeue();
        if (pr.idnumber != -1)                                      //if
        {
            processed++;                                                //increment number records processed
            pr.tax = pr.price * pr.number * 0.05;                       //calculate tax
            pr.stations[0] = 1;
        }
        if (pr.idnumber < 1000)                                    //if idnumber indicates shipping
        {
            queues[1].enqueue(pr);                                     //write sANDh to station 1
        }
        else
        {
            queues[2].enqueue(pr);                                     //write sANDh free to station 2
        }
    }while(pr.idnumber != -1);
    cout << "Station 0 processed " << processed <<" product records."<< endl;
    pthread_exit(0);
}
/// <summary>
/// compute shipping and handling
/// </summary>
void* station1(void*)
{
    size_t processed = 0;                                           //number of records processed
    product_record pr;
    do
    {
        pr = queues[1].dequeue();
        if (pr.idnumber != -1)                                      //for handling of empty file
        {
            processed++;                                            //increment number records processed
            pr.sANDh = (pr.price*pr.number)*.01 + 10;
            pr.stations[1] = 1;
        }
        queues[2].enqueue(pr);                                         //write to station2

    }while(pr.idnumber != -1);
    cout << "Station 1 processed " << processed <<" product records."<< endl;

    pthread_exit(0);
}
/// <summary>
/// compute order total
/// </summary>
void* station2(void*)
{
    size_t processed = 0;                                               //number of records processed
    product_record pr;

    do
    {
        pr = queues[2].dequeue();
        if (pr.idnumber != -1)                                      //for handling of empty file
        {
            processed++;                                                //increment number records processed
            pr.total = (pr.number*pr.price) + pr.sANDh + pr.tax;
            pr.stations[2] = 1;
        }
        queues[3].enqueue(pr);                                         //write to station 3
    }while(pr.idnumber != -1);
    cout << "Station 2 processed " << processed <<" product records."<< endl;

    pthread_exit(0);
}
/// <summary>
/// compute and display running total
/// </summary>
void* station3(void*)
{
    size_t processed = 0;                                               //number of records processed
    product_record pr;
    double runningTotal = 0;                                        //holds the total of records processed
    do
    {
        pr = queues[3].dequeue();
        if (pr.idnumber != -1)                                      //for handling of empty file
        {
            processed++;                                                //increment number records processed
            runningTotal += pr.total;
            pr.stations[3] = 1;
            cout << "Running Total: " <<runningTotal<< endl;
        }
        queues[4].enqueue(pr);                                         //write current record to station 4
    }while(pr.idnumber != -1);
    cout << "Station 3 processed " << processed <<" product records."<< endl;
    pthread_exit(0);
}
/// <summary>
/// display the record and pipe back to parent
/// </summary>
void* station4(void*)
{
    size_t processed = 0;                                               //number of records processed
    product_record pr;

    do
    {
        pr = queues[4].dequeue();
        if (pr.idnumber != -1)                                      //for handling of empty file
        {
            processed++;                                                //increment number records processed
            pr.stations[4] = 1;
            displayRecord(pr);
        }
        queues[5].enqueue(pr);      //write to writeThread queue
    }while(pr.idnumber != -1);
    cout << "Station 4 processed " << processed <<" product records."<< endl;

    pthread_exit(0);
}
/// <summary>
/// Reads input file and pipes the records to
/// </summary>
void* writeFileT(void* argv)
{
    char* a = (char*)argv;                        //copy command params
    size_t recordsRcvd = 0;
    int errorCode = 0;
    product_record pr;                              //holder for current record

    fstream fout (a,fstream::out);			// initialize output file
    do                 //while there are unprocessed records
    {
        pr = queues[5].dequeue();
        if (pr.idnumber == -1)                      //for handling of empty file
        {
            break;
        }
        recordsRcvd++;
        errorCode += writeRecordToFile(fout, pr);                //write record to file
    } while(errorCode >= 0);
    fout.close();                                   //close output filestream

    pthread_exit(0);
}
/// <summary>
/// Displays an array of product_record to standardoutput
/// </summary>
/// <param name="p">The array of product_record</param>
void displayRecord(product_record& p)
{
    cout << "idnumber = " << p.idnumber << endl;
    cout << "    name = " << p.name << endl;
    cout << "   price = " << p.price << endl;
    cout << "  number = " << p.number << endl;
    cout << "     tax = " << p.tax << endl;
    cout << "     s&h = " << p.sANDh << endl;
    cout << "   total = " << p.total << endl;
    cout << "stations = " << p.stations[0] << " " << p.stations[1] << " " << p.stations[2] << " " << p.stations[3] << " " << p.stations[4] << " " << endl;
    cout << "\n\tEND RECORD" << endl;
}
/// <summary>
/// Creates an array of product_records from a file.
/// </summary>
/// <param name="f">The file stream.</param>
/// <param name="pp">The ref to product_record</param>
/// <param name="objs">The number objs read in.</param>
/// <returns>-1 if failure, 0 if success</returns>
int readRecordFromFile(fstream& f, product_record& p)
{
	//int objs = 0;											//number of objects read in = 0
	string line;										    //current input line

	if (!f)
	{
		cout << "could not read input file" << endl;
		return -2;
	}
	else
	{
        int errorcode = -1;                                 //return -1 if empty file
		if (getline(f, line))
		{
		    p.idnumber = atoi(line.c_str());			//idnumber
			getline(f, line);							//name
			strncpy(p.name,line.c_str(), line.length()+1);			//copy name to struct
			getline(f,line);							//price
			p.price = atof(line.c_str());
			getline(f, line);							//number
			p.number = atoi(line.c_str());
			getline(f, line);							//tax
			p.tax = atof(line.c_str());
			getline(f, line);							//sANDh
			p.sANDh = atof(line.c_str());
			getline(f, line);							//total
			p.total = atof(line.c_str());
			getline(f, line);							//read stations vector
			p.stations[0] = 0;
			p.stations[1] = 0;
			p.stations[2] = 0;
			p.stations[3] = 0;
			p.stations[4] = 0;
			errorcode = 0;
		}
		return errorcode;
	}
}
/// <summary>
/// Writes product_record to opened file.
/// </summary>
/// <param name="f">The file stream</param>
/// <param name="p">The pointer to product_record</param>
/// <returns>-1 on failure, 0 on success</returns>
int writeRecordToFile(fstream& f, product_record& p)
{
	// For practice, open an output file
	// Options:
	//    fstream::out             for write, kills existing file
	//    fstream::in              for read-only
	//    fstream::in | ios::out   for read/write
	//    fstream::binary          for binary access

	// Always check that the file opened correctly
	if (!f)
	{
		// f.open( ) failed
		cout << "output file failed on open" << endl;
		return -1;
	}
	else {
        f << p.idnumber << endl;
        f << p.name << endl;
        f << p.price << endl;
        f << p.number << endl;
        f << p.tax << endl;
        f << p.sANDh << endl;
        f << p.total << endl;
        f << p.stations[0] << " " << p.stations[1] << " " << p.stations[2] << " " << p.stations[3] << " " << p.stations[4] << " " << endl;
    }
	return 0;
}

