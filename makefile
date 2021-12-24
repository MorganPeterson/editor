NAME=tvi
CC=cc
FLAGS+=-O2 -std=c11 -pedantic -W -Wall -g

OS:=$(shell uname -s)
ifeq ($(OS),Linux)
	FLAGS+=-D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -lncursesw
endif

SRCD=src
BIND=bin
OBJD=obj

M=main
I=ioutils
S=strutils
B=buffer
W=window
K=keys
C=commands
D=display
U=utf
R=regex
H=search

MN=$(SRCD)/$(M).c
IU=$(SRCD)/$(I).c
SU=$(SRCD)/$(S).c
BF=$(SRCD)/$(B).c
WN=$(SRCD)/$(W).c
KY=$(SRCD)/$(K).c
CM=$(SRCD)/$(C).c
DY=$(SRCD)/$(D).c
U8=$(SRCD)/$(U).c
RX=$(SRCD)/$(R).c
CH=$(SRCD)/$(H).c

IUO=$(OBJD)/$(I).o
SUO=$(OBJD)/$(S).o
BFO=$(OBJD)/$(B).o
WNO=$(OBJD)/$(W).o
KYO=$(OBJD)/$(K).o
CMO=$(OBJD)/$(C).o
DYO=$(OBJD)/$(D).o
U8O=$(OBJD)/$(U).o
RXO=$(OBJD)/$(R).o
CHO=$(OBJD)/$(H).o

OBJS=$(SUO) $(IUO) $(BFO) $(WNO) $(KYO) $(CMO) $(DYO) $(U8O) $(RXO) $(CHO)

.PHONY:all
all: $(OBJS) $(NAME)

.PHONY:$(I)
$(I):$(IUO)

.PHONY:$(S)
$(S):$(SUO)

.PHONY:$(B)
$(B):$(BFO)

.PHONY:$(W)
$(W):$(WNO)

.PHONY:$(K)
$(K):$(KYO)

.PHONY:$(C)
$(C):$(CMO)

.PHONY:$(D)
$(D):$(DYO)

.PHONY:$(U)
$(U):$(U8O)

.PHONY:$(R)
$(R):$(RXO)

.PHONY:$(H)
$(H):$(CHO)

.PHONY:$(NAME)
$(NAME):$(BIND)/$(NAME)

$(IUO):$(IU)
	@echo "building $(I)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(IU) -o $@

$(SUO):$(SU)
	@echo "building $(S)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(SU) -o $@

$(BFO):$(BF)
	@echo "building $(B)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(BF) -o $@

$(WNO):$(WN)
	@echo "building $(W)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(WN) -o $@

$(KYO):$(KY)
	@echo "building $(K)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(KY) -o $@

$(CMO):$(CM)
	@echo "building $(C)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(CM) -o $@

$(DYO):$(DY)
	@echo "building $(D)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(DY) -o $@

$(U8O):$(U8)
	@echo "building $(U)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(U8) -o $@

$(RXO):$(RX)
	@echo "building $(R)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(RX) -o $@

$(CHO):$(CH)
	@echo "building $(H)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(CH) -o $@

$(BIND)/$(NAME):$(MN)
	@echo "building $(NAME)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -o $@ $< $(OBJS)

clean:
	@echo "cleaning workspace"
	@rm -rf $(BIND)
	@rm -rf $(OBJD)
