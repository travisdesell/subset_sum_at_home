BOINC_DIR = /boinc
MYSQL_DIR = /usr/include/mysql

CXX = g++
CXXFLAGS = -g -O -I$(BOINC_DIR) -I$(BOINC_DIR)/db -I$(BOINC_DIR)/api -I$(BOINC_DIR)/lib -I$(BOINC_DIR)/tools -I$(BOINC_DIR)/sched -I$(MYSQL_DIR) -DGMLE_BOINC -I/usr/local/include
LDFLAGS = -g -L$(BOINC_DIR)/db -L$(BOINC_DIR)/api -L$(BOINC_DIR)/lib -L$(BOINC_DIR)/tools -L$(BOINC_DIR)/sched -L/usr/X11R6/lib -L/usr/local/lib

LIBS = -lm -pthread -lsched -lboinc -lm -lboinc_crypt -lboinc_api
MYSQL = -L/usr/lib64/mysql -lmysqlclient


ASSIMILATOR_OBJS = \
    $(BOINC_DIR)/sched/assimilator.cpp \
    ./assimilation_policy.cpp

VALIDATOR_OBJS = \
    $(BOINC_DIR)/sched/validator.cpp \
	$(BOINC_DIR)/sched/validate_util.cpp \
	$(BOINC_DIR)/sched/validate_util2.cpp \
	$(BOINC_DIR)/sched/credit.cpp \
	$(BOINC_DIR)/sched/sched_util.cpp \
	$(BOINC_DIR)/sched/sched_config.cpp \
	$(BOINC_DIR)/sched/sched_limit.cpp \
	$(BOINC_DIR)/lib/parse.cpp \
	$(BOINC_DIR)/db/boinc_db.cpp \
	./sss_validation_policy.cpp \
	../undvc_common/file_io.cxx

START_SEARCH_OBJS = \
    ./start_search.cpp

make_work_OBJS = \
    ./make_work.cpp

work_generator_OBJS = \
    ./sss_work_generator.cpp \
	 $(BOINC_DIR)/sched/sched_util.cpp \
	 $(BOINC_DIR)/sched/sched_config.cpp \
	 $(BOINC_DIR)/sched/sched_limit.cpp \
	 $(BOINC_DIR)/lib/parse.cpp \
	 $(BOINC_DIR)/db/boinc_db.cpp 

PROGS = validator assimilator work_generator 

all: validator assimilator work_generator 

assimilator: $(ASSIMILATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -o assimilator $(ASSIMILATOR_OBJS) $(MYSQL) $(LIBS)

validator: $(VALIDATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -o sss_validator $(VALIDATOR_OBJS) $(MYSQL) $(LIBS)

work_generator: $(ASSIMILATOR_OBJ)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -o sss_work_generator $(work_generator_OBJS) $(MYSQL) $(LIBS)

.cxx.o:
	$(CXX) $(CXXFLAGS) $(CFLAGS) -Wall -Wextra -x c++ -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(CFLAGS) -Wall -Wextra -x c++ -c $< -o $@

.C.o:
	$(CXX) $(CXXFLAGS) $(CFLAGS) -Wall -x c++ -c $< -o $@

.c.o:
	$(CXX) $(CXXFLAGS) $(CFLAGS) -Wall -x c++ -c $< -o $@

clean:
	rm -f $(PROGS)
