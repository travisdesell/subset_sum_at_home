cd ../client/
g++ -DVERBOSE -DFALSE_ONLY -D_BOINC_ -O2 -msse2 -m32 -ftree-vectorize -funroll-loops -Wall -export-dynamic -I/home/deselt/Software/boinc -I/home/deselt/Software/boinc/api -I/home/deselt/Software/boinc/lib subset_sum_main.cpp -o ../bin/SubsetSum_$1_i686-pc-linux-gnu -L/home/deselt/Software/boinc/lib -L/home/deselt/Software/boinc/api -lboinc_api -lboinc -pthread
cd ../bin/
