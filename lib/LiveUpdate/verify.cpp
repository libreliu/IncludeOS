/**
 * Master thesis
 * by Alf-Andre Walla 2016-2017
 * 
**/
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <util/crc32.hpp>

int main(void)
{
  uint32_t x = CRC32_BEGIN();
  for (int i = 0; i < 1000; i++) {
    int len = 4096;
    auto* buf = new char[len];
    memset(buf, 'A', len);
    x = crc32(x, buf, len);
    delete[] buf;
  }
  const uint32_t FINAL_CRC = CRC32_VALUE(x);
  printf("CRC32 should be: %08x\n", FINAL_CRC);
    
    const uint16_t PORT = 6667;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "10.0.0.42", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(PORT);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");
    
    uint32_t crc = 0xFFFFFFFF;
    while (true)
    {
      char buffer[4096];
      int n = read(sockfd, buffer, sizeof(buffer));
      if (n < 0) perror("ERROR reading from socket");
      
      if (n == 0) break;
      
      // update CRC32 partially
      crc = crc32(crc, buffer, n);
      
      if (~crc == FINAL_CRC)
        break;
      else
        printf("Partial CRC: %08x\n", ~crc);
    }
    crc = ~crc;
    
    printf("Final CRC: %08x  vs  %08x\n", crc, FINAL_CRC);
    close(sockfd);
    
    assert(crc == FINAL_CRC);
    return 0;
}
