Banker¡¯s Algorithm

For this project, you will write a multithreaded program that implements the banker¡¯s algorithm discussed in class. Several customers request and release resources from the bank. The banker will grant a request only if it leaves the system in a safe state. A request that leaves the system in an unsafe state will be denied. This programming assignment combines three separate topics:
(1) multithreading, (2) preventing race conditions, and (3) deadlock avoidance.


Implementation

MAKE FILE by gcc: gcc BA.c -o BA

You should invoke your program by passing the number of resources of each type on the command line. For example, if there were four resource types, with ten instances of the first type, five of the second type, seven of the third type, and eight of the fourth type, you would invoke your program follows: (suppose your executable filename is pj5)

./pj5 10 5 7 8

