all: cudd minisat cmsat lingeling

cudd:
	cd ./cudd-2.5.0
	make all
	cd ./obj
	make all
	cd ../../

minisat:
	cd ./minisat
	# make config prefix=$(PREFIX)
	# make install
	cd ./core
	make libr
	cd ../../

cmsat:
	cd ./cmsat-2.9.9
	-mkdir build
	cd build
	../configure
	make
	cd ../../

lingeling:
	cd ./lingeling
	./configure.sh
	make
	cd ../



