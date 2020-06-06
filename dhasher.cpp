//
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

int n_threads = 8;
vector<string> job_data;
int result;
int diff;
int t_result;
int hashes = 0;

void *worker(void* thread_id) {
    int t_id = (int)thread_id;
    int part, rest, start, end;
    while(true)
    {
        if(result == -1)
        {
            part = diff / n_threads;
            rest = diff % n_threads;
            start = t_id * part;
            end = t_id * part + part;
            if(t_id == n_threads - 1)
                end += rest;
            for(int iJob = start; iJob <= end; iJob++)
            {
                if(result != -1)
                    break;
                string hashing = sha1(job_data[0]+to_string(iJob));
                hashes++;
                if(hashing.compare(job_data[1]) == 0)
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

void *hs(void * arg) {
    while(true)
    {
        auto sec = TimerInit();
        while(TimerDiff(sec) < 1000) {usleep(1000);}
        printf("Hashrate: %i h/s\n", hashes);
        hashes = 0;
    }
}

int main(int argc, char const *argv[])
{

    easysock::init();

    auto s = easysock::tcp::connect("3.20.98.123", 16198);

    cout << "Server version: v" << s->read() << endl;

    s->write("LOGI,login,password");

    cout << "Logging in: " << s->read() << endl;
    string jobb;
    string check;
    for (int iThr = 0; iThr < n_threads; iThr++)
    {
        pthread_create(nullptr, NULL, worker, (void*)iThr);
    }
    pthread_create(nullptr, NULL, hs, NULL);
    while(true)
    {
        s->write("JOB");
        jobb.clear();
        while(jobb.empty())
        {
  	      jobb = s->read();
        }
        cout << "New job" << endl;
        job_data = StringSplit(jobb.c_str(), ',');
        diff = 100 * stoi(job_data[2]) + 1;
        result = -1;
        auto time = TimerInit();
        int t_found;
        while(true)
        {

            if(result != -1)
            {
                t_found = TimerDiff(time);
                s->write(to_string(result));
                check = s->read();
                if(check.compare("GOOD") == 0 || check.compare("BLOCK") == 0)
                    printf("Thread #%i share accepted (%i) found in %i ms ping %i ms\n", t_result, result, t_found, TimerDiff(time) - t_found);//cout << "accepted (" << result << ")" << endl;
                    break;
                if(check.compare("BAD") == 0)
                    printf("Thread #%i share rejected (%i) %i ms\n", t_result, result, TimerDiff(time));//cout << "rejected (" << result << ")" << endl;
                    break;
            }
        }
    }

    return 0;
}
