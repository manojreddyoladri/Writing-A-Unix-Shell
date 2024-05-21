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


