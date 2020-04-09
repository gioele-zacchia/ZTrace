.PHONY: build install uninstall clean test
build:
	$(MAKE) -C build
install:
	cp build/libZTrace.so /usr/lib
	chmod 0755 /usr/lib/libZTrace.so
	ldconfig
	cp -r ZTR /usr/include
uninstall:
	rm /usr/lib/libZTrace.so
	ldconfig
	rm -r /usr/include/ZTR
clean:
	rm build/*.o
	rm build/libZTrace.so
	rm test
test:
	gcc -g test.c -o test -lZTrace