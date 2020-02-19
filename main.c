#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

// remove subsetsum timers and test with large inputs to see if execution fails,
// check output fails for correctedness. and checking if above and belove output correctly

// function calls
bool isSubsetSum(int set[], int n, int sum, int subset[], int count);

// time termination prototypes, code from unix book, print astrisk every 2 seconds
static void myhandler(int s);
static int setupinterrupt(void); /* set up myhandler for SIGPROF */
static int setupitimer(int time); /* set ITIMER_PROF for 2-second intervals */
void sigint_handler(int sig_num);

//signal handler for alarm for -t in getopt
void sigAlarmHandler(int sig_num);
void sigChildAlarm(int sig_num);


//counter and timer for child
  pid_t child_pid;
  clock_t before;
  int msec = 0, trigger = 1000;
  int iterations = 0;
  int quitFlag = 0;
  int childQuitFlag = 0;
  int temp;

  // for printing solution to subsetsum
  int globalCount = 0;
  int store[48];


int main(int argc, char *argv[]){

  pid_t pid;
  // file operation variables
  FILE *file = NULL;
  FILE *ofp = NULL;
  char *ifname = NULL;
  char *ofname = NULL;
  int i;
  int iflag = 0, oflag = 0;

  int time = 60; //set by getopt for program termination
  int done; //flag to help determine program termination
  // variable for first int inside formatted dat file
  int childLoops = 0;
  int *intBuff = malloc(sizeof(int) * 64);

  // get opt loop var
  int opt;

  // default file names
  ifname = "input.dat";
  ofname = "output.dat";

  while((opt = getopt(argc, argv, "hi:o:t:")) != -1){
    switch(opt){
      case 'h':
        printf("this program is designed to take input file of specific format and runs subsetsum algo. on input lines\n");
        printf("Usage: logParse [-h] [-i][dirname ] [-o][dirname] [-t n]\n");
        printf("-h,   --help                    Print a help message and exit\n");
        printf("-i,   --input file              Include input file name\n");
        printf("-o,   --ouput file              Include output file name\n");
        printf("-t n  --timer                   user set timer for program termination using integer n representing seconds\n");
        printf("input file is format is:\n");
        printf("n               were n is a single integer represnting the number of lines that will be processed as children\n");
        printf("x a b c d       each line is then proccsed by forking a child to solve subsetsum problem with x being the sum\n");
        printf("default settings: -i input.dat -o output.dat -t 60\n");
        exit(0);
      case 'i':
        ifname = optarg;
        iflag = 1;
        //printf("option i:  %c\t'%s'\t'%s'\n", opt, optarg);
        break;
      case 'o':
        ofname = optarg;
        oflag = 1;
        //printf("option o: %c\t'%s'\n", opt, optarg);
        break;
      case 't':
        time = atoi(optarg);
        break;
      case ':':
        printf("option needs a value\n");
        break;
      case '?':
        printf("unknown option: %c\n", optopt);
        exit(1);
        break;
    }
  }
  // alarm for -t program termination
  //signal(SIGALRM, sigAlarmHandler);
  //alarm(time);

  // signal hander
  if (setupinterrupt() == -1) {
    perror("Failed to set up handler for SIGPROF");
    return 1;
  }
  if (setupitimer(time) == -1) {
    perror("Failed to set up the ITIMER_PROF interval timer");
    return 1;
  }

  //ctrl c termination
  signal(SIGINT, sigint_handler);
  //printf("time: %d\n", time);

  //main program -t termination signal
  alarm(time);
  signal(SIGALRM, sigAlarmHandler);

  // argc statement determines if a file was supplied
  if (argc > optind){
      //printf("argc > optind, a file was supplied\n");
      //printf("%s \n", optarg);
  } else if (optind == argc){
      //printf("optind == argc; this is a default\n");
  } else {
      //printf("error-> valid input message HERE\n");
  }

  // printing assigned fileName
  //printf("ifname & ofname :   %s & %s\n", ifname, ofname);

  // FILE VALIDAITON DONE
  // OPENING INPUT FILE
  file = fopen(ifname, "r");
  if(file == NULL){
    perror(argv[0]);
    perror("input file: Error: input file failed to open");
    return EXIT_FAILURE;
  }

  ofp = fopen(ofname, "r");
  if(ofp == NULL){
    perror(argv[0]);
    perror("output file: Error: design decision FILE MUST EXIST.");
    return EXIT_FAILURE;
  }

  // OPENING OUTPUT FILE
  ofp = fopen(ofname, "w");
  if(ofp == NULL){
    perror(argv[0]);
    perror("output file: Error: output file failed to open.");
    return EXIT_FAILURE;
  }


  //variables for counting integers per line
  char arr[1024], *p, *e;
  long v;

  //variables for storing data read from input file
  int lineC = 0;
  int lengthsArrs[128];
  int parsedArrs[128][128];
  int x = 0, y = 0; // x is the number of loops;
  int total = 0;

  //variable for forks
  int status;

  //variables for passing values to subsetsum fx
  int j;

  //scan the first line to remove it
  fscanf(file, "%d\n", &intBuff[i]);
  //printf("child fscanf: %d\n", intBuff[i]);
  //
  // stores input file into 2d array, based on number of rows from the first lines input
  // than parses each line after from 2...n, storing values in 2d array
  // x is the current lines index, y is the current iteration of that lines index
  while (fgets(arr, sizeof(arr), file)) {
    p = arr;
    for(p = arr; ; p = e){
      v = strtol(p, &e, 10);
      parsedArrs[x][y] = v;
      y = y + 1;
      if(p == e)
        break;
      lineC = lineC +1;
      lengthsArrs[x] = lineC; // storing the length in an array w/ index == parsedArr[][]
    }
    //printf("lineC: %d\n", lineC);
    lineC = 0;
    x = x+1; // iterate to next array
    y = 0; // restore column counter to zero
  }
        // END OF COMMANDLINE //

  // working with pid and cids.
  int pids[x]; // int array to hold childpids, value based on x processes
  pids[0] = -2; //a default value i can identifiy
  j = 0;
  temp = x;
  int index;

  // arr variable for storing subsetsum solution
  int subset[64];
  int count = 0;
  while(quitFlag !=1){
    if (done == 1)
      break;
    for(i = 0; i < temp; i++){
      //trygn to stop processes with -t, canot get child kill to work
      //printf("time to terminate all \n");
      j = i;
      // storing values for passing to subset sum
      // lengthsArrs[x] = size n; parsedArrs[x][y] = set, parsedArrs[x][0] = sum
      int n = lengthsArrs[i] - 1;
      int sum = parsedArrs[i][0];
      int set[64];
      for(y = 0; y < n; y++)
        set[y] = parsedArrs[i][y+1];

      // fork off and process subsetsum inside child proceses with parent waiting
      pid = fork();

      // setting signal handlers after forking so child dosn't inherit them?
      // signal(SIGQUIT, sigquit_handler);

      if (pid == -1){
      perror("forkError: Error: failed to fork pid = -1 ");
      }
      // if child processe go
      if (pid == 0){
        // child process starts here
        // fprintf pid to output.dat
        fprintf(ofp, "%ld: ", (long) getpid());

        // passing to isSubsetSum(set[], size n, sum)
        before = clock();
        if(isSubsetSum(set, n, sum, subset, count) == true){
          //printf("found a subset sum\n");
          //printing to terminal solution of subsetsum
          for(i = 0; i < globalCount; i++){
             //printf(" %d  ",  store[i]);
              fprintf(ofp, "%d ", store[i]);
              if(i < (globalCount - 1)){
                fprintf(ofp, "+ ");
              }
            }
          fprintf(ofp,"= %d", parsedArrs[j][0]);
          fprintf(ofp,"\n");
          //printf("\n");
          // reset global after printing
          globalCount = 0;
          count = 0;
        } else {
          fprintf(ofp," No subset of numbers summed to %d.\n", sum);
          msec = 0;
        }
        return 0;
      } else {
        // storing child pids for output and termination
        child_pid = pid;
        pids[i] = pid;
        waitpid(pid, &status, 0);
        //printf("ParentPID is %ld\n", (long) getpid());
        //printf("childPID is %d\n",pids[i]);
      }
    }
   done = 1;

  }

  fclose(file);
  file = NULL;
  return 0;

}

        // functions

bool isSubsetSum(int set[], int n, int sum, int subset[], int count){
  int i = 0;
  iterations++;
  clock_t difference = clock() - before;
  msec = difference * 1000 / CLOCKS_PER_SEC;
  if(msec > trigger){
    //printf("too long\n");
    kill(child_pid,SIGKILL);
    return false;
  } else {

    if(sum == 0){
      for(i=0; i < count; i++){
        //fix leading zero appends here
        store[i] = subset[i];
        globalCount = count;
      }
      return true;
    }
    if (n < 0 && sum != 0) return false;
    subset[count] = set[n];
    return
       isSubsetSum(set, n-1, sum-set[n], subset, count + 1)
         || isSubsetSum(set, n-1, sum, subset, count );
  }
}

  // ARGSUSED
static void myhandler(int s) {
 int errsave;
 errsave = errno;
 childQuitFlag = 1;
 printf("1 second childprocess limit");
 errno = errsave;
}

static int setupinterrupt(void) { // set up myhandler for SIGPROF */
 struct sigaction act;
 act.sa_handler = myhandler;
 act.sa_flags = 0;
 return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}

static int setupitimer(int time) { // set ITIMER_PROF for 2-second intervals */
 struct itimerval value;
 value.it_interval.tv_sec = 1;
 value.it_interval.tv_usec = 0;
 value.it_value = value.it_interval;
 return (setitimer(ITIMER_PROF, &value, NULL));
}


// ctrl c
void sigint_handler(int sig_num){
  printf("siginit: %d\n", sig_num);
  signal(SIGINT, sigint_handler);
}
// main loop
void sigAlarmHandler(int sig_num){
  quitFlag = 1;
  int i;
  printf("-t time exceeded\n");
  for(i = 0; i < temp; i++){
    kill(child_pid, SIGKILL);
  }
  signal(SIGALRM, sigAlarmHandler);

}




[freeman@hoare7 freeman.2]$
