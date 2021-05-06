
filter: filter.o 
	$(CC) -o filter $?
	
clean:
	rm *.o output.ppm
