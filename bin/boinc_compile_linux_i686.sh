#compile BOINC with:

#_autosetup
#./configure --disable-client --disable-server LDFLAGS=-static-libgcc
#make

#ln -s `g++ -print-file-name=libstdc++.a`
#make



cd ../client/
g++ -m32 -DVERBOSE -DENABLE_CHECKPOINTING -DFALSE_ONLY -D_BOINC_ -D__STDC_LIMIT_MACROS -O2 -msse2 -m32 -ftree-vectorize -funroll-loops -Wall -static -static-libgcc -I/home/tdesell/Code/boinc -I/home/tdesell/Code/boinc/api -I/home/tdesell/Code/boinc/lib -I/home/tdesell/Code/boost_1_53_0 -I/home/tdesell/Code/gmp-5.1.2 subset_sum_main.cpp ../common/binary_output.cpp ../common/generate_subsets.cpp ../common/n_choose_k.cpp -o ../bin/SubsetSum_$1_i686-pc-linux-gnu -L/home/tdesell/Code/boinc/lib -L/home/tdesell/Code/boinc/api /usr/lib/gcc/i686-linux-gnu/4.7.3/libstdc++.a /home/tdesell/Code/boost_1_53_0/stage/lib/libboost_system.a /home/tdesell/Code/gmp-5.1.2/.libs/libgmp.a -lboinc_api -lboinc -pthread
cd ../bin/
