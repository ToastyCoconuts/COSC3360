/*Looked at GeeksforGeeks and Wikipedia for some reference on the algoritm, 
but didn't directly copy. Also used notes, and slides from class for reference on
the threading portion.*/

#include <iostream>
#include <pthread.h>
#include <vector>

//structure to easily store and access characters and frequencies
struct data
{
    std::string input;
    std::vector<char>characters;
    std::vector<int>frequencies;
};

//RLE Thread function
void *RLEFunc(void *RLE)
{
    data* output = (data*)RLE;

    //creating a temp variable to hold the current character for comparison
    char temp = output->input[0];
    int count = 1;

    for(int i = 1; i < output->input.length() + 1; i++)
    {
        //finding character run length by comparing characters until they do not match
        if(output->input[i] == temp)
        {
            count++;
        }
        else
        {
            if(count > 1)
            {   /*pushing the current character into the rle 'string' vector
                and adding the count to the frequency vector when the character
                count is greater than 1*/
                for(int j = 0; j < 2; j++)
                {
                    output->characters.push_back(temp);
                }
                output->frequencies.push_back(count);
                //setting the comparison variable to the next character
                temp = output->input[i];
            }
            /*when count = 1 the character is pushed into the string once
            and the frequency is not added to the frequency vector */
            else
            {
                output->characters.push_back(temp);
                temp = output->input[i];
            }
            //resetting temp and count for the next run
            temp = output->input[i];
            count = 1;
        }
    }
    

    return NULL;
}


int main()
{ 
    std::string inputs;
    std::vector<std::string> strings;
    
    while(std::getline(std::cin, inputs) && inputs.size() != 0)
    {
        strings.push_back(inputs);
    }

    //using input length to declare struct
    data* info = new data[strings.size()];
    
    //loop to push the string into the struct member 'input'
    /*for(int i = 0; i < input.length(); i++)
    {
        info[i].input = input;
    }
        */           
    pthread_t ptid[strings.size()];
    //creating threads
    for(int i = 0; i < strings.size(); i ++)
    {   
        info[i].input = strings[i];
        if(pthread_create(&ptid[i], nullptr, RLEFunc, &info[i]))
        {
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
    }
    //waiting for threads to finish
    for(int i = 0; i < strings.size(); i++)
    { 
        pthread_join(ptid[i], nullptr);
    }
    
    for(int j = 0; j < strings.size(); j++)
    {        
        std::cout << "Input string: ";
        for(int i = 0; i < info[j].input.length(); i++)
        {
            std::cout << info[j].input[i];
        }

        std::cout << "\nRLE String: ";
        for(int i = 0; i < info[j].characters.size(); i++)
        {
            std::cout << info[j].characters[i];
        }

        std::cout << "\nRLE Frequencies: ";
        for(int i = 0; i < info[j].frequencies.size(); i++)
        {
            std::cout << info[j].frequencies[i] << " ";
        }
        std::cout << std::endl << std::endl;
    }

    return 0;
}