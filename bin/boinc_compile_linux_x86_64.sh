cd ../client/
g++ -m64 -O3 -msse3 -ftree-vectorize -funroll-loops -Wall -DENABLE_CHECKPOINTING -D_BOINC_ -I/boinc -I/boinc/api -I/boinc/lib subset_sum_main.cpp ../common/output.cpp -o ../bin/SubsetSum_$1_x86_64-pc-linux-gnu -L/boinc/lib -L/boinc/api -lboinc_api -lboinc -pthread
cd ../bin/
