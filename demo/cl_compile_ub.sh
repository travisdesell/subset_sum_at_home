#g++ -Wall -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_COLOR -DSHOW_SUM_CALCULATION-DSHOW_SUM_CALCULATION  -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -DVERBOSE -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DENABLE_CHECKPOINTING -DVERBOSE -DENABLE_COLOR -DFALSE_ONLY -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp ../common/binary_output.cpp -o ../bin/subset_sum -I/opt/local/include -lboost
g++ -DENABLE_COLOR -Wall  -O3 -msse3 -funroll-loops -ftree-vectorize -I/usr/include/CL/ -L/usr/lib/x86_64-linux-gnu subset_sum_main.cpp opencl_bit_logic.cpp ../common/binary_output.cpp ../common/n_choose_k.cpp  ../common/generate_subsets.cpp -o ./subset_sum -std=c++11 -lpthread -lOpenCL -Wno-deprecated-declarations
#g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -DFALSE_ONLY -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
