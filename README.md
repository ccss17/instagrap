# instagrap

### Overview
  - student submit C source via client program
  - server builds and tests given C source with test cases
  - server sends the result back to student

### Components
  - Program Components
    - master: __instagrapd__
    - sandboxed worker: __worker__
    - client: __submitter__
    - [submitter] <--> [instagrapd] <--> [worker]
  - Users
    - admin
      - launches __worker__ process
      - launches __instagrapd__ process with test cases
      - there is only one admin in whole system
    - students
      - runs __submitter__ to turn C source in instagrapd
      - then, __submitter__ returns result message from the server
      - there are multiple students eash of who has a unique __student ID__ 

### Workflow
  1. __submitter__ requests to __instagrapd__ for evaluating C source with student info
  2. __instagrapd__ delivers the C source with test input to __worker__ 
  3. then, __worker__ builds and runs the C source with given input, and returns back the output
    - repeat this step for each test case
  4. __submitter__ asks __instagrapd__ whether the result is ready.
    - If it's ready, __instagrapd__ sends results back to __submitter__

### Interface

##### `./submitter -n <IP>:<PORT> -u <ID> -k <PW> <FILE>`
- [IP]   IP of instagrapd
- [PORT] PORT of instagrapd
- [ID]   Student ID (8 digit number) (e.g., 21400802)
- [PW]   password (8 digit alphanumeric string) (e.g.,abcd1234)
- [FILE] C source
- Behaviors
  - connects to __instagrapd__ in TCP
  - requests for evaluating a C source to __instagrapd__
  - frequently connects with __instagrapd__ to receive the results
  - once received, print result on stdout

##### `./intagrapd -p <PORT> -w <IP>:<WPORT> <DIR>`
- [PORT] PORT for listening
- [IP]   IP address for listening
- [WPORT]PORT of worker
- [DIR] path to testcase directory
- Testcase
  - testcase directory contains 20 files whose name are 1.in, 1.out, 2.in, 2.out, ..., 10.in, and 10.out
  - __n.in__ is a test input and __n.out__ is the expected output
- Behaviors
  - listens at a port
  - requests worker to run the C source with test input
  - rejects connection if it give wrong password __(different from the one that is given at the submission)__
  - Once all testing is done, instagrapd return number of test cases that C source passes to __submitter__; or, send back the build failure message
  - __instagrapd__ should be able to communication with multiple __submitter__ concurrently
  - __instagrapd__ accepts any requests from __submitter__ with a new Student ID and a new password

##### `./worker -p <PORT>`
- [PORT]  listening port
- Behaviors
  - receives a pair of C source and test input file
  - builds C source. If fails, it should send the build failure message back to intagrapd
    - target program always receives input from stdin, and print out the result to stdout
  - If target program execution time exceeds 3 secs, __worker__ stops program and return timeout message to instagrapd
  - __worker__ should clean up the C source and binary files after each test
- NOTE
  - __worker__ is intended to run in different account/machine

##### Other Requirements
- communication between processes must be implemented either with pipe or socket
  - DO NOT USE FILE
- give __Makefile__ which compile properly the three modules __instagrapd__, __worker__, and __submitter__
- show your implementations fulfill all requirements by __youtube video__
  - cover various cases
- describe your design and implementation in __write-up__
  - explain your protocols on network communication
  - explain what you used techniques to solve different requirements
  - up to 6 pages
  - include __YouTube video URL__
- discuss problems/issues/limitations of __InstaGrap system__ as an automated grading system in write-up
- TA will evaluate your __InstaGrap__ at __Ubuntu 16.04.6 kernel 4.15.0__
