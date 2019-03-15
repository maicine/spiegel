spiegel: spiegel.c

test: spiegel
		./spiegel -test
		./test.sh

clean: 
		rm -f spiegel *.o *~ tmp*