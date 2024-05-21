# Writing-A-Unix-Shell
Building an own (non fully functional) shell 

A shell is a user interface that provides access to an operating system's services. Users, or clients, interact with the shell by issuing commands in the form of successive lines of text, commonly referred to as command lines. Essentially, the shell is another program provided by the operating system.

## Key Features of Shells

- **Interchangeable Programs:** 
  - The operating system allows you to install additional shells or even create your own.
  - Examples of popular shells include DOS, bash, ksh, csh, and Powershell.

- **Scripting and Programming Capabilities:** 
  - Many shells offer their own programming or scripting languages.
  - These languages come with unique syntax and operators to control command execution.
    - Common operators include:
      - `|` (pipe)
      - `>` (output redirector)
      - `<` (input redirector)
      - Conditional statements like `||` and `&&`

By understanding these features, users can effectively interact with the operating system through various shells and leverage their scripting capabilities for efficient command execution.

After compiling the code with the command `make all`, use the following command to evaluate each test script run:

```sh
./osh -t < testscripts/testscript.txt > & tmp ; diff tmp testscripts/ea.txt ;
```
## Approach

- **Phase 1 - input parsing**
  - read the input line.
  - tokenize input string to command, arguments and operators.
  - optional store it in a format to make it easy to handle input.
  - identify malformed commands.

- **Identifying malformed commands**
  - NULL Command 
  eg: `osh> | ls` 
  this is wrong as we have a null command before the `|` 
  - Missing Files for redirectors
  eg: `osh> ls >` 
  the output redirector is expecting a file, but it is null in command 
  - Multiple Redirectors 
  eg: `osh> ls > file | cat` 
  in this case, we have two output directors that can be associated to ls. This is ambiguous
  and thus malformed.

- **Phase 2 - Implement handling of logical operators `-` `&&` `||` `;`**
  - To implement logical operators
    1. Check to see if current and previous commands separated by logical operator.
    2. If so, get exit status of the previous command.
    3. Based on exit status, determine whether to execute current command.
    4. If you need to skip the current command set the exit status of the current command to be the same as the previous command. 
  - This logic is sufficient to handle chains of operators.
 
- **Phase 3 - Implement a program switch for testing (`-t`)** 
  - Should not print the “osh>” prompt
  - This switch allows us to input an entire file of commands for testing like:
  ```sh
  osh -t < 1.singleCommand.txt > & tmp ; diff tmp testscripts/ea1.txt ;
  ```
- **Phase 4 - Implement interprocess communication with pipes `|` to complete the shell**
  - Allows us to directly pass output from one command into another rather than using a file and redirection. 
  - e.g., `osh> ls | grep main`
  - We use pipes to accomplish this IPC. There is a system call, `pipe()`, that can be used:
    `pipe() - http://man7.org/linux/man-pages/man2/pipe.2.html`

   - “pipes” are either bidirectional or unidirectional data channels, used for IPC. For this project we assume pipes are unidirectional and only pass information in one direction. 
   - For this project. using pipes boils down to 3 steps: 
     - create the pipe. 
     - connect `stdout` of the first command (e.g., `ls`) to one end of the pipe. 
     - connect stdin of the next command (e.g., `grep main`) to the other end of the pipe.
   - The challenge here is to get the timing of the connections correct. The process image is copied upon a call to fork(), so to pass the file descriptors of the pipe along to the child, the pipe needs to be created before the fork. Once this is done, connect the pipes at the beginning of the loop (after `fork()` and before calling `exec()`). We can generalize this as follows:
     - If the current command is connected to the next command by a pipe:
       - create a pipe.
       - store the pipe handles at a temp location.
       - connect stdout of the current process to the pipe.
     - If the current command is connected to the previous command by a pipe:
       - connect the stdin of the current command to the pipe created earlier.
       


