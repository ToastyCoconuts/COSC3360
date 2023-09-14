/*
 *ASSIGNMENT 2: INTERPROCESS COMMUNICATION
 *
 *Multi-threaded Run-Length Encoder using POSIX-threads:
 *
 *After reading the information from STDIN, 
 *this program creates m child threads (where m is the number of strings in the input files). 
 *Each child thread executes the following tasks: 
 *1. Receives the information about the input string from the main thread.
 *2.Create a socket to communicate with the server program.
 *3.Send the input string to the server program using sockets. 
 *4.Wait for the RLE string and the frequency array from the server program.
 *5.Write the received information into a memory location accessible by the main thread.
 *
 ***Used professor Rincon's client.cpp file and tweaked it for my program***
 */


#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

#include <vector>
#include <pthread.h>

/*
 *  structure declaration which stores each input string, compressed RLE strings,
 *  and frequency integers for easier access in both the thread and main functions.
 */
struct data
{
    std::string input;
    std::vector<char> characters;
    std::vector<int> frequencies;
    std::string result;
    char *server;
    char *portno;
};

/*
 *  thread function that receives the input string from STDIN, creates a socket to send
 *  the input to the server program to perform Run-Length Encoding(RLE). Once that is done
 *  the function will receive and write the character and frequency array to memory accessible
 *  to the main function.
 * **THIS FUNCTION IS MOSTLY CODE FROM PROFESSOR RINCON'S CLIENT.CPP**
 */
void *sendToServer(void *RLE)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct data *output = (struct data *)RLE;
    portno = std::atoi(output->portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket" << std::endl;
    }

    server = gethostbyname(output->server);
    if (server == NULL)
    {
        std::cerr << "ERROR, no such host\n";
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //connecting to the server
    {
        std::cerr << "ERROR connecting\n";
        exit(1);
    }

    //sending input size to the server
    std::string s = output->input; 
    int strSize = s.length();
    n = write(sockfd, &strSize, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket\n";
        exit(1);
    }
    //sending the input data to the server
    char input1[strSize];
    strcpy(input1, s.c_str());
    n = write(sockfd, input1, strSize);
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket \n";
        exit(1);
    }

    //receving the size of the char array from the server
    int size;
    n = read(sockfd, &size, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket in client\n";
        exit(1);
    }

    //initializing buffer to receive and store the char array data from the server
    char *buffer = new char[size + 1];
    bzero(buffer, size + 1);
    n = read(sockfd, buffer, size);
    std::string str(buffer);
    for (char st : str)
    {
        output->characters.push_back(st);
    }

    //receiving the size of the frequency array from the server
    int size2;
    n = read(sockfd, &size2, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket in client\n";
        exit(1);
    }

    //initializing another buffer to receive and store the frequency data from the server
    int *buffer2 = new int[size2];
    n = read(sockfd, buffer2, sizeof(int) * size2);
    for (int i = 0; i < size2; i++)
    {
        output->frequencies.push_back(buffer2[i]);
    }

    //closing the socket and cleaning up dynamically allocated memory
    close(sockfd);
    delete[] buffer;
    delete[] buffer2;
    return NULL;
}

int main(int argc, char *argv[])
{
    std::string inputs;
    std::vector<std::string> strings;

    while (std::getline(std::cin, inputs) && inputs.size() != 0)//loop to push inputs into the strings vector while input line is not empty
    {
        strings.push_back(inputs);
    }

    data *info = new data[strings.size()];//using strings vector size to declare info size

    if (argc < 3) 
    {
        std::cerr << "usage " << argv[0] << " hostname port\n";
        exit(0);
    }

    pthread_t ptid[strings.size()];//initalizing thread id size with the size of the strings vector
    for (int i = 0; i < strings.size(); i++)//creating threads
    {
        info[i].server = argv[1];//assign data member server with the value from argv[1]
        info[i].portno = argv[2];//assign data member portno with the value from argv[2]
        info[i].input = strings[i];//assign data member input (string) with the value in strings[i]
        if (pthread_create(&ptid[i], nullptr, sendToServer, &info[i]))
        {
            std::cout << "Error creating thread\n";
            return 1;
        }
    }

    for (int i = 0; i < strings.size(); i++)//waiting for threads to finish
    {
        pthread_join(ptid[i], nullptr);
    }

    for (int j = 0; j < strings.size(); j++)//for loop to print the expected output for each string in vector strings
    {
        std::cout << "Input string: ";
        for (int i = 0; i < info[j].input.length(); i++)
        {
            std::cout << info[j].input[i];
        }

        std::cout << "\nRLE String: ";
        for (int i = 0; i < info[j].characters.size(); i++)
        {
            std::cout << info[j].characters[i];
        }

        std::cout << "\nRLE Frequencies: ";
        for (int i = 0; i < info[j].frequencies.size(); i++)
        {
            std::cout << info[j].frequencies[i] << " ";
        }
        std::cout << std::endl
                  << std::endl;
    }

    delete[] info; // deallocating memory used for struct
    return 0;
}