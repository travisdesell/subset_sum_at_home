cd ../client/
#g++ -Wall -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_COLOR -DSHOW_SUM_CALCULATION-DSHOW_SUM_CALCULATION  -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -DVERBOSE -DFALSE_ONLY -DSHOW_SUM_CALCULATION -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
#g++ -Wall -DVERBOSE -DHTML_OUTPUT -DENABLE_CHECKPOINTING -DENABLE_COLOR -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
g++ -Wall -DVERBOSE -DENABLE_CHECKPOINTING -DENABLE_COLOR -DFALSE_ONLY -O3 -msse3 -funroll-loops -ftree-vectorize subset_sum_main.cpp -o ../bin/subset_sum
cd ../bin/
