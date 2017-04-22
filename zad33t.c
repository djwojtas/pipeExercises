#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

//sort -r <- uniq -c <- sort <- cut -d' ' -f1 <- who
//  p1        p2        p3          p4           p5

int main(void) {
  int p12[2];
  int p23[2];
  int p34[2];
  int p45[2];
  
  pipe(p12);
  if(fork()>0) { //p5 (+ p1)
    pipe(p23);
    if(fork()>0) { //p5 (+ p2)
      pipe(p34);
      //p3, p4 i p5 nie będą się komunikować z 1 procesem
      close(p12[0]);
      close(p12[1]);
      if(fork()>0) { //p5 (+ p3)
        pipe(p45);
        //p4 i p5 nie będą się komunikować z 2 procesem
        close(p23[0]);
        close(p23[1]);
        if(fork()>0) { //p5 (+ p4)
          //nie korzystamy z stdin do procesu 4
          close(p45[0]);
        
          //wysyłamy stdout do procesu 4
          dup2(p45[1], 1);
          close(p45[1]);

          execlp("who", "who", (char*) NULL);
          perror(NULL);
        } else { //p4
          //nie korzystamy z stdout do procesu 5
          close(p45[1]);
          //nie korzystamy z stdin do procesu 3
          close(p34[0]);

          //otrzymujemy stdin od procesu 5
          dup2(p45[0], 0);
          close(p45[0]);
          
          //wysyłamy stdout do procesu 3
          dup2(p34[1], 1);
          close(p34[1]);
          
          execlp("cut", "cut", "-d", " ", "-f1", (char*) NULL);
          perror(NULL);
        }
      } else { //p3
        //nie korzystamy z stdout do procesu 4
        close(p34[1]);
        //nie korzystamy z stdin do procesu 2
        close(p23[0]);
      
        //otrzymujemy stdin od procesu 4
        dup2(p34[0], 0);
        close(p34[0]);
        
        //wysyłamy stdout do procesu 2
        dup2(p23[1], 1);
        close(p23[1]);
        
        execlp("sort", "sort", (char*) NULL);
        perror(NULL);
      }
    } else { //p2
      //nie korzystamy z stdout do procesu 3
      close(p23[1]);
      //nie korzystamy z stdout do procesu 3
      close(p12[0]);
    
      //otrzymujemy stdin od procesu 3
      dup2(p23[0], 0);
      close(p23[0]);
      
      //wysyłamy stdout do procesu 1
      dup2(p12[1], 1);
      close(p12[1]);
      
      execlp("uniq", "uniq", "-c", (char*) NULL);
      perror(NULL);
    }
  } else { //p1
    //nie korzystamy z stdout do procesu 2
    close(p12[1]);
    
    //otrzymujemy stdin od procesu 3
    dup2(p12[0], 0);
    close(p12[0]);
  
    execlp("sort", "sort", "-r", (char*) NULL);
    perror(NULL);
  }
  return 0;
}
