#! /bin/sh
rm -r include
mkdir include
cd include

ln -s ../cudd/cudd.h .
ln -s ../cudd/cuddInt.h .
ln -s ../epd/epd.h .
ln -s ../dddmp/dddmp.h .
ln -s ../mtr/mtr.h .
ln -s ../obj/cuddObj.hh .
ln -s ../st/st.h .
ln -s ../util/util.h .
ln -s ../mnemosyne/mnemosyne.h .