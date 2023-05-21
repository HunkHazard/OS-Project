
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "executor.h"
#include <sysexits.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sched.h>
#include <signal.h>

static int execute_aux(struct tree *t, int o_input_fd, int p_output_fd);

int execute(struct tree *t)
{
   execute_aux(t, STDIN_FILENO, STDOUT_FILENO); /*processing the root node first*/

   /* print_tree(t);*/

   return 0;
}
/*
static void print_tree(struct tree *t) {
   if (t != NULL) {
      print_tree(t->left);

      if (t->conjunction == NONE) {
         printf("NONE: %s, ", t->argv[0]);
      } else {
         printf("%s, ", conj[t->conjunction]);
      }
      printf("IR: %s, ", t->input);
      printf("OR: %s\n", t->output);

      print_tree(t->right);
   }
}
*/
static int execute_aux(struct tree *t, int p_input_fd, int p_output_fd)
{
   int pid, status, new_pid, fd, fd2, success_status = 0, failure_status = -1, new_status;
   int pipe_fd[2], pid_1, pid_2;

   /*none conjuction processing*/
   if (t->conjunction == NONE)
   {

      /*Check if exit command was entered, exit if it was*/
      if (strcmp(t->argv[0], "exit") == 0)
      {
         exit(0);
      }
      /*check if user wants to change directory, if so change directory*/
      else if (strcmp(t->argv[0], "cd") == 0)
      {

         if (strlen(t->argv[1]) != 0)
         {
            if (chdir(t->argv[1]) == -1)
            {
               perror(t->argv[1]);
            }
         }
         else
         {
            chdir(getenv("HOME")); /*change to home directory if no argument is provided*/
         }
      }

      /* check if user wants to remove a file, if so remove the file */
      else if (strcmp(t->argv[0], "rm") == 0)
      {
         if (t->argv[1] != NULL)
         {
            if (remove(t->argv[1]) == -1)
            {
               /* perror(t->argv[1]);*/
            }
         }
         else
         {
            fprintf(stderr, "rm: missing operand\n");
         }
      }

      /* check if user wants to print working directory, if so print it */
      else if (strcmp(t->argv[0], "pwd") == 0)
      {
         char cwd[1024];
         if (getcwd(cwd, sizeof(cwd)) != NULL)
         {
            printf("%s\n", cwd);
         }
         else
         {
            perror("pwd");
         }
      }

      /* check if user wants to create a file, if so create it */
      else if (strcmp(t->argv[0], "touch") == 0)
      {
         if (t->argv[1] != NULL)
         {
            int fd = open(t->argv[1], O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK, 0666);
            if (fd == -1)
            {
               perror(t->argv[1]);
            }
            else
            {
               close(fd);
            }
         }
         else
         {
            fprintf(stderr, "touch: missing file operand\n");
         }
      }

      /* check if user wants to print the first 10 lines of a file, if so print them */
      /* else if (strcmp(t->argv[0], "head") == 0)
       {
          if (t->argv[1] != NULL)
          {
             FILE *file = fopen(t->argv[1], "r");
             if (file == NULL)
             {
                perror(t->argv[1]);
             }
             else
             {
                char line[256];
                for (int i = 0; i < 10; i++)
                {
                   if (fgets(line, sizeof(line), file) == NULL)
                   {
                      break;
                   }
                   printf("%s", line);
                }
                fclose(file);
             }
          }
          else
          {
             fprintf(stderr, "head: missing file operand\n");
          }
       }*/

      /* check if user wants to rename/move a file, if so do it */
      else if (strcmp(t->argv[0], "mv") == 0)
      {
         if (t->argv[1] != NULL && t->argv[2] != NULL)
         {
            if (rename(t->argv[1], t->argv[2]) == -1)
            {
               perror("mv");
            }
         }
         else
         {
            fprintf(stderr, "mv: missing file operand\n");
         }
      }

      /* check if user wants to copy a file, if so do it */
      else if (strcmp(t->argv[0], "cp") == 0)
      {
         if (t->argv[1] != NULL && t->argv[2] != NULL)
         {
            FILE *source_file = fopen(t->argv[1], "r");
            if (source_file == NULL)
            {
               perror(t->argv[1]);
            }
            else
            {
               FILE *destination_file = fopen(t->argv[2], "w");
               if (destination_file == NULL)
               {
                  perror(t->argv[2]);
               }
               else
               {
                  char c;
                  while ((c = fgetc(source_file)) != EOF)
                  {
                     fputc(c, destination_file);
                  }
                  fclose(destination_file);
               }
               fclose(source_file);
            }
         }
         else
         {
            fprintf(stderr, "cp: missing file operand\n");
         }
      }
      /* check if user wants system information */
      else if (strcmp(t->argv[0], "sysinfo") == 0)
      {
         struct sysinfo s_info;
         if (sysinfo(&s_info) == 0)
         {
            printf("Uptime: %ld\n", s_info.uptime);
            printf("Load averages: %ld, %ld, %ld\n", s_info.loads[0], s_info.loads[1], s_info.loads[2]);
            printf("Total RAM: %ld\n", s_info.totalram);
            printf("Free RAM: %ld\n", s_info.freeram);
            printf("Process count: %d\n", s_info.procs);
         }
         else
         {
            perror("sysinfo");
         }
      }
      /* check if user wants system name */
      else if (strcmp(t->argv[0], "uname") == 0)
      {
         struct utsname u_name;
         if (uname(&u_name) == 0)
         {
            printf("System name: %s\n", u_name.sysname);
            printf("Node name: %s\n", u_name.nodename);
            printf("Release: %s\n", u_name.release);
            printf("Version: %s\n", u_name.version);
            printf("Machine: %s\n", u_name.machine);
         }
         else
         {
            perror("uname");
         }
      }

      /* check if user wants the hostname */
      else if (strcmp(t->argv[0], "gethostname") == 0)
      {
         char hostname[1024];
         if (gethostname(hostname, sizeof(hostname)) == 0)
         {
            printf("Hostname: %s\n", hostname);
         }
         else
         {
            perror("gethostname");
         }
      }

      /* check if user wants the process ID */
      else if (strcmp(t->argv[0], "getpid") == 0)
      {
         pid_t pid = getpid();
         printf("Process ID: %d\n", pid);
      }
      /* check if user wants resource usage */
      else if (strcmp(t->argv[0], "getrusage") == 0)
      {
         struct rusage usage;
         if (getrusage(RUSAGE_SELF, &usage) == 0)
         {
            printf("User CPU time used: %ld seconds\n", usage.ru_utime.tv_sec);
            printf("System CPU time used: %ld seconds\n", usage.ru_stime.tv_sec);
            printf("Maximum resident set size: %ld\n", usage.ru_maxrss);
            printf("Page reclaims (soft page faults): %ld\n", usage.ru_minflt);
            printf("Page faults (hard page faults): %ld\n", usage.ru_majflt);
         }
         else
         {
            perror("getrusage");
         }
      }
      /*------------------------------------------------------------------------------------*/

      /* numify - Convert a file into a stream of ASCII values. */
      else if (strcmp(t->argv[0], "numify") == 0)
      {
         char chr;
         int fd1, fd2;
         char buf[1024];
         ssize_t n;
         if (t->argv[1] != NULL && t->argv[2] != NULL)
         {
            fd1 = open(t->argv[1], O_RDONLY);
            if (fd1 == -1)
            {
               perror("open");
               return;
            }
            fd2 = open(t->argv[2], O_WRONLY | O_CREAT, 0644);
            if (fd2 == -1)
            {
               perror("open");
               return;
            }

            while ((n = read(fd1, &chr, sizeof(chr))) > 0)
            {
               snprintf(buf, sizeof(buf), "%d ", chr);
               if (write(fd2, buf, strlen(buf)) == -1)
               {
                  perror("write");
                  return;
               }
            }

            if (n == -1)
            {
               perror("read");
               return;
            }

            close(fd1);
            close(fd2);
         }
      }
      else if (strcmp(t->argv[0], "addfile") == 0)
      {
         char buf[1024];
         ssize_t len;
         if (t->argv[1] != NULL && t->argv[2] != NULL && t->argv[3] != NULL)
         {
            int fd1 = open(t->argv[1], O_RDONLY);
            int fd2 = open(t->argv[2], O_RDONLY);
            int fd3 = open(t->argv[3], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd1 == -1 || fd2 == -1 || fd3 == -1)
            {
               perror("open");
               return;
            }

            while ((len = read(fd1, buf, 1024)) > 0)
            {
               write(fd3, buf, len);
            }
            while ((len = read(fd2, buf, 1024)) > 0)
            {
               write(fd3, buf, len);
            }
            close(fd1);
            close(fd2);
            close(fd3);
         }
      }

      /*process_info pid*/
      else if (strcmp(t->argv[0], "process_info") == 0)
      {
         if (t->argv[1] != NULL)
         {
            pid_t pid;
            char path[1024];
            FILE *f;
            int pid_value, ppid_value, pgrp_value, session_value, tty_nr_value, tpgid_value;
            unsigned int flags_value;
            int uid_value, gid_value;

            pid = atoi(t->argv[1]);
            sprintf(path, "/proc/%d/stat", pid);

            f = fopen(path, "r");
            if (!f)
            {
               perror("fopen");
               return;
            }

            fscanf(f, "%d %*s %*c %d %d %d %d %d %u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %d %d",
                   &pid_value, &ppid_value, &pgrp_value, &session_value, &tty_nr_value, &tpgid_value, &flags_value, &uid_value, &gid_value);

            printf("PID: %d\n", pid_value);
            printf("Parent PID: %d\n", ppid_value);
            printf("Process group ID: %d\n", pgrp_value);
            printf("Session ID: %d\n", session_value);
            printf("Controlling terminal: %d\n", tty_nr_value);
            printf("User ID: %d\n", uid_value);
            printf("Group ID: %d\n", gid_value);

            fclose(f);
         }
         else
         {
            fprintf(stderr, "Error: Process ID is required\n");
            return;
         }
      }

      /* process_snapshot - Take a snapshot of the current process state and save it to a file. */

      else if (strcmp(t->argv[0], "process_snapshot") == 0)
      {
         const char *stat_keys[] = {
             "Process ID",
             "Process Name",
             "State",
             "Parent Process ID",
             "Real User ID",
             "Effective User ID",
             "Process Group ID",
             "Session ID",

         };

         if (t->argv[1] != NULL)
         {
            pid_t pid;
            char snapshot_filename[256], cmdline_path[64], stat_path[64], buffer[1024];
            FILE *cmdline_file, *stat_file, *snapshot_file;
            size_t bytes;
            int key_count;
            int count;
            int current_key;
            pid = atoi(t->argv[1]);
            snprintf(snapshot_filename, sizeof(snapshot_filename), "%s_snapshot.txt", t->argv[1]);
            sprintf(cmdline_path, "/proc/%d/cmdline", pid);
            sprintf(stat_path, "/proc/%d/stat", pid);

            cmdline_file = fopen(cmdline_path, "r");
            stat_file = fopen(stat_path, "r");
            snapshot_file = fopen(snapshot_filename, "w");

            if (cmdline_file == NULL || stat_file == NULL || snapshot_file == NULL)
            {
               perror("fopen");
               return;
            }

            fprintf(snapshot_file, "Cmdline:\n");

            while ((bytes = fread(buffer, 1, sizeof(buffer), cmdline_file)) > 0)
            {

               char *token = strtok(buffer, " ");
               while (token != NULL)
               {
                  fprintf(snapshot_file, "Cmdline Key: %s\n", token);
                  token = strtok(NULL, " ");
               }
            }

            fprintf(snapshot_file, "\nStat:\n");

            key_count = sizeof(stat_keys) / sizeof(stat_keys[0]);
            current_key = 0;

            while ((bytes = fread(buffer, 1, sizeof(buffer), stat_file)) > 0)
            {

               char *token = strtok(buffer, " ");
               count = 0;
               while (token != NULL && current_key < key_count)
               {
                  if (count == current_key)
                  {
                     fprintf(snapshot_file, "%s : %s\n", stat_keys[current_key], token);
                     current_key++;
                  }
                  token = strtok(NULL, " ");
                  count++;
               }
            }

            fclose(cmdline_file);
            fclose(stat_file);
            fclose(snapshot_file);
         }
         else
         {
            fprintf(stderr, "Error: Process ID is required\n");
            return;
         }
      }

      /* terminate_process - Sends a SIGTERM signal to a given process. */
      else if (strcmp(t->argv[0], "yeet") == 0)
      {
         pid_t pid;

         if (t->argv[1] != NULL)
         {
            pid = atoi(t->argv[1]);
            if (kill(pid, SIGTERM) == -1)
            {
               perror("kill");
               return;
            }
            printf("SIGTERM signal sent to process %d.\n", pid);
         }
         else
         {
            printf("Please provide a process id to terminate.\n");
         }
      }
      /*-----------------------------------------------------------------------------------*/

      /*process any entered linux commands*/
      if ((pid = fork()) < 0)
      {
         perror("fork");
         exit(-1);
      }
      /*parent processing*/
      if (pid != 0)
      {
         wait(&status);
         return status;
      }

      /*child processing*/
      else
      {

         /*check if we have any input/output files*/
         if (t->input != NULL)
         {
            if ((fd = open(t->input, O_RDONLY)) < 0)
            {
               perror("fd");
               exit(-1);
            }
            /*get input from the file if it exists, and opening it has not failed*/
            if (dup2(fd, STDIN_FILENO) < 0)
            {
               perror("dup2");
               exit(-1);
            }

            /*close the file descriptor*/
            if (close(fd) < 0)
            {
               perror("close");
               exit(-1);
            }
         }

         /*use provided output file if it exists*/
         if (t->output != NULL)
         {
            if ((fd = open(t->output, O_CREAT | O_WRONLY | O_TRUNC)) < 0)
            {
               perror("open");
               exit(-1);
            }

            /*change standard output to get output from provided file if exists*/
            if (dup2(fd, STDOUT_FILENO) < 0)
            {
               perror("dup2");
               exit(-1);
            }

            /*close file descriptor associated with output file*/
            if (close(fd) < 0)
            {
               perror("close");
               exit(-1);
            }
         }

         /*execute the command using*/
         execvp(t->argv[0], t->argv);
         /* fprintf(stderr, "Failed to execute %s\n", t->argv[0]);*/
         exit(-1);
      }
   }

   else if (t->conjunction == AND)
   {

      /*Process left subtree, then right subtree if leftsubtree is processed correctly*/
      new_status = execute_aux(t->left, p_input_fd, p_output_fd);

      /*if the left_subtree was processed was succesfully, process the right subtree */
      if (new_status == 0)
      {
         return execute_aux(t->right, p_input_fd, p_output_fd);
      }
      else
      {
         return new_status;
      }
   }

   /*of the current conjuction is of type pipe, process*/
   else if (t->conjunction == PIPE)
   {

      if (t->left->output != NULL)
      {
         printf("Ambiguous output redirect.\n");
         return failure_status;
      }

      if (t->right->input != NULL)
      {
         printf("Ambiguous output redirect.\n");
         return failure_status;
      }

      /* create a pipe */
      if (pipe(pipe_fd) < 0)
      {
         perror("pipe");
         exit(-1);
      }

      /*fork(creating child process)*/
      if ((pid_1 = fork()) < 0)
      {
         perror("fork");
      }

      if (pid_1 == 0)
      { /*child 1  process code*/

         close(pipe_fd[0]); /*close read end since were not using it*/

         if (t->input != NULL)
         {

            if ((fd = open(t->input, O_RDONLY)) < 0)
            {
               perror("open");
               exit(-1);
            }

            /*make the pipes write end the new standard output*/
            dup2(pipe_fd[1], STDOUT_FILENO);

            /*pass in input file if exists and the pipes write end for the output file descriptor*/
            execute_aux(t->left, fd, pipe_fd[1]);

            /*closed pipe write end*/
            if (close(pipe_fd[1]) < 0)
            {
               perror("close");
               exit(-1);
            }

            /*close input file*/
            if (close(fd) < 0)
            {
               perror("close");
               exit(-1);
            }
         }
         /*if no input file was provided, use parent file descriptor as the input file*/
         else
         {
            dup2(pipe_fd[1], STDOUT_FILENO);
            execute_aux(t->left, p_input_fd, pipe_fd[1]); /*process the left subtree*/
            if (close(pipe_fd[1] < 0))
            {
               perror("close");
               exit(-1);
            }
         }
      }
      else
      {
         /*create second child to handle right subtree*/

         if ((pid_2 = fork()) < 0)
         {
            perror("fork");
            exit(-1);
         }

         if (pid_2 == 0)
         { /*child two code*/

            /*close write end of pipe, dont need it*/
            close(pipe_fd[1]);

            /*use output file if provided, else use parent output file descriptor*/
            if (t->output != NULL)
            {

               /*open provided output file if exists*/
               if ((fd = open(t->output, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
               {
                  perror("open");
                  exit(-1);
               }

               /*make the pipes read end the new standard input*/
               dup2(pipe_fd[0], STDIN_FILENO);
               execute_aux(t->right, pipe_fd[0], fd); /*process the right subtree*/

               /*close pipe read end*/
               if (close(pipe_fd[0]) < 0)
               {
                  perror("close");
                  exit(-1);
               }

               /*closed output file*/
               if (close(fd) < 0)
               {
                  perror("close");
                  exit(-1);
               }
            }

            else
            {
               dup2(pipe_fd[0], STDIN_FILENO);
               execute_aux(t->right, pipe_fd[0], p_output_fd); /*process the right subtree*/
            }
         }
         else
         {

            /* Parent has no need for the pipe */
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            /* Reaping each child */
            wait(&status);
            wait(&status);
         }
      }
   }
   /*if current conjuction enumerator is of type subshell, process commands
     in a child process and return success/failure status after executing*/
   else if (t->conjunction == SUBSHELL)
   {

      /*fork(creating child process)*/
      if ((new_pid = fork()) < 0)
      {
         perror("fork");
         exit(-1);
      }

      /*parent code*/
      if (new_pid != 0)
      {
         wait(&status);             /*wait for child*/
         exit(WEXITSTATUS(status)); /*return the status*/
      }
      else
      {
         /*child code*/

         /*get input from input file if it exists*/
         if (t->input != NULL)
         {
            /*open input file*/
            if ((fd = open(t->input, O_RDONLY)) < 0)
            {
               perror("fd");
               exit(-1);
            }
            /*changed standard input to use input from input file*/
            if (dup2(fd, STDIN_FILENO) < 0)
            {
               perror("dup2");
               exit(-1);
            }
            /*close file descriptor(input file)*/
            if (close(fd) < 0)
            {
               perror("fd");
               exit(-1);
            }
         }
         /*if there was no input file use provided parent file descriptor(the file parameter of the function)*/
         else
         {
            fd = p_input_fd;
         }

         /*use an output file if it exists*/
         if (t->output != NULL)
         {
            if ((fd2 = open(t->output, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
            {
               perror("fd");
               exit(-1);
            }

            /*change standard output to the output file(output will be written to output file)*/
            if (dup2(fd2, STDOUT_FILENO))
            {
               perror("dup2");
               exit(-1);
            }
            /*close the output file descriptor*/
            if (close(fd2) < 0)
            {
               perror("fd");
               exit(-1);
            }
         }
         /*if no outputfile exists write output to provided parent output file */
         else
         {
            fd2 = p_output_fd;
         }
         /*execute left subtree and get status*/
         status = execute_aux(t->left, fd, fd2);
         /*return with value of statujs(0: success, -1: failure*/
         exit(status);
      }
   }

   return success_status;
}
