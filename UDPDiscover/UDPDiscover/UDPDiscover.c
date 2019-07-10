/*********************************************************************
----------------------------------------------------------------------
File    : UDP_Discover.c
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define FIND_RESPONSE_BUFFER_SIZE       128                     // Maximum FIND packet size is 512 bytes.
                                                                // It usually makes sense to limit this since the work buffer is located
                                                                // on the stack, and a bigger buffer would increase stack requirements.
#define FIND_DISCOVER_BUFFER_SIZE       128                     // Maximum FIND packet size is 512 bytes.
                                                                // It usually makes sense to limit this since the work buffer is located
                                                                // on the stack, and a bigger buffer would increase stack requirements.
/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define FIND_RECEIVE_IDENTIFIER         "FINDReq=1;"
#define FIND_RECEIVE_IDENTIFIER_LEN     11                      // FIND_RECEIVE_IDENTIFIER length including the zero-termination
#define FIND_SEND_IDENTIFIER            "FIND=1;"
#define FIND_SEND_IDENTIFIER_LEN        7                       // FIND_SEND_IDENTIFIER length including the zero-termination
#define FIND_DEF_PORT                   50022

/*********************************************************************
*
*       Static Variables
*
**********************************************************************
*/
unsigned int BroadcastIP = 0xFFFFFFFF;                          // IP address, broadcast

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       main()
*/
int main(int argc, char* argv[], char* envp[]) {
  WORD wVersionRequested;
  WSADATA wsaData;
  SOCKET sock;
  SOCKADDR_IN LAddr;
  SOCKADDR_IN TargetAddr;

  int Len;
  int t, t0;
  int NumBytes;
  unsigned char aFINDResponse[FIND_RESPONSE_BUFFER_SIZE];
  unsigned char aFINDDiscover[FIND_DISCOVER_BUFFER_SIZE];

  //
  // Initialize winsock. Required to use TCP/IP
  //
  wVersionRequested = MAKEWORD(2, 0);
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    printf("Could not init WinSock.\n");
    return 1;
  }
  timeBeginPeriod(1);
  //
  // Create datagram socket and enable sending of broadcasts over this socket
  //
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == SOCKET_ERROR) {
    printf ("Could not create socket.");
  }
  {
    char Opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &Opt, sizeof(Opt));
  }
  {
    unsigned long Opt = 1;
    ioctlsocket(sock, FIONBIO, &Opt);
  }
  memset(&LAddr, 0, sizeof(LAddr));
  LAddr.sin_family      = AF_INET;
  LAddr.sin_port        = htons(FIND_DEF_PORT);
  LAddr.sin_addr.s_addr = INADDR_ANY;
  bind(sock, (struct sockaddr *)&LAddr, sizeof(LAddr));
  while (1) {
    memset(aFINDDiscover, 0, FIND_DISCOVER_BUFFER_SIZE);
    strcpy(aFINDDiscover, FIND_RECEIVE_IDENTIFIER);
    TargetAddr.sin_family = AF_INET;
    TargetAddr.sin_port = htons(FIND_DEF_PORT);
    TargetAddr.sin_addr.s_addr = htonl(BroadcastIP);
    NumBytes = sendto(sock, &aFINDDiscover[0], FIND_RECEIVE_IDENTIFIER_LEN, 0, (SOCKADDR*)&TargetAddr, sizeof(TargetAddr));
    if (NumBytes == SOCKET_ERROR) {
      printf("sendto error\n");
      break;
    }
    printf("Sending discover - Waiting for replies...\n");
    t0 = timeGetTime();
    do {
      memset(aFINDResponse, 0, FIND_RESPONSE_BUFFER_SIZE);
      Len = sizeof(TargetAddr);
      NumBytes = recvfrom(sock, &aFINDResponse[0], FIND_RESPONSE_BUFFER_SIZE, 0, (SOCKADDR*)&TargetAddr, &Len);
      if (NumBytes > 0) {
        if (memcmp(aFINDResponse, FIND_SEND_IDENTIFIER, FIND_SEND_IDENTIFIER_LEN) == 0) {
          //
          // Output reply infos
          //
          printf("%dms: %s\n", timeGetTime() - t0, aFINDResponse);
        }
      }
      t = timeGetTime();
    } while ((t0 + 100 - t) > 0);
    Sleep(1000);
  }
  closesocket(sock);
  WSACleanup();
  timeEndPeriod(1);
  return 0;
}

/*************************** end of file ****************************/
