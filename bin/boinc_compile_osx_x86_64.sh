cd ../client/
g++ -DVERBOSE -DENABLE_CHECKPOINTING -DFALSE_ONLY -D_BOINC_ -D__STDC_LIMIT_MACROS -O3 -msse3 -funroll-loops -ftree-vectorize -Wall -I/Users/deselt/Documents/Dropbox/software/boinc -I/Users/deselt/Documents/Dropbox/software/boinc/api -I/Users/deselt/Documents/Dropbox/software/boinc/lib -I/opt/local/include -L/Users/deselt/Documents/Dropbox/software/boinc/mac_build/build/Deployment -L/opt/local/lib -lboinc_api -lboinc subset_sum_main.cpp ../common/binary_output.cpp -o ../bin/SubsetSum_$1_x86_64-apple-darwin /opt/local/lib/libboost_*.a /opt/local/lib/libgmp.a
cd ../bin/
