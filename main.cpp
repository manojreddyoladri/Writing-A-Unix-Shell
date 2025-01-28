#include <iostream>
#include <string>
#include <vector>
#include "command.hpp"
#include "parser.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;

#define MAX_ALLOWED_LINES 25

const char** f1(shell_command cmd){
    const char *command = cmd.cmd.c_str();
    const char **argv = new const char* [cmd.args.size()+2];
    argv[0] = command;
    if(cmd.args.size()>0){
        for (int i = 0;  i < (signed) cmd.args.size() + 1;  i++){
	        argv[i + 1] = cmd.args[i].c_str();
        }
    }
    argv[cmd.args.size()+1] = NULL;
    return argv;
}

void f2(std::vector<shell_command> shell_commands) {
    int prev_pipe[2] = {-1, -1}; // Store previous pipe
    int curr_pipe[2]; // Store current pipe

    bool flag = true;
    
    for(int i = 0; i < shell_commands.size(); i++){
        
        /* fork a child process*/
        pid_t pid;
        int status = 0;
        
        // Create pipe for current command if not the last one
        if(i < shell_commands.size() - 1 && pipe(curr_pipe) < 0) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
        
        pid = fork();
        if(pid < 0){ /* error occurred*/
            perror("Fork Failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { /* child process */
            // Redirect input from previous pipe if available
            if (prev_pipe[0] != -1) {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            // Redirect output to current pipe if available
            if(i < shell_commands.size() - 1) {
                dup2(curr_pipe[1], STDOUT_FILENO);
                close(curr_pipe[0]);
                close(curr_pipe[1]);
            }

            // Close unused pipe ends
            if (prev_pipe[0] != -1) {
                close(prev_pipe[0]);
            }
            if (curr_pipe[0] != -1) {
                close(curr_pipe[0]);
            }
            if (curr_pipe[1] != -1) {
                close(curr_pipe[1]);
            }

            // Execute the command
            execvp(shell_commands[i].cmd.c_str(), (char**) f1(shell_commands[i]));
            // If execvp fails, print error
            perror("Exec Failed");
            exit(EXIT_FAILURE);
        }
        else { /*parent process*/
            // Close previous pipe if available
            if(prev_pipe[0] != -1) {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            // Store current pipe for next iteration if not the last command
            if(i < shell_commands.size() - 1) {
                prev_pipe[0] = curr_pipe[0];
                prev_pipe[1] = curr_pipe[1];
            }
            else { // Close the last pipe if it's the last command
                if(curr_pipe[0] != -1) {
                    close(curr_pipe[0]);
                }
                if(curr_pipe[1] != -1) {
                    close(curr_pipe[1]);
                }
            }

            // Wait for child to complete
            waitpid(pid, &status, 0);

            // Update flag based on command's next mode and exit status
            if (shell_commands[i].next_mode == next_command_mode::always) {
                flag = true;
            } else if (shell_commands[i].next_mode == next_command_mode::on_success) {
                flag = (WEXITSTATUS(status) == 0);
            } else if (shell_commands[i].next_mode == next_command_mode::on_fail) {
                flag = (WEXITSTATUS(status) != 0);
            }

            // Break the loop if the flag condition is met
            if ((shell_commands[i].next_mode == next_command_mode::on_success && !flag) ||
                (shell_commands[i].next_mode == next_command_mode::on_fail && flag)) {
                break;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    std::string input_line;

    for (int i=0;i<MAX_ALLOWED_LINES;i++) { // Limits the shell to MAX_ALLOWED_LINES
        // Print the prompt.
        /*if (argv[1] != std::string("-t")){
            std::cout << "osh>" << std::flush;
        }*/
        if(argc == 1){
            std::cout << "osh>" << std::flush;
        }
     
        // Read a single line.
        if (!std::getline(std::cin, input_line) || input_line == "exit") {
            break;
        }

        try {

            // Parse the input line.
            std::vector<shell_command> shell_commands
                    = parse_command_string(input_line);

            f2(shell_commands);
        }
        catch (const std::runtime_error& e) {
            std::cout << e.what() << "\n";
        }
    }

    //std::cout << std::endl;
    return 0;
}
