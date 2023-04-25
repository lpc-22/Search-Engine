# comp4133--search-engine

## Search engine 1:
To run this program.
1. Put file.txt, post1.txt, and queryT in the folder
2. Compile the file with the following command
```
g++ stem.cpp -c -w
g++ IInvFile.cpp -c -w
g++ BuildInvFile.cpp -c -w
g++ DocLen.cpp -c -w
g++ Retrieval.cpp -c -w
g++ -o BuildInvFile BuildInvFile.o IInvFile.o stem.o -w
g++ -o DocLen DocLen.o IInvFile.o stem.o -w
g++ -o Retrieval Retrieval.o IInvFile.o stem.o -w
```
3. Run BuildInvFile.exe  
4. Run DocLen.exe
5. Run Retrieval.exe. Then input queryT and press enter

## Search engine 2:
