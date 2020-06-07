/*
dhasher v1.0.0
copyrights (C) 2020 MrKris7100
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include "sha1.cpp"
#include "tcp.hpp"
#include <sstream>
#include <chrono>

#define WIN32

using namespace std;

//Function for splitting string into array using specified delimiter
vector<string> StringSplit(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}
//Global variables
int n_threads = 4; //number of hashing threads
vector<string> job_data; //job data received from server
int result; //result variable
int diff; //difficulty variable
int t_result; //which thread found solution identificator
int hashes = 0; //hash counter

//Hashing thread
void *worker(void* thread_id) {
    int t_id = (int)thread_id; //thread identificator
    int part, rest, start, end;
    while(true)
    {
        if(result == -1) //there is job to do
        {
            //Calculating which part this thread should calculate
            part = diff / n_threads;
            rest = diff % n_threads;
            start = t_id * part;
            end = t_id * part + part;
            if(t_id == n_threads - 1) //adding rest of dividing
                end += rest;
            for(int iJob = start; iJob <= end; iJob++) //hashing loop
            {
                if(result != -1) //if result was found by other thread break loop
                    break;
                string hashing = sha1(job_data[0]+to_string(iJob));
                hashes++; //hash counting
                if(hashing.compare(job_data[1]) == 0) //result founded
                {
                    result = iJob;
                    t_result = t_id;
                    break;
                }
            }
            while(result == -1) {usleep(1000);}
        }
    usleep(1000);
    }
}
//function for time measurment
chrono::high_resolution_clock::time_point TimerInit()
{
    return chrono::high_resolution_clock::now();
}

int TimerDiff(chrono::high_resolution_clock::time_point handle)
{
    auto t = chrono::high_resolution_clock::now();
    chrono::duration<int64_t,nano> e = t - handle;
    return (int)e.count() / 1000000;
}
//Hashrate report thread
void *hs(void * arg) {
    vector<int> hashrate;
    while(true)
    {
        auto sec = TimerInit();
        while(TimerDiff(sec) < 1000) {usleep(1000);}
        if(hashrate.size() < 50)
        {
            hashrate.push_back(hashes);
        } else {
            rotate(hashrate.begin(), hashrate.begin()+1, hashrate.end());
            hashrate[49] = hashes;
        }
        hashes = 0;
        int average = 0;
        for(int i = 0; i < hashrate.size(); i++)
            average += hashrate[i];
        average /= hashrate.size();
        printf("Hashrate: %i h/s\n", average);
        hashes = 0;
    }
}

int main(int argc, char const *argv[])
{
    easysock::init(); // init socket library

    auto s = easysock::tcp::connect("3.20.98.123", 16198); //connect to server

    cout << "Server version: v" << s->read() << endl; //read server version

    s->write("LOGI,login,password"); //login to server

    cout << "Logging in: " << s->read() << endl; // read user is logged
    string jobb;
    string check;
    for (int iThr = 0; iThr < n_threads; iThr++) //Creating N threads for hashing
    {
        pthread_create(nullptr, NULL, worker, (void*)iThr);
    }
    pthread_create(nullptr, NULL, hs, NULL); //Creating hashrate report thread
    while(true)
    {
        s->write("JOB"); //Get job from server
        jobb.clear();
        while(jobb.empty())
        {
  	      jobb = s->read(); //read job from server
        }
        cout << "New job" << endl;
        job_data = StringSplit(jobb.c_str(), ','); // split data into pieces
        diff = 100 * stoi(job_data[2]) + 1; //calculate loop range
        result = -1; //clear last result
        auto time = TimerInit(); //init time measurment
        int t_found;
        while(true)
        {

            if(result != -1) //Some thread found solution
            {
                t_found = TimerDiff(time);
                s->write(to_string(result)); // send result to server
                check = s->read();
                if(check.compare("GOOD") == 0 || check.compare("BLOCK") == 0) //checking result was good
                    printf("Thread #%i share accepted (%i) found in %i ms ping %i ms\n", t_result, result, t_found, TimerDiff(time) - t_found);//cout << "accepted (" << result << ")" << endl;
                    break;
                if(check.compare("BAD") == 0) // result was bad
                    printf("Thread #%i share rejected (%i) %i ms\n", t_result, result, TimerDiff(time));//cout << "rejected (" << result << ")" << endl;
                    break;
            }
        }
    }

    return 0;
}
