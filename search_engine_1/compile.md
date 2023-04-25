##

## Compile the programs using the GNU C++ compiler (i.e., g++)

##

## Compile into object codes

g++ stem.cpp -c -w
g++ IInvFile.cpp -c -w
g++ BuildInvFile.cpp -c -w
g++ DocLen.cpp -c -w
g++ Retrieval.cpp -c -w

## Compile executables

g++ -o BuildInvFile BuildInvFile.o IInvFile.o stem.o -w
g++ -o DocLen DocLen.o IInvFile.o stem.o -w
g++ -o Retrieval Retrieval.o IInvFile.o stem.o -w

## Debug mode

g++ stem.cpp -c -w -g
g++ IInvFile.cpp -c -w -g
g++ BuildInvFile.cpp -c -w -g
g++ DocLen.cpp -c -w -g
g++ Retrieval.cpp -c -w -g

g++ -o BuildInvFile BuildInvFile.o IInvFile.o stem.o -w -g
g++ -o DocLen DocLen.o IInvFile.o stem.o -w -g
g++ -o Retrieval Retrieval.o IInvFile.o stem.o -w -g
