/*
 * ASSIGNMENT 2: INTERPROCESS COMMUNICATION
 *
 * Multi-threaded Run-Length Encoder using POSIX-threads:
 *
 * The server program executes the following task:
 * Receive multiple requests from the client program using sockets. Therefore, the server program creates a child process per request to
 * handle these requests simultaneously. For this reason, the parent process must handle zombie processes by implementing the fireman()
 * function call (unless you can determine the number of requests the server program receives from the client program). 
 *
 * Each child process executes the following tasks:
 * 1.First, receive the input string from the client program.
 * 2.Next, encode the input string using the RLE algorithm. The child process will generate the RLE string and the frequency array.
 * 3.Finally, return the RLE string and the frequency array to the client program using sockets.
 *
 ***Used professor Rincon's server.cpp file and tweaked it for my program***
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <vector>


//function to handle zombie processes
void fireman(int) 
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*
 * main function which receives multiple requests from the client program using sockets.
 * receives input string from client.cpp and encodes using RLE algo, sending back
 * the RLE string and frequency array to the client program
 **THIS FUNCTION IS MOSTLY CODE FROM PROFESSOR RINCON'S SERVER.CPP**
 */
int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    signal(SIGCHLD, fireman);
    if (argc < 2)
    {
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR on binding";
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0)
        {
            if (newsockfd < 0)
            {
                std::cerr << "ERROR on accept";
                exit(1);
            }

            //reading the input size from client 
            int size;
            n = read(newsockfd, &size, sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }

            //receiving input data from client
            char *buffer = new char[size + 1];
            bzero(buffer, size + 1);
            n = read(newsockfd, buffer, size);
            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }

            /*
             * START OF RUN-LENGTH ENCODING ALGO:
             * 1.uses the input string sent from client, 
             * 2.returns the RLE string vector (in which characters with a frequency > 1 are doubled),
             * and a frequency int vector(which only holds frequencies > 1).
             */
            std::string input = buffer;
            int n = input.length();
            std::vector<char> characters;
            std::vector<int> frequencies;
            char temp = input[0];
            int count = 1;

            for (int i = 1; i < n + 1; i++)
            {
                if (input[i] == temp) //finding character run length by comparing characters until they do not match
                {
                    count++;
                }
                else
                {
                    /*  
                     * pushing the current character into the RLE 'string' vector
                     * and adding the count to the frequency vector when the character
                     * count is greater than 1
                     */
                    if (count > 1)
                    {
                        for (int j = 0; j < 2; j++)
                        {
                            characters.push_back(temp);
                        }
                        frequencies.push_back(count);
                        temp = input[i];//setting the comparison variable to the next character
                    }
                    /*  
                     * when count = 1 the character is pushed into the string once
                     * and the frequency is not added to the frequency vector 
                     */
                    else
                    {
                        characters.push_back(temp);
                        temp = input[i];
                    }
                    temp = input[i]; //resetting temp and count for the next run
                    count = 1;
                }
            }
            //initializing buffer with the size of the character array
            const char *charBuffer = characters.data();
            size_t charSize = characters.size();
            //sending the size of the character array to the client
            n = write(newsockfd, &charSize, sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket 1";
                exit(1);
            }
            //sending the character array data to the client
            n = write(newsockfd, charBuffer, charSize);
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket 2";
                exit(1);
            }
            //sending the frequency array size to the client
            size_t freqSize = frequencies.size();
            n = write(newsockfd, &freqSize, sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket 3";
                exit(1);
            }
            //sending the frequency array data to the client
            n = write(newsockfd, frequencies.data(), freqSize * sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket 4";
                exit(1);
            }

            //closing the new socket and cleaning up dynamically allocated memory
            close(newsockfd);
            delete[] buffer, charBuffer;
            _exit(0);
        }
    }

    //closing socket
    close(sockfd);
    return 0;
}