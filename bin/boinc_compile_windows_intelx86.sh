# COMPIEL BOINC WITH:
#_autosetup
#./configure --disable-client --disable-server LDFLAGS=-static-libgcc CFLAGS=-ftree-vectorize CXXFLAGS=-ftree-vectorize FFLAGS=-ftree-vectorize
#make
#ln -s `g++ -print-file-name=libstdc++.a`
#make



cd ../client/
g++ -DVERSION=$1 -O2 -m32 -msse3 -static -ftree-vectorize -funroll-loops -static-libgcc -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DFALSE_ONLY -D_BOINC_ -D__STDC_LIMIT_MACROS -I../../boost_1_53_0 -I../../boinc_repo -I../../boinc_repo/api -I../../boinc_repo/lib subset_sum_main.cpp ../common/binary_output.cpp ../common/n_choose_k.cpp ../common/generate_subsets.cpp -o ../bin/SubsetSum_$1_windows_intelx86.exe -L../../boinc_repo/api -L../../boinc_repo/lib -lboinc_api -lboinc -lgmp -lpsapi
