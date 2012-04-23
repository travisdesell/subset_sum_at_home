cd ../client/
g++ -DVERBOSE -DENABLE_CHECKPOINTING -DFALSE_ONLY -D_BOINC_ -O2 -msse2 -m32 -ftree-vectorize -funroll-loops -Wall -export-dynamic -I/home/tdesell/Software/boinc -I/home/tdesell/Software/boinc/api -I/home/tdesell/Software/boinc/lib subset_sum_main.cpp ../common/output.cpp -o ../bin/SubsetSum_$1_i686-pc-linux-gnu -L/home/tdesell/Software/boinc/lib -L/home/tdesell/Software/boinc/api -lboinc_api -lboinc -pthread
cd ../bin/
