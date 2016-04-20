cd ../client/
#g++ -Wall -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_COLOR -DSHOW_SUM_CALCULATION-DSHOW_SUM_CALCULATION  -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -DVERBOSE -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DENABLE_CHECKPOINTING -DVERBOSE -DENABLE_COLOR -DFALSE_ONLY -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp ../common/binary_output.cpp -o ../bin/subset_sum -I/opt/local/include -lboost
g++ -Wall -DVERBOSE -DENABLE_COLOR -DFALSE_ONLY -D_OpenCl_ -O3 -msse3 -funroll-loops -ftree-vectorize -I/usr/local/cuda/include/ -L/usr/lib64/nvidia/ subset_sum_main.cpp ../common/binary_output.cpp ../common/n_choose_k.cpp  ../common/generate_subsets.cpp -o ../bin/subset_sum -lOpenCL
#g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -DFALSE_ONLY -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
cd ../bin/
