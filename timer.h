/* ========================================================================
   $HEADER FILE
   $File: timer.h $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/17 $
   $Description: This file contains all of macros/struct that are required
                 to time other function. $
   $Revisions: $
   ======================================================================== */

#if !defined(TIMER_H)
#define TIMER_H

#include <stdio.h>
#include <time.h>

// These macros create a timer struct for the function. (There can be
// multiple per function)
#define TIMED_BLOCK_(BlockName, Number) timer TimedBlock_##Number(BlockName)
#define TIMED_BLOCK() TIMED_BLOCK_(__FUNCTION__, __LINE__)

// The timer struct that times the current code block. When the block
// of code goes out of scope and the destructor is called, the
// structure calculates how much time has elapsed.
struct timer
{

    const char *block_name;
    timespec start_time;

    // Constructor - Here we get the initial time that the block has
    // started.
    timer(const char *block)
    {
        block_name = block;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    }

    // Destructor - Here we get the ending time of the block,
    // calculate and finally display the duration.
    ~timer()
    {
        timespec end_time;
        timespec duration;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

        // Get the duration.
        if ((end_time.tv_nsec - start_time.tv_nsec) < 0)
        {
            duration.tv_sec = end_time.tv_sec - start_time.tv_sec - 1;
            duration.tv_nsec = 1000000000 + end_time.tv_nsec - start_time.tv_nsec;
        }
        else
        {
            duration.tv_sec = end_time.tv_sec - start_time.tv_sec;
            duration.tv_nsec = end_time.tv_nsec - start_time.tv_nsec;
        }

        // Convert the time from nanoseconds to milliseconds so it's easier to read.
        double time = ((double)(duration.tv_sec * 1000000000) + duration.tv_nsec) / 1000000;

        printf("[%s] Time: %.2fms\n", block_name, time);

    }

};

#endif
