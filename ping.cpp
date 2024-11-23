#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <icmpapi.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
/*

Owen Fazzini
CS-374

Contains working implementation of ping command

*/

void ping(const char* addr, int bytes, int numpings, int timeout){
/*
PING
Recreation of the classic ping shell command.
Accepts the following parameters:
    - const char* addr: string representation of an IP address (IPv4 only)
    - bytes: size of the message to send
    - numpings: amount of times to ping target IP
    - timeout: time between pings in milliseconds. Should probably be set to 1000.
*/

/*

wsastartup initializes winsock functions; winsock expects that it will be called first.
it accepts two parameters:
    a word which represents the winsock version being used -- here i'm using 2.2, so 2 in both the low and high bytes --
    and a pointer to a WSADATA structure which contains information about the implementation.

*/

WSADATA wsadata;
int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if(iResult != 0){
        perror("WSAStartup failed.\n");
        exit(1);
    }

// creates icmp handle for sending echo requests
HANDLE hIcmpFile = IcmpCreateFile();
    if(hIcmpFile == INVALID_HANDLE_VALUE)
    {
        printf("Unable to open handle.\n");
        printf("Error: ", GetLastError());
    }

// converts const char* from input into an IN_ADDR structure -- representation of an IPv4 address
unsigned long destaddr = inet_addr(addr);

// for packet loss + calculations later
int numLost = 0;
vector<unsigned long> timearr;


printf("Pinging %s with %d bytes of data...\n", addr, bytes);

// performs the ping
for(int i = 0; i < numpings; i++){
// create buffers
char req_buff[bytes] = "test";
char* req_buff_ptr = req_buff;

DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(req_buff);
void* reply_buff = (void*)malloc(reply_size);

// echo request
// IcmpSendEcho returns the number of echo replies it receives from the destination address.
DWORD send_echo = IcmpSendEcho(hIcmpFile, destaddr, req_buff_ptr, bytes, NULL, reply_buff, 1024, 500);
    if(send_echo == 0)
    {
        printf("No reply received.\n");
    }

// creates a pointer (PICMP_ECHO_REPLY) to the ICMP_ECHO_REPLY stored inside the reply buffer
// will use this to access information about the echo reply
PICMP_ECHO_REPLY pecho = (PICMP_ECHO_REPLY)reply_buff;

char* testaddr = inet_ntoa(*(struct in_addr*)&pecho->Address);

// if the transmission was not successful, then the packet was lost.
    if(pecho->Status != 0)
    {
        numLost++;
    }

// adds round trip time to array for calculation later
unsigned long tempo = pecho->RoundTripTime;
timearr.push_back(tempo);

// now, print information
if(pecho->Status == 0){
    cout << "Reply from " << testaddr << ": " << "time = " << to_string(pecho->RoundTripTime) << "\n";
    Sleep(timeout);
}
else{
    Sleep(timeout);
}
}

// calculates minimum, maximum, and average times
int maxtime = *std::max_element(timearr.begin(), timearr.end());
int mintime = *std::min_element(timearr.begin(), timearr.end());
float avgtime = accumulate(timearr.begin(), timearr.end(), 0.0)/timearr.size();

// prints statistics like the Windows command line
printf("Ping statistics for %s:\n", addr);
printf("    Packets: Sent = %d, Received = %d, Lost = %d (%d%% Loss)\n", numpings, (numpings-numLost), numLost, (100 * numLost / numpings));
printf("Approximate packet round-trip times in milliseconds:\n");
printf("    Minimum = %d, Maximum = %d, Average = %f\n", mintime, maxtime, avgtime);


IcmpCloseHandle(hIcmpFile);
WSACleanup();

}


int main(){
    ping("8.8.8.8", 32, 8, 1000);
    return 0;
}
