# RUSH

  Rush (Rapid Unix SHell) is a simple, efficient command-line interpreter designed as part of an academic project. It aims to provide fundamental shell functionalities, including command execution, built-in command handling, input/output redirection, and parallel command processing


# Features

* **Command Execution:** Rush supports executing commands by creating a child process for each command, excluding built-ins.
* **Built-in Commands:** Implements exit, cd, and path as internal commands.
* **Input/Output Redirection:** Enables redirecting command output to files using the > symbol.
* **Parallel Commands:** Allows executing multiple commands in parallel, separated by &.
* **Error Handling:** Utilizes a uniform error message strategy for command execution failures.
* **Shell Path Management:** Manages a shell path variable to search for executable files in specified directories.

# Installation
Clone this repository and compile rush.c using GCC or any compatible C compiler:

$ git clone https://github.com/rfeinberg3/RUSH-LinuxShell.git

$ cd rush-shell

$ gcc rush.c -o rush
