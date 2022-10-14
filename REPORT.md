sshell
======
The first project for ECS150 was to implement a simplified version of a shell.
Authored by Hyunkyong Boo and Mac Lee.

Implementation
--------------
In order to provide a greater overview of how our shell was designed, we would
need to first talk about the Structs we designed to store commands and groups of
commands. We first implemented during the initial phases of our code in a way
that only creates one Struct for one command, since that is what we were working
with. However, during the piping phase, we realized that we likely required a 
wrapper of sorts to hold a logical group of commands. While we intially thought
to create an array to hold these structs, we realized very soon that there were
certain data pertinent to piping that needed to be considered to complete the
implementation for piping. Namely, we found it more palatable to store the 
argument count and the pipe count along with the array of Cmd structs; thus was
born the CmdStorage struct. Now to explain the Cmd struct in a slightly bit more
detail. One struct Cmd is supposed to be equivalent to a single command, that is
, a single part of a pipeline. Each Command struct contains informational
segments: parse related data and output redirection data. We created a
constructor/initializer and a destructor of sorts for the Command struct in
order to reduce the number of repetitive and cluttered lines that reduce the
clarity of our code.

Within our main function, we divided the code into two segments of (1) parsing
and (2) execution. The parsing of commands is mainly contained within the 
parse() function. The parse() function itself has 2 segements: (1)pipeline
parsing and (2) non-pipline parsing. The pipeline parsing section parses
the commands requried for piping. The non-pipeline parsing consists of several
sub-segments: (i) Output redirection parsing (ii) input redirection parsing,
(iii) path parsing [for cd], and (iv) general parsing. This function makes use
of recursion in order to reduce the amount of redundant code. We also made a 
parse_error_handle() function which handles most of the errors regarding the 
parsing of commands.

### Pipe
For the pipeline functions, main structure was to fork once more than the number 
of pipes. For instance, if there are two pipe symbols, or threee commands, 
the function pipeline must fork three times. Pipe() should be called one time 
less than the commands, or as the same number as the pipe symbol '|'. Therefore,
using for loop to call pipe function was necessary. This function pipeline() 
focuses on connecting right output of the prior command to the input of the next
command. Pid1 is the first pipeID given from the first forking. The reason for 
this part of the function to be not part of the loop is that it is the last to 
close everything after going through other children of children. It is important
for so-called parent process to wait for the status. This exitstatus needed to 
be stored in the array so that process of exitstatus is tracked down. Then, 
there is a for loop that goes around for 'count' number of times. This number is
the number of the pipes. It is clear that child has to connect to the current
filedescriptor(fd)'s stdin with the pipe. Then, the child has to connect to the
next filedescriptor's stdout with the pipe. This connects forked process. It 
is important to correctly connect and correctly close all the files. This same 
proces goes on for as many number of pipes there are. However, child process 
should not need to connect stdout to the next pipe, if it is the last one. 
Therefore, it does not go into the if nest when it is the last forked process.

### Extra Features
Implementing extra features used a lot of functions that already existed. Pushd
handles the direcotry, find whether it is valid, and change directory to that 
path. However, unlike the cd function, it stores the previous path to the stack.
The implementation of struct dirstack kept the stack. This stack was also used 
in function dirs and popd. Implementing popd was simple as all the popd needed
to do was change directory to one above the current direcotry. Then, it needed
to pop out what was the top-most memeber of the stack-array. Implementing dirs
was to simply print this stack and the current directory.

### Quirks and Failures
First and foremost, we would like to address the fact that, in some areas of the
code, the implementation logic may be evasive and may lack clarity. We tried to
ameliorate these shortcomings through comments; however, they too may be lacking
due to a lack of sufficient time (which is on us and our coding/debugging 
abilities). Also, the way we are allocating memory and freeing that memory is
inefficient in terms of line economy; that is there are way to many repetitions
of free() that can be avoided by allocating memory at a different part of the
code.

Testing
-------
During the initial phase of our testing (which we performed during the early
phases of the project), we referred to the project prompt and tried running the 
commands that were given as examples through our shell and compared the outputs
to check whether our program ran correctly. We also made use of the reference 
shell executable provided by the professor to compare its outputs to ours. After
the autograder was released on Gradescope, this became our go-to for testing our
code. Additionally, albeitto a smaller degree due to time constraints, we also 
walked through our codeto manually and systematically check for edge cases and 
other cases that may not have been provided by the autograder and the project 
prompt.

Sources
-------
For the most part, we referened the material provided by the professor. Code
snippets from the lecture slides were the foundation to our implementation
logic. Obviously, the Internet was also used as a resource, especially 
cplusplus.com where we mostly referred to the documentation for the C string
library required for parsing operations and more. Additionally, man7.org was 
also used to refer to the various syscall functions we were required to use to 
access the kernel of the operating system. For the more quirkier compile and 
runtime errors, we sometimes referenced forums much like Stack Overflow; we took
the professor's advice and merely referred to these sources at the highest
level. That is, if there were code snippets, we attempted to understand the code
and replicate the solution to our situation, which yielded mixed results but was
crucial to solving some of our more weirder or obscure errors.