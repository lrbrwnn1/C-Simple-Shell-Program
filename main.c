#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include<dirent.h>

char  line[1024];
char  *parameters[64]; 
char   *input[2];
char  **stuff; //just a temp variable to hold make it easier to pass some stuff to runProgram
int fd;
static int writeflag = 0; // set to 1 if '>' detected
static int readflag = 0; // set to 1 if '<' detected

void  inputParser(char *line, char **parameters)
{
	int i = 0;

	while (*line != '\0') 
	{
		while (*line == '\n' || *line == ' ')
			*line++ = '\0';//get rid of whitespace
		if (*line == '>'){ //checks for write command
			writeflag=1;
		}
		if (*line == '<'){ //checks for read command
			readflag=1;
		}
		*parameters++ = line;// save arg position

		while (*line != '\0' && *line != '\n' && *line != ' ')
			line++; //if line isn't ended or already parsed, keep going
	}
	*parameters = '\0';
}

int runProgram(char **argc)
{
  pid_t pid, otherpid;
  int status;

  pid = fork(); //forks child process
  if (pid == 0) {
    if (execvp(argc[0], argc) == -1) {
      printf("Error executing process\n"); //error catching
    }
    exit(0);
  } else if (pid < 0) {
    printf("Error executing process\n");//error catching
  } else {
    do {
      otherpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int main(int argc, char **parameters)
{
	char readData[2048];//char array to read data from a file

  while(1){
    printf("uofmsh > ");
    fgets(line, 20, stdin);

    input[0] = line;
		input[1] = NULL; 
		stuff = input;

    inputParser(line,parameters); //parse input

    if (strcmp(parameters[0],"exit") == 0) { //exit command
      exit(0);
    }

    if (strcmp(parameters[0],"cd") == 0) {
      chdir(parameters[1]);
			continue; 
    }

		if (writeflag == 1){ // deals with '>', writes to file
			fd = open(parameters[2],O_WRONLY | O_CREAT | O_APPEND, 0644); //set to write only, create if it doesn't exist, and append to the end of a file if there is pre-existing data.
			runProgram(stuff); //run command to get output
			dup2(fd, 1); //copy output into destination file
			close(fd); //close file
			writeflag = 0; //reset writeflag
			break;
		}

		if (readflag == 1){ // deals with '<', reads file
			fd = open(parameters[2],O_RDONLY, 0644); //set to read only and open file.

			read(fd, readData, 128); //store text in char array
			close(fd); //close file
			readflag = 0; //reset readflag
			printf("%s\n", readData); //displays contents of file
			break;
		}
		runProgram(stuff); //finally if nothing else catches, exec.
  }
}
