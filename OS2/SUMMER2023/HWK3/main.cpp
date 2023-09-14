/*
 *ASSIGNMENT 3: INTERPROCESS SYNCHRONIZATION
 *
 *Multi-threaded Run-Length Encoder using POSIX-threads:
 *
 *After reading the information from STDIN,
 *this program creates n child threads (where n is the number of strings in the input files).
 *Each child thread executes the following tasks:
 *1. Receives the information about the input string from the main thread.
 *2. Uses the Run-length encoding algorithm to determine the RLE string and the frequency array.
 *3. Prints the information about the assigned input string using the output messages
 *
 ***Used GeeksForGeeks as reference for the RLE algo on my Assignment 1***
 ***Also referenced the reverse code from teams/class***
 */

#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>

/*
 *  structure declaration which stores each input string, compressed RLE strings,
 *  and frequency integers for easier access in both the thread and main functions.
 *  also initializing pthread_mutex and pthread_cond as well as variables for the threadID and turn
 */
struct data
{
    std::string input;
    std::vector<char> characters;
    std::vector<int> frequencies;

    pthread_mutex_t *bsem;
    pthread_mutex_t *bsem2;
    pthread_cond_t *turn;
    int tid, sigTurn;
};

// RLE Thread function
void *RLEFunc(void *RLE)
{
    // casting RLE pointer to data struct pointer
    data *output = (struct data *)RLE;

    // storing struct information to local variables
    std::string input = output->input;
    std::vector<char> charVector = output->characters;
    std::vector<int> freqVector = output->frequencies;
    int id = output->tid;

    pthread_mutex_unlock(output->bsem);
    // CRITICAL SECTION END(started in parent thread in main)
    // creating a temp variable to hold the current character for comparison and a count variable for that character
    char temp = input[0];
    int count = 1;

    // Run-Length Encoding algorithm start
    for (int i = 1; i < input.length() + 1; i++)
    {
        // finding character run length by comparing characters until they do not match
        if (input[i] == temp)
        {
            count++;
        }
        else
        {
            if (count > 1)
            { /*pushing the current character into the rle 'string' vector
              and adding the count to the frequency vector when the character
              count is greater than 1*/
                for (int j = 0; j < 2; j++)
                {
                    charVector.push_back(temp);
                }
                freqVector.push_back(count);
                // setting the comparison variable to the next character
                temp = input[i];
            }
            /*when count = 1 the character is pushed into the string once
            and the frequency is not added to the frequency vector */
            else
            {
                charVector.push_back(temp);
                temp = input[i];
            }
            // resetting temp and count for the next run
            temp = input[i];
            count = 1;
        }
    }
    // CRITICAL SECTION START
    pthread_mutex_lock(output->bsem2);
    // synchronizing threads using the id and turn variables
    while (output->sigTurn != id)
    {
        pthread_cond_wait(output->turn, output->bsem2);
    }

    pthread_mutex_unlock(output->bsem2);
    // CRITICAL SECTION END

    // printing output outside of critical sections
    std::cout << "Input string: " << input << std::endl;
    std::cout << "RLE String: ";
    for (int i = 0; i < charVector.size(); i++)
    {
        std::cout << charVector[i];
    }
    std::cout << std::endl;
    std::cout << "RLE Frequencies: ";
    for (int i = 0; i < freqVector.size(); i++)
    {
        std::cout << freqVector[i] << " ";
    }
    std::cout << std::endl << std::endl;

    // CRITICAL SECTION START
    pthread_mutex_lock(output->bsem2);
    output->sigTurn = output->sigTurn + 1;
    // unblocking waiting threads prior to unlock
    pthread_cond_broadcast(output->turn);
    pthread_mutex_unlock(output->bsem2);
    // CRITICAL SECTION END

    return nullptr;
}

int main()
{
    std::string inputs;
    std::vector<std::string> strings;

    // receiving input from STDIN until blank line entered
    while (std::getline(std::cin, inputs) && inputs.size() != 0)
    {
        strings.push_back(inputs);
    }

    // NTHREADS is the # of POSIX threads determined by the # of input strings
    const int NTHREADS = strings.size();
    pthread_t ptid[NTHREADS];

    // initializing mutex bsem and bsem2 with default attributes
    static pthread_mutex_t bsem;
    static pthread_mutex_t bsem2;
    pthread_mutex_init(&bsem, nullptr);
    pthread_mutex_init(&bsem2, nullptr);

    // initializing turn condition for thread synchronization
    static pthread_cond_t turn = PTHREAD_COND_INITIALIZER;

    // initializing a data struct object info(singular **not multiple memory addresses like assignment 1)
    data info;
    info.bsem = &bsem;
    info.bsem2 = &bsem2;
    info.turn = &turn;
    info.sigTurn = 0;

    // creating parent thread
    for (int i = 0; i < NTHREADS; i++)
    {   
        // CRITICAL SECTION START(modifying info attributes with current input string and thread ID)
        pthread_mutex_lock(&bsem);
        info.input = strings[i];
        info.tid = i;
        // creating child thread for each input string
        if (pthread_create(&ptid[i], nullptr, RLEFunc, &info))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }
    // waiting for threads to finish
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(ptid[i], nullptr);
    }

    return 0;
}