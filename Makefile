#---------------------------------------------------------------------------

all: clean zlog Gateway.out

Gateway.out:
	cd src && make clean
	cd src && make all
zlog:
	cd zlog-latest-stable && make clean
	cd zlog-latest-stable && make
	cd zlog-latest-stable && sudo make install
	sudo ldconfig
clean:
	cd src && make clean
	cd zlog-latest-stable && make clean
