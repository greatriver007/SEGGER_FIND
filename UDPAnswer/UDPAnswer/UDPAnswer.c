/*********************************************************************
----------------------------------------------------------------------
File    : UDPAnswer.c
--------- END-OF-HEADER --------------------------------------------*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>

/*********************************************************************
*
*       PUSH_IPADDR macro
*
**********************************************************************
  Purpose:
    Macro for passing IP addresses to my_printf octet at a time. usage:
    printf("IP is %u.%u.%u.%u\n", PUSH_IPADDR(ip)); Since we store IP
    addresses in net endian, it's endian sensitive.
*/
#define  PUSH_IPADDR(ip) (unsigned)(ip&0xff), (unsigned)((ip>>8)&0xff), (unsigned)((ip>>16)&0xff), (unsigned)(ip>>24)

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define TARGET_NAME                     "MyTarget"              // Target Name. Mandatory, usually a fixed string
#define TARGET_SERIAL_NUMBER            12345678                // Serial number. Mandatory, the define is a number, the value transmitted
                                                                // is an ASCII string. Typically, a function call will be used to get the
                                                                // serial number.
#define FIND_RESPONSE_BUFFER_SIZE       128                     // Maximum FIND packet size is 512 bytes.
                                                                // It usually makes sense to limit this since the work buffer is located
                                                                // on the stack, and a bigger buffer would increase stack requirements.
#define FIND_DISCOVER_BUFFER_SIZE       128                     // Maximum FIND packet size is 512 bytes.
                                                                // It usually makes sense to limit this since the work buffer is located
                                                                // on the stack, and a bigger buffer would increase stack requirements.
#define FIND_SEND_ADDITIONAL_INFO 		"FIND TEST"		            // Additional Info

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define FIND_RECEIVE_IDENTIFIER         "FINDReq=1;"
#define FIND_RECEIVE_IDENTIFIER_LEN     11                      // FIND_RECEIVE_IDENTIFIER length including the zero-termination
#define FIND_SEND_IDENTIFIER            "FIND=1;"
#define FIND_DEF_PORT                   50022

/*********************************************************************
*
*       Static Variables
*
**********************************************************************
*/
unsigned int BroadcastIP = 0xFFFFFFFF;                         // IP address, broadcast

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_GetIPAddr()
*/
unsigned long  IP_GetIPAddr (unsigned char IFace)
{
  (void) IFace;
  return 0xFFFFFFFE;
}

/*********************************************************************
*
*       IP_GetHWAddr()
*/
void IP_GetHWAddr (unsigned char IFace, unsigned char * pDest, unsigned Len)
{
  (void) IFace;
  (void) Len;
  unsigned char mac[6] = {0x00,0x07,0xED,0xAA,0xBB,0xCC};
  memcpy(pDest, mac, sizeof(mac));
}

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
  int Pos;
  int SerialNumber;
  unsigned long IPAddr;
  unsigned char aMacData[6];
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
  //
  // Loop until we get a socket
  //
  do {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != SOCKET_ERROR) {
      //
      // Create datagram socket and enable sending of broadcasts over this socket
      //
      {
        char Opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &Opt, sizeof(Opt));
      }
      {
        unsigned long Opt = 1;
        ioctlsocket(sock, FIONBIO, &Opt);
      }
      //
      // Bind socket to UDP port
      //
      memset(&LAddr, 0, sizeof(LAddr));
      LAddr.sin_family      = AF_INET;
      LAddr.sin_port        = htons(FIND_DEF_PORT);
      LAddr.sin_addr.s_addr = INADDR_ANY;
      bind(sock, (struct sockaddr *)&LAddr, sizeof(LAddr));
      break;
    }
    Sleep(100);    // Try again
  } while (1);

  //
  // Wait for incoming UDP discover packets
  //
  TargetAddr.sin_family = AF_INET;
  TargetAddr.sin_port = htons(FIND_DEF_PORT);
  TargetAddr.sin_addr.s_addr = htonl(BroadcastIP);
  printf("Waiting for FIND queries on port %d\n",FIND_DEF_PORT);
  while (1) {
    memset(aFINDDiscover, 0, FIND_DISCOVER_BUFFER_SIZE);
    Len = sizeof(TargetAddr);
    Len = recvfrom(sock, &aFINDDiscover[0], FIND_DISCOVER_BUFFER_SIZE, 0, (struct sockaddr*)&TargetAddr, &Len);
    if (Len > 0) {
      if (memcmp(aFINDDiscover, FIND_RECEIVE_IDENTIFIER, FIND_RECEIVE_IDENTIFIER_LEN) == 0) {
        //
        // Build response, Fill packet with data, containing IPAddr, HWAddr, S/N and name.
        //
        memset(aFINDResponse, 0, FIND_RESPONSE_BUFFER_SIZE);                                  // Make sure all fields are cleared.
        Pos = snprintf(aFINDResponse, FIND_RESPONSE_BUFFER_SIZE, "%s", FIND_SEND_IDENTIFIER); // Answer string for a discover response.
        //
        // Add IPv4 information.
        //
        IPAddr = IP_GetIPAddr(0);
        IPAddr = htonl(IPAddr);
        Pos   += snprintf(&aFINDResponse[Pos], FIND_RESPONSE_BUFFER_SIZE - Pos, "IP=%03u.%03u.%03u.%03u;", PUSH_IPADDR(IPAddr));
        //
        // Add MAC information.
        //
        IP_GetHWAddr(0, aMacData, 6);
        Pos += snprintf(&aFINDResponse[Pos], FIND_RESPONSE_BUFFER_SIZE - Pos, "HWADDR=%02X:%02X:%02X:%02X:%02X:%02X;", aMacData[0], aMacData[1], aMacData[2], aMacData[3], aMacData[4], aMacData[5]);
        //
        // Add device name.
        //
        Pos += snprintf(&aFINDResponse[Pos], FIND_RESPONSE_BUFFER_SIZE - Pos, "DeviceName=%s;", TARGET_NAME);
        //
        // Add serial number.
        //
        SerialNumber = TARGET_SERIAL_NUMBER;
        Pos += snprintf(&aFINDResponse[Pos], FIND_RESPONSE_BUFFER_SIZE - Pos, "SN=%08d;", SerialNumber);
        //
        // Add Additional Info.
        //
        Pos += snprintf(&aFINDResponse[Pos], FIND_RESPONSE_BUFFER_SIZE - Pos, "%s", FIND_SEND_ADDITIONAL_INFO);
        //
        // Allocate and send response packet.
        //
        sendto(sock, aFINDResponse, Pos + 1, 0, (struct sockaddr*)&TargetAddr, sizeof(TargetAddr));
        //
        // Output reply infos
        //
        printf("Response (%s) sent.\n", &aFINDResponse);
      }
    }
  }
  closesocket(sock);
  WSACleanup();
  return 0;
}

/*************************** end of file ****************************/
