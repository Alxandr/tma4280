SRC = src
OBJ = obj
BIN = bin

PROJECTS = $(patsubst $(SRC)/%, %, $(wildcard $(SRC)/*))

CC = mpicc
CFLAGS =

EXEC = mpiexec
EXECFLAGS = -np 4

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

SOURCES-$1 = $$(wildcard $$(SRC)/$1/*.c)
OBJECTS-$1 = $$(patsubst $$(SRC)/$1/%.c,$$(OBJ)/$1/%.o,$$(SOURCES-$1))

$1: $$(BIN)/$1

mpirun-$1: $$(BIN)/$1
	@echo "\n\n===== RUNNING APPLICATION WITH MPI =====\n"
	$$(EXEC) $$(EXECFLAGS) $$<

run-$1: $$(BIN)/$1
	@echo "\n\n===== RUNNING APPLICATION =====\n"
	@$$<

$$(OBJ)/$1/%.o: $$(SRC)/$1/%.c | $$(OBJ)/$1
	$$(CC) $$(CFLAGS) -c $$< -o $$@

$$(OBJ)/$1:
	@mkdir -p $$@

$$(BIN)/$1: $$(OBJECTS-$1) | $$(BIN)
	$$(CC) $$(OBJECTS-$1) -Wall -o $$@

endef

$(foreach p, $(PROJECTS), \
	$(eval $(call PROJECT_template,$(p)))\
)

.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs