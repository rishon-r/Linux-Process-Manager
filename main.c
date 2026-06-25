#include <string.h> // For string functions
#include <stdbool.h> // For boolean values
#include <stdlib.h> // For malloc(), free(), exit()
#include <unistd.h> // For fork(), exec*() and sysconf(_SC_CLK_TCK)
#include <stdio.h> // For printf(), scanf(), fgets()
#include <stdlib.h>
#include <sys/types.h> // For pid_t
#include <sys/wait.h> // For wait(), waitpid()
#include <signal.h> // For kill()
#include <errno.h>
#include "linked_list.h" // For our version of PCB


Node* head = NULL; // Initializing the value of head to be null when there are no jobs


void func_BG(char **cmd){
  // Here, we aim to execute the process whose name is mentioned in lst[2], in the background
  char *process_name= cmd[1]; // Pointer to first character of the process name



  pid_t pid= fork(); // We create a child process using fork() in order to run the required process in the background

  if (pid < 0){
    perror("Error in fork\n"); // pid is less than 0 only in the event that fork() fails to create a child process
    return;
  }
  else{
    if (pid==0){
      // Inside the child process where pid is 0
      // We pass a pointer to the second character in character array which will be the beginning of the list of arguments
      if (execvp(process_name, cmd + 1) < 0){
          perror("Exec failed\n"); // In the event that exec fails print error
          exit(1); // exit with code 1
      }
    }
    else{
      // Inside the parent process where the pid is the process id of child
      char *ps_path= strdup(process_name); // setting the path to be passed by duplicating the string
      if (ps_path==NULL){
        perror("Unable to duplicate string"); // In case strdup() fails
        return;
      }
      head= add_newNode(head, pid, ps_path); // When a new process is created to run in the background, we also add a new node to our linked list of processes
    }
  }
  return; // void return type, will return nothing

}


void func_BGlist(char **cmd){
	// bglist command should make the program list all the processes currently running in the background along with their PID
        // Output should look like this:
        /*
        123: /home/user/a1/foo
        456: /home/user/a1/foo
        Total background jobs: 2
        */
  int count=0; // Used to count the number of background jobs
  Node *cur=head; // setting current node equal to the head node

  while(cur!=NULL){
    // Looping through all nodes in the linked list and displaying their results
    printf("%d: %s\n", cur->pid, cur->path);
    cur=cur->next; // Updatijg value of cur to be the next node in linked list
    count++; // Incrememnting the count of the number of background jobs
  }

  printf("Total background jobs: %d\n", count); 
  return;
}


void func_BGkill(char * str_pid){
	// Here, we are passed the pid and are requested to kill it
  // But the pid passed is in the form of a string and we need one of integer type or pid_t type in order to pass it to kill

  // In case no argument is passed, we error check in order to avoid segmentation fault
  if (str_pid==NULL){
    printf("Error: No pid passed\n");
    return;
  }

  // CONVERTING TO INT
  char *endpt; //endpt variable to pass to strtol
  pid_t term_pid= (pid_t)strtol(str_pid, &endpt, 10);


  int retVal= kill(term_pid, SIGTERM); // PASSING SIGTERM so that the process terminates

  if (retVal==0){
      return; // Here retVal is zero if kill() is a success. We don't delete the node from linkedlist here as that is handled in main() after waitpid() is performed.
  }
  else{
    perror("Could not kill process"); // kill() returns -1 when it fails, in that case we will print an error message
  }
}


void func_BGstop(char * str_pid){
	// Here, we are passed the pid and are requested to stop it
  // But the pid passed is in the form of a string and we need one of integer type or pid_t type in order to pass it to kill

  // In case no argument is passed, we error check in order to avoid segmentation fault
  if (str_pid==NULL){
    printf("Error: No pid passed\n");
    return;
  }

  // CONVERTING TO INT
  char *endpt; //endpt variable to pass to strtol
  pid_t stop_pid= (pid_t)strtol(str_pid, &endpt, 10);


  int retVal= kill(stop_pid, SIGSTOP); // PASSING SIGSTOP so that the process stops

  if (retVal==0){
      return; // retVal of zero means kill() succeeded and the process has been stopped temporarily
  }
  else{
    perror("Could not stop process"); // Error message in case kill() fails
  }
}


void func_BGstart(char * str_pid){
	// Here, we are passed the pid and are requested to kill it
  // But the pid passed is in the form of a string and we need one of integer type or pid_t type in order to pass it to kill

  // In case no argument is passed, we error check in order to avoid segmentation fault
  if (str_pid==NULL){
    printf("Error: No pid passed\n");
    return;
  }

  // CONVERTING TO INT
  char *endpt; // endpt variable to pass to strtol
  pid_t cont_pid= (pid_t)strtol(str_pid, &endpt, 10);


  int retVal= kill(cont_pid, SIGCONT); // PASSING SIGCONT so that the process continues

  if (retVal==0){
      return; // in case kill() succeeds and the process resumes
  }
  else{
    perror("Could not continue process"); // If kill() fails we print an error
  }
}


void func_pstat(char * str_pid){

  // In case no argument is passed, we error check in order to avoid segmentation fault
  if (str_pid==NULL){
    printf("Error: No pid passed\n");
    return;
  }

	// Here, we're passed the pid of a process and we have to display some statistics about it
  // For that we'll have to look inside the virtual /proc directory
  pid_t int_pid=strtol(str_pid, NULL, 10);
  if(PifExist(head, int_pid)==0){
    printf("Error: Process %s does not exist.\n", str_pid); // If pid is not in the linked list of background processes, we print an error message
  }

else{ // If PID does exist in list of background processes
  // To get comm, state, utime, stime and rss, we need to read from /proc/pid/stat
  // To get voluntary_ctxt_switches and nonvoluntary_ctxt_switches, we need to read from /proc/pid/status

  // Below, we will create the paths for /proc/pid/stat and /proc/pid/status
  char stat_path[255]; // Buffer to store the stat path
  char status_path[255]; // buffer to store the status path
  snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", str_pid); // creating the stat path
  snprintf(status_path, sizeof(status_path), "/proc/%s/status", str_pid); // creating the statuss path


  // First, we will read from the /proc/pid/stat
  FILE *fp= fopen(stat_path, "r"); // Opening the file in read mode (we do not want to write anything in /proc)

  if (fp==NULL){
    perror("Could not read stat file in proc");
    return;
  }
  char buffer[1024];
  fgets(buffer, 1024, fp); // /proc/pid/stat is a one line file with required information at particular places
  fclose(fp); // closing the file
  // We only need to read once as this is a one line file

  // Now, we will tokenise the string stored in buffer to determine our required values
  // Note that in the file: comm is at index 2, state at index 3, utime at index 14, stime at index 15, rss at index 24
  // We will tokenise the string and delimit it based on space so that we can access all information accurately
  // Declaring variables for data we need:
  char comm[255]; // We initialize a buffer instead of a pointer to a string here as we want it to be large enough to accomodate a laarge filename
  char state;
  unsigned long utime;
  unsigned long stime;
  unsigned long rss;
  // We don't use int for the above values due to the fact that utime, rss and stime may all blow past the space traditionally given to an int

  char *ptr= strtok(buffer, " \n");
  int file_index=1;

  while (ptr!=NULL){
    if (file_index==2){
      int comm_length=strlen(ptr);

      if (ptr[comm_length-1]==')'){
        // This is the case where there are no spaces inside comm and the tokeniser doesn't break comm itself into multiple pieces
        // as we are tokenising with spaces
        // In this casse the value of comm is simply the string that ptr points to
        strcpy(comm, ptr);
      }
      else{
        strcpy(comm, ptr); // Copy the initial value of the pointer before we continue looping
        while(ptr[comm_length-1]!=')'){
          // We loop until the last character of the string is ')'
          ptr= strtok(NULL, " \n"); // We continue tokenising string
          // However, we do not update the index variable as we are still within index 2 , looking for the full comm value
          strcat(comm, " "); // First, we concatenate the space that we delimited
          strcat(comm, ptr); // Then we add the remainder of the string
        }
        // When it exits it will go to the bottom of parent loop where the
      }
    }
    else if(file_index==3){
      state= ptr[0]; // state is a character
    }
    else if(file_index==14){
      utime= strtoul(ptr, NULL, 10) / sysconf(_SC_CLK_TCK); // utime is measured in clock ticks which we have to divide by sysconf(_SC_CLK_TCK)
    }
    else if(file_index==15){
      stime= strtoul(ptr, NULL, 10) / sysconf(_SC_CLK_TCK); // stime is measured in clock ticks which we have to divide by sysconf(_SC_CLK_TCK)
    }
    else if(file_index==24){
      rss= strtoul(ptr, NULL, 10); // rss is a long int
    }
    ptr= strtok(NULL, " \n"); //Updating the string pointer in order to continue tokenising the file
    file_index++; // Updating index value at the end of every iteration
  }

  // Now, we will read from the /proc/pid/status

  // We will initialize variables to hold the data we require
  int vltr_ctx_swtchs;
  int nvltr_ctx_swtchs;

  // Initializing buffer and file pointer
  char new_buff[1024];
  FILE *pf= fopen(status_path,"r");

  if (pf==NULL){
    perror("Cannot read status file in /proc"); // In the event of an empty file
    return;
  }

  // Here, the file is in a human readable format
  // The data we require is typically at the last two lines of the file
  // We read the file line by line and use strncmp to check if the first value of each line corresponds with that of the attributes whose value we want to read

  while (fgets(new_buff, sizeof(new_buff), pf)!=NULL){
    if(strncmp(new_buff, "voluntary_ctxt_switches", 24)==0){
      char *ptr1= strtok(new_buff, " ");
      if (ptr1!=NULL){
        ptr1=strtok(NULL, " "); // We move the pointer by 1 as the numerical data we require is stored after the first sequence of spaces
      }
      vltr_ctx_swtchs= strtol(ptr1, NULL, 10); // We then have to convert the string to a numerical format

    }
    else if(strncmp(new_buff, "nonvoluntary_ctxt_switches", 27)==0){
      char *ptr2= strtok(new_buff, " ");
      if (ptr2!=NULL){
        ptr2=strtok(NULL, " "); // We move the pointer by 1 as the numerical data we require is stored after the first sequence of spaces
      }
      nvltr_ctx_swtchs= strtol(ptr2, NULL, 10); // We then have to convert the string into a numerical format
    } 
  }

  fclose(pf); // Closing the file after we have extracted all required information

  // Now, we print out all the details for the given process
  printf("Process %s has the following details -\n", str_pid);
  printf("comm: %s\n", comm);
  printf("state: %c\n", state);
  printf("utime: %lu\n", utime);
  printf("stime: %lu\n", stime);
  printf("rss: %lu\n", rss);
  printf("voluntary context switches: %d\n", vltr_ctx_swtchs);
  printf("non voluntary context switches: %d\n", nvltr_ctx_swtchs);

  } 
}

int main(){
    char user_input_str[50];
    while (true) {

      // Infinite loop for user input
      // Our goal here is that PMan will accept user input and run programs in the background while continuing to prompt for user input
      // i.e we try to input our own background execution

      // Before we prompt for user input, we do a check here to see if any background jobs have been terminated
      int status; // status variable that we need to pass to waitpid()
      pid_t done; //initializing variab.e to store pid of job that just terminated
      while((done= waitpid(-1, &status, WNOHANG)) > 0){
        // We check if any child process running has finished execution and loop till it returns 0 (which means no job has terminated)
        // -1 passed to waitpid() means it checks for all child processes and WNOHANG makes it non-blocking
        printf("Background job %d has terminated\n", done);
        head= deleteNode(head, done); // deleting the node pertaining to the job that just finished from and resetting head appropriately
      }

      // Now, we can start prompting for user input

      printf("PMan: > "); // Prompts for user to input required command
      // Reads 50 characters from the Standard input stream (stdin) which is an abstraction for keyboard into character array user_input_str
      fgets(user_input_str, 50, stdin); 
      printf("User input: %s \n", user_input_str); // Here, we print back out the user input

      // Below, we begin to tokenize the string that we have accepted as input
      // For this we use the strtok(char *star, const char *delims) function
      // The strtok() function splits the passed string into tokens based on the delimiters passed
      // It will return the pointer to the first token in the string. We can increment this pointer to get the next token and so on. 
      // Will return NULL if no more tokens are found
      // To use it properly and get all parts of the delimited string we need to use it in a loop
      char *ptr = strtok(user_input_str, " \n");
      if(ptr == NULL){
        // Checking for empty string
        continue;
      }
      char *lst[50]; // Initialising a list of 50 characters
      int index = 0; // Initialising index to 0
      lst[index] = ptr;
      index++;
      while(ptr != NULL){ // We keep looking till there exist no more tokens
        ptr = strtok(NULL, " \n"); // Pass NULL on subsequent calls to keep tokenizing the same string
        lst[index]=ptr; //  Store every delimiteed part of the string into a list lst
        index++; // increment index so that you store them sequentially
      }

      // strcmp(const char *str1, const char *str2) function is used to compare two strings
      // Compares two strings lexicographically, returns 0 if they are equal
      // negative if str1 comes before str2 lexicographically, positive if it comes after

      // Below, we compare the first delimited part of user input (which should be our command, indicating what task to execute) with all commands we provide

      if (strcmp("bg",lst[0]) == 0){
        // If the first part of command is bg, we need to run the program name mentioned as second part of command in the background
        // E.g bg foo will make us run the program foo in the background while PMan continues to ask for prompts
        func_BG(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        // bglist command should make the program list all the processes currently running in the background along with their PID
        // Output should look like this:
        /*
        123: /home/user/a1/foo
        456: /home/user/a1/foo
        46 Total background jobs:
        */
        func_BGlist(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        // This is for the command of the form bgkill pid
        // Which will have to send the TERM signal to the job with given pid
        // Your PMan should let the users know when a process has been terminated. The suggestion is to use waitpid() with the WNOHANG option
        func_BGkill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        // This is for the command of type bgstop pid
        // Which will send the STOP signal to the job with given pid, which will stop the process temporarily
        func_BGstop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        // This is for the command of the type bgstart pid
        // This will send the CONT signal to the process with given pid, which will restart the process
        func_BGstart(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        // pstat pid prompts us to list information pertaining to a particular process with the given pid
        // The information required is listed in the function documentation of func_pstat
        // Read from /proc to determine the details corresponding to a process

        // ON /proc
        // /proc is a virtual file system, built from scratch every time that we boot the machine
        // It exists in the RAM as long as the machine is running and disappears when the machine is powered off
        // Then, only the /proc empty directory is left, which is kind of used as a mount point
        // Inside the proc direcotry, there are a bunch of folders with their names being numbers
        // Each of these numbers corresponds to the PID of a running process i.e Each process has a folder in /proc
        // This folder however is of size 0, it is a virtual file with a pointer to the PCB
      
        func_pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
        // Entering q quits PMan
        printf("Bye Bye \n");
        exit(0);
      } else {
        // You need to return a valid error message if the user enters an invalid command
        printf("Invalid input\n");
      }
    }

  return 0;
}

