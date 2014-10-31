all: benchmark send receive hrav_protocol send_file
send_file: send_file.o hrav_protocol.o
	gcc -o send_file send_file.o hrav_protocol.o -lm -lpthread
send:	send.o hrav_protocol.o
	gcc -o send send.o hrav_protocol.o
receive:	receive.o
	gcc -o receive receive.o
send_file.o: send_file.c
	gcc -c send_file.c
send.o: send.c
	gcc -c send.c
hrav_protocol.o: hrav_protocol.c
	gcc -c hrav_protocol.c
receive.o: receive.c
	gcc -c receive.c
benchmark: benchmark.o serial.o
	gcc -o benchmark benchmark.o serial.o -lm -lpthread
benchmark.o: benchmark.c
	gcc -c benchmark.c
serial.o: serial.c
	gcc -c serial.c
clean:
	rm -rf benchmark send receive test *.o *~
