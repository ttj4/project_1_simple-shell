#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void op_redirection(char *argar[],int op_n);
void clear_array(char *argar[]);
void piping(char *argv[],int p_n);
void change_array(char *argv[],int p_n);
void errExit(char *err);
main()
{
	char ar[100],pathname[50];
	char *newline, *d, *name;
	char *argar[10];
	int argn, flag, pid, op_redirection_flag, op_n, i;
	int p_flag, p_n, p_fork;
	
	while (1) {

		name = getcwd(pathname,sizeof(pathname));
		printf("[my-shell : %s]$: ",name);
		
		fgets(ar,sizeof(ar),stdin);
		
		
		newline = strchr(ar,'\n');
		if (newline)
			*newline = 0;
		clear_array(argar);
		argn = 0;
		op_n = 0;
		p_n = 0;
		d = strtok(ar," ");
		while (d != NULL) {
			argar[argn++] = d;
			if (strcmp(argar[argn-1],">") == 0){
					op_redirection_flag = 1;
					op_n = argn-1;
					}
			if (strcmp(argar[argn-1],"|") == 0) {
					p_flag = 1;
					p_n = argn-1;
			}
			d = strtok(NULL," ");
		}
		if (op_redirection_flag == 1) {
			op_redirection(argar,op_n);
			op_redirection_flag = 0;
		}
		else if (p_flag == 1) {
			p_fork = fork();
			switch(p_fork) {
				case -1:
					errExit("fork");
					break;
				case 0:
					piping(argar,p_n);
					break;
				default:
					wait(&p_fork);
					break;
			}

			p_flag = 0;
		}
		else {
			i = 0;
			pid = fork();
			switch (pid) {
				case -1:
					errExit("fork");
					break;
				case 0:
					execvp(argar[0],argar);
					break;
				default:
					wait(&pid);
					break;
				}
			clear_array(argar);
		}
	}
	return 0;
}

void clear_array(char *argar[])
{
	int i = 0;
	while (i < 10) {
		argar[i] = 0;
		i++;
	}
}



void op_redirection(char *argar[],int op_n)
{
	int fd,pid;
	char *buf;
	buf = argar[op_n+1];
	argar[op_n] = 0;
	pid = fork();
	if (pid == -1)
		errExit("fork");
	else if (pid == 0) {
		fd = open(buf,O_RDWR | O_CREAT | O_TRUNC,0644);
		close(1);
		dup(fd);
		execvp(argar[0],argar);
	}
	else {
		wait(&pid);
	}
}




void piping(char *argar[],int p_n)
{
	int pfd[2];
	int fd, pid;
	pipe(pfd);
	pid = fork();
	switch(pid) {
		case -1:
			errExit("fork");
			break;
		case 0:
			argar[p_n] = 0;
			close(1);
			dup(pfd[1]);
			close(pfd[0]);
			execvp(argar[0],argar);
			break;
		default:
			change_array(argar,p_n);
			close(0);
			dup(pfd[0]);
			close(pfd[1]);
			execvp(argar[0],argar);
			break;
	}
}

void change_array(char *argar[],int flag)
{
	int i = 0;
	int count = 0;
	int p = flag + 1;
	while (argar[i] != 0) {
		count++;
		i++;
	}
	--count;
	i = 0;
	while (i <= flag) {
		argar[i] = argar[p];
		i++;
		p++;
	}
	argar[count-flag] = 0;
}

void errExit(char *err)
{
	perror(err);
	exit(EXIT_FAILURE);
}
