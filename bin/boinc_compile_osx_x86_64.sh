cd ../client/
g++ -DVERBOSE -DENABLE_CHECKPOINTING -DFALSE_ONLY -D_BOINC_ -O3 -msse3 -funroll-loops -ftree-vectorize -Wall -I/Users/deselt/Software/boinc -I/Users/deselt/Software/boinc/api -I/Users/deselt/Software/boinc/lib -L/Users/deselt/Software/boinc/mac_build/build/Deployment -lboinc_api -lboinc subset_sum_main.cpp ../common/output.cpp -o ../bin/SubsetSum_$1_x86_64-apple-darwin
cd ../bin/
