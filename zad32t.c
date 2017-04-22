#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096
#define NUMBUFSIZE 8

// P0 printf <--- P2 *5 <---- P1 seq

int main(void) {
  char buf[BUFSIZE];
  
  int p12[2];
  int p20[2];
  
  int readbytes;

  pipe(p12);
  
  if(fork() > 0) { //p0
    pipe(p20);
    
    if(fork() > 0) { //p0
      //p0 ma się nie komunikować z p1 i tylko odbierać od p2
      close(p12[0]);
      close(p12[1]);
      close(p20[1]);
      
      //odbieramy dane od p2
      while((readbytes = read(p20[0], buf, sizeof(buf))) != 0) {
        buf[readbytes] = '\0';
        printf("%s\n", buf);
      }
      
    } else { //p2 ma pomnożyć stdin i wysłać dalej
      //będziemy tylko odbierać od p1 i wysyłać do p0
      close(p12[1]);
      close(p20[0]);
      
      //kierujemy stdout do pipa łaczącego z p0
      close(1);
      dup(p20[1]);
      
      //seq 10 nie zajmie całego bufora (w każdym razie mam taką nadzieję w ramach tego programu)
      readbytes = read(p12[0], buf, sizeof(buf));
      buf[readbytes] = '\0';
      
      //wykrywanie liczb, mnożenie i wypisywanie na przekierowany stdout
      int i;
      int number = 0;
      int numdetected = 0;
      for(i=0; buf[i] != '\0'; ++i) {
        for(; buf[i] >= '0' && buf[i] <= '9'; ++i) {
          number *= 10;
          number += buf[i] - '0';
          numdetected = 1;
        }
        if(numdetected) {
          number *= 5;
          printf("%d\n", number);
          number = 0;
          numdetected = 0;
        }
      }

      close(p20[1]);
    }
  } else { //p1 - ma wysłać seq 10
    //p1 nie potrzebuje wyjścia pipa
    close(p12[0]);
    //przekierowanie stdout na wejście do pipa
    close(1);
    dup(p12[1]);
    
    execlp("seq", "seq", "10", (char*) NULL);
    perror(NULL);
  }

  return 0;
}
