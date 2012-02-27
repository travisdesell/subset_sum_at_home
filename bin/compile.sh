cd ../client/
#g++ -Wall -DVERBOSE -DENABLE_COLOR -O3 subset_sum_main.cpp -o ../bin/subset_sum
g++ -Wall -DVERBOSE -DENABLE_COLOR -DFALSE_ONLY -O3 subset_sum_main.cpp -o ../bin/subset_sum
cd ../bin/
