# Covid-19-Reservation-System


* This project was completed on Mac M1 and tested on Amazon AWS cloud EC2 instances.

What I have done in the assignment:
Building TCP and UDP connections between client, hospitals and scheduler. Assigning client to a hospital with highest score and shortest distance.

What my code files are?

scheduler.cpp: Acting as a TCP server of client(student) to send and receive data. At the same time, communicating with three hospitals via UDP protocol to acquire hospitals' scores and distances. Then,  the scheduler will make a decision on designation of client according to scores and send final result to client. There are 1 TCP socket and 1 UDP sockets created in this file.

client.cpp: As a TCP client, sending student's location to scheduler through TCP protocol and receiving the designated hospital information from scheduler after calculation. There is 1 TCP socket created in this file.

hospitalA: Hospital A is responsible to send its location, total capacity, initial occupancy to scheduler and receive client location to calculate score and distance. There is 1 UDP socket created in this file.

hospitalB: Hospital B is responsible to send its location, total capacity, initial occupancy to scheduler and receive client location to calculate score and distance. There is 1 UDP socket created in this file.

hospitalC: Hospital C is responsible to send its location, total capacity, initial occupancy to scheduler and receive client location to calculate score and distance. There is 1 UDP socket created in this file.


The format of all the messages exchanged:
<hospital A | total capacity, initial occupancy, port number>
<hospital B | total capacity, initial occupancy, port number>
<hospital C | total capacity, initial occupancy, port number>
<client | location>
<hospital A | distance, score, port number>
<hospital B | distance, score, port number>
<hospital C | distance, score, port number>
<client | assigned hospital>
<hospital A/B/C | updated availability, occupancy>


The idiosyncrasy of the project:
Sometimes the command line will notify that the address is already used if there is a zombie process.


Reused code:

Source code: https://blog.csdn.net/Carizy/article/details/107280388
Purpose: Using Dijkstra algorithm calculate the shortest path from the client to each hospital.

Source code: Beej's Guide to Soket Programming
Purpose: To establish TCP and UDP initialization and connections.

 
