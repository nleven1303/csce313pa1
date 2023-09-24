/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Nate Leven
	UIN: 231009586
	Date: 09/24/23
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <string>

using namespace std;


int main (int argc, char *argv[]) {

	int pid = fork();
	if(pid == 0){
		execvp("./server", argv);
	}
	else {
		FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
		int opt;
		int p = 1;
		double t = 0.0;
		int e = 1;
		int j = 0;
		bool newChan = false; //serve as flags
		int c = 0;
		int m = MAX_MESSAGE;
		
		string filename = "";
		while ((opt = getopt(argc, argv, "p:t:e:f:m:j:c")) != -1) {
			switch (opt) {
				case 'p':
					p = atoi (optarg);
					break;
				case 't':
					t = atof (optarg);
					break;
				case 'e':
					e = atoi (optarg);
					break;
				case 'f':
					filename = optarg;
					break;
				case 'm':
					m = atoi (optarg);
					break;
				case 'j':
					m = atoi(optarg);
					break;
				case 'c':
					newChan = true;
					break;
				default:
					break;
			}
		}
		//checks to make sure that the user inputs correct values for time, person, and ecgno
		if ((t < 0 || t > 59.996) && (t != -1)) { 
			EXITONERROR("invalid time");
		}
		if (p < 1 || p > 15) {
			EXITONERROR("invalid person");
		}
		if (e != 1 && e != 2) {
			EXITONERROR("invalid ecgno");
		}

		if (j == 1)
		{ // requesting 1 datapoint
			// this datamsg is constructed with values p, t, e incoming from user terminal input
			datamsg *x = new datamsg(p, t, e); // dynamic allocation of x

			chan.cwrite(x, sizeof(datamsg)); // sending to server a request x

			double *datarec = new double;		 // double value for ecg of requested patient @ time and ecgno type
			chan.cread(datarec, sizeof(double)); // cread from server with data going to pointer to double datarec

			std::cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << *datarec << endl; // outputting the result

			// free the dynamic memory from the heap
			delete x;
			delete datarec;
		}
		else if (j > 1)
		{ // request for multiple msgs to output to a csv file
			// timing analysis vars
			struct timeval start;
			struct timeval end;
			gettimeofday(&start, NULL); // start
			// need to open an output file
			ofstream outfile;
			string person = to_string(p);						 // cast
			string filename_mp = "received/x" + person + ".csv"; // file + location dir
			outfile.open(filename_mp);							 // file open

			// now send cwrite to server to request the number of datapoints, with output of both ecgno's
			for (int i = 0; i < j; i++)
			{
				outfile << t << ",";
				// send request msg to server for ecg1
				datamsg *mp_datarq = new datamsg(p, t, 1); // since we are looking at first ecg and second one
				chan.cwrite(mp_datarq, sizeof(datamsg));
				// read the ecg double value from the buf
				double *ecg = new double;
				chan.cread(ecg, sizeof(double));
				// send the first value to the x1.csv file
				outfile << *ecg << ",";

				// now repeat for ecgno 2
				datamsg *mp_datarq2 = new datamsg(p, t, 2); // ecg now 2
				chan.cwrite(mp_datarq2, sizeof(datamsg));
				double *ecg2 = new double;
				chan.cread(ecg2, sizeof(double));
				outfile << *ecg2 << endl;
				// free heap memory
				delete mp_datarq;
				delete mp_datarq2;
				delete ecg;
				delete ecg2;
				// increments
				if (t > 59.996)
					t = 0; // reset when past 59.996
				t += .004; // next time index
			}
			outfile.close();		  // close ofstream outfile
			gettimeofday(&end, NULL); // end of timing
			// runtime in us
			double time_taken = (end.tv_sec - start.tv_sec) * 1e6;			  // us to sec
			time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6; // sec to us
			std::cout << "Time taken to request " << j << " datapoints: " << time_taken  << " seconds" << endl;
			std::cout << "This is equivalent to " << time_taken << "e6 microseconds" << endl;
		}

				// requests for the data points
				char *buf = new char[m];		   // 256
				datamsg *x = new datamsg(p, t, e); // change from hardcoding to user's values

				memcpy(buf, &x, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // question
				double reply;
				chan.cread(&reply, sizeof(double)); // answer
				cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;

				// sending a non-sense message, you need to change this
				filemsg fm(0, 0);
				string fname = "teslkansdlkjflasjdf.dat";

				int len = sizeof(filemsg) + (fname.size() + 1);
				char *buf2 = new char[len];
				memcpy(buf2, &fm, sizeof(filemsg));
				strcpy(buf2 + sizeof(filemsg), fname.c_str());
				chan.cwrite(buf2, len); // I want the file length;

				delete[] buf2;

				// closing the channel
				MESSAGE_TYPE m = QUIT_MSG;
				chan.cwrite(&m, sizeof(MESSAGE_TYPE));
			}
		}
