SRC = src
OBJ = obj
BIN = bin

PROJECTS = $(patsubst $(SRC)/%, %, $(wildcard $(SRC)/*))

CC = mpicc
CFLAGS = -std=c1x -fopenmp

FC = gfortran
FFLAGS = 

NP ?= 4
EXEC = mpiexec
EXECFLAGS = -np $(NP)

default:
	@echo "Usage: make <PROJECT>"
	@echo "Projects: $(PROJECTS)"

all: $(PROJECTS)

$(BIN):
	@mkdir -p $@

clean:
	rm -rf $(OBJ)
	rm -rf $(BIN)

define PROJECT_template

SOURCES-$1 = $$(wildcard $$(SRC)/$1/*.c) $$(wildcard $$(SRC)/$1/*.f)
OBJECTS-$1 = $$(patsubst $$(SRC)/$1/%.c,$$(OBJ)/$1/%.o,$$(patsubst $$(SRC)/$1/%.f,$$(OBJ)/$1/%.o,$$(SOURCES-$1)))

$1: $$(BIN)/$1

mpirun-$1: $$(BIN)/$1
	@echo "\n\n===== RUNNING APPLICATION WITH MPI =====\n"
	$$(EXEC) $$(EXECFLAGS) $$<

run-$1: $$(BIN)/$1
	@echo "\n\n===== RUNNING APPLICATION =====\n"
	@$$< $$(FARGS)

$$(OBJ)/$1/%.o: $$(SRC)/$1/%.c | $$(OBJ)/$1
	$$(CC) $$(CFLAGS) -c $$< -o $$@

$$(OBJ)/$1/%.o: $$(SRC)/$1/%.f | $$(OBJ)/$1
	$$(FC) $$(FFLAGS) -c $$< -o $$@

$$(OBJ)/$1:
	@mkdir -p $$@

$$(BIN)/$1: $$(OBJECTS-$1) | $$(BIN)
	$$(CC) $$(OBJECTS-$1) -Wall -fopenmp -o $$@

endef

$(foreach p, $(PROJECTS), \
	$(eval $(call PROJECT_template,$(p)))\
)

.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs