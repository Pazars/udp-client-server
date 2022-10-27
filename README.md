# udp-client-server

## Goal and requirements
The goal was to write a program that communicates between client and server using the UDP protocol.

The requirements were:
 1) Use e-poll both on the server, as well as client side
 2) Sent file should preserve its name
 3) Client should see the file transfer speed
 4) Server should be able to handle multiple clients simultaneously

## Usage
Server and client are split into separate programs
### Compiling

Server

    gcc -server.c -o server.out

Client

    gcc -client.c -o client.out

### Running

Server

    ./server.out <PORT>

Received files are stored in `./receive` directory

Client

    ./client.out <PORT> <FILE_NAME>
for individual file, or

    bash run_multiple_clients.sh

for sending 2 provided example text files.

The file should be a text file, be in the `./` directory, and passed to the program just as the name (not full path)

## Implementation and design

For now not all requirements are completely met.

 1) The program is slightly simplified by only communicating in one direction - client sends files to the server.
 Hence, there implementing e-poll on the client side was not a priority.

 2) Files preserve their names. Also when sent across multiple packets and in-between other files.
 This is done by reserving first 3 bytes for the file name length followed by the file name.

 3) The `sendto()` command is timed on the client's side, and that is displayed as the transfer speed.
 To avoid cluttering `stdout` with tons of messages, the previous one is re-written.

 4) Use of e-poll as well as the strategy for figuring out to which file data belongs allows handling multiple clients.

 ## Bugs and to-do

 - Implement two-way communication
 - There is an issue with the last character in the buffer
 - Only text files can be sent in multiple packets
 - Improve transfer speed display
 - Generalize hard-coded values as constants in `consts.h`
 - Checksum for packet verification