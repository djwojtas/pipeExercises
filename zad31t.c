#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096

int main(void) {
  char buf[BUFSIZE];
  int pfd1i[2];
  int pfd1o[2];
  int pfd2i[2];
  int pfd2o[2];

  int fd;
  int readbytes;
    
  pipe(pfd1i);
  pipe(pfd1o);
  
  fd = open("dictionary.txt", O_RDONLY);
  if(fd == -1) { 
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  
  if(fork() > 0) { //proces matka
    pipe(pfd2i);
    pipe(pfd2o);
    
    if(fork() > 0) { //proces matka
      //zamknięcie niepotrzebnych wyjść pipa w procesie-matce
      close(pfd1i[0]);
      close(pfd2i[0]);
      //wysyłanie danych
      while((readbytes = read(fd, buf, sizeof(buf))) == sizeof(buf)) {
        write(pfd1i[1], buf, readbytes);
        write(pfd2i[1], buf, readbytes);
      }
      //zamknięcie wejść pipa aby wysłać EOF do dzieci
      close(pfd1i[1]);
      close(pfd2i[1]);
  
      //zamknięcie niepotrzebnego wejścia do pipa - będziemy z niego tylko czytać
      close(pfd1o[1]);
      //śmiałe założenie, że dziecko nie zwróci więcej niż sizeof(buf) danych
      readbytes = read(pfd1o[0], buf, sizeof(buf));
      buf[readbytes] = '\0'; 
      printf("Ile linii : %s", buf);
    
      //analogicznie do wyższego
      close(pfd2o[1]);
      readbytes = read(pfd2o[0], buf, sizeof(buf));
      buf[readbytes] = '\0'; 
      printf("Ilosc linii z pipe: %s", buf);    
    } else { //proces dziecko - posiada wszystkie pipy
      //usuwanie odziedziczonego przez fork połączenia z wc aby nie zblokować procesu
      close(pfd1i[1]);
    
      //przekierowanie stdout na wejście do pipa
      close(1);
      dup(pfd2o[1]);
      
      //przekierowanie stdin na wyjście pipa
      close(0);
      dup(pfd2i[0]);
      
      //zamknięcie niepotrzebnych wyjść pipa w tym procesie
      close(pfd2o[0]);
      close(pfd2i[1]);
      
      execlp("grep", "grep" , "-c", "pipe", (char*) NULL);
      perror(NULL);
    }
  } else { //proces dziecko - posiada tylko swoje pipy
    //przekierowanie stdout na wejście do pipa
    close(1);
    dup(pfd1o[1]);
    
    //przekierowanie stdin na wyjście pipa
    close(0);
    dup(pfd1i[0]);
    
    //zamknięcie niepotrzebnych wyjść pipa w tym procesie
    close(pfd1o[0]);
    close(pfd1i[1]);
    
    execlp("wc", "wc", "-l",(char*) NULL);
    perror(NULL);
  }   
  
  return 0;
}
