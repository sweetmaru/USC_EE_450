all:client.cpp hospitalA.cpp hospitalB.cpp hospitalC.cpp scheduler.cpp
	g++ scheduler.cpp -o scheduler
	g++ hospitalA.cpp -o hospitalA
	g++ hospitalB.cpp -o hospitalB
	g++ hospitalC.cpp -o hospitalC
	g++ client.cpp -o client

clean:
	rm  *.exe
