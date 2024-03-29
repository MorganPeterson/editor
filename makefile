NAME=mpe
CC=cc
FLAGS+=-O2 -std=c99 -pedantic -W -Wall -g

OS:=$(shell uname -s)

LIBS=-lncursesw

ifeq ($(OS),Linux)
	FLAGS+=-D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700
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
P=complete
C=commands
D=display
U=utf
R=regex
H=search
N=undo
X=syntax
E=replace
T=themes

MN=$(SRCD)/$(M).c
IU=$(SRCD)/$(I).c
SU=$(SRCD)/$(S).c
BF=$(SRCD)/$(B).c
WN=$(SRCD)/$(W).c
KY=$(SRCD)/$(K).c
CP=$(SRCD)/$(P).c
CM=$(SRCD)/$(C).c
DY=$(SRCD)/$(D).c
U8=$(SRCD)/$(U).c
RX=$(SRCD)/$(R).c
CH=$(SRCD)/$(H).c
UN=$(SRCD)/$(N).c
SY=$(SRCD)/$(X).c
TM=$(SRCD)/$(T).c
RP=$(SRCD)/$(E).c

IUO=$(OBJD)/$(I).o
SUO=$(OBJD)/$(S).o
BFO=$(OBJD)/$(B).o
WNO=$(OBJD)/$(W).o
KYO=$(OBJD)/$(K).o
CPO=$(OBJD)/$(P).o
CMO=$(OBJD)/$(C).o
DYO=$(OBJD)/$(D).o
U8O=$(OBJD)/$(U).o
RXO=$(OBJD)/$(R).o
CHO=$(OBJD)/$(H).o
UNO=$(OBJD)/$(N).o
SYO=$(OBJD)/$(X).o
TMO=$(OBJD)/$(T).o
RPO=$(OBJD)/$(E).o

OBJS=$(SUO) $(IUO) $(BFO) $(WNO) $(KYO) $(CPO) $(CMO) $(DYO) $(U8O) $(RXO) $(CHO) $(RPO) $(UNO) $(SYO) $(TMO)

.PHONY:all $(I) $(S) $(B) $(W) $(K) $(P) $(C) $(D) $(U) $(R) $(H) $(E) $(N) $(X) $(T) $(NAME)

all: $(OBJS) $(NAME)
$(I):$(IUO)
$(S):$(SUO)
$(B):$(BFO)
$(W):$(WNO)
$(K):$(KYO)
$(P):$(CPO)
$(C):$(CMO)
$(D):$(DYO)
$(U):$(U8O)
$(R):$(RXO)
$(H):$(CHO)
$(N):$(UNO)
$(X):$(SYO)
$(T):$(TMO)
$(E):$(RPO)
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

$(CPO):$(CP)
	@echo "building $(P)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(CP) -o $@

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

$(RPO):$(RP)
	@echo "building $(E)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(RP) -o $@

$(UNO):$(UN)
	@echo "building $(N)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(UN) -o $@

$(SYO):$(SY)
	@echo "building $(X)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(SY) -o $@

$(TMO):$(TM)
	@echo "building $(T)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $(TM) -o $@

$(BIND)/$(NAME):$(MN)
	@echo "building $(NAME)"
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -o $@ $< $(OBJS) $(LIBS)

clean:
	@echo "cleaning workspace"
	@rm -rf $(BIND)
	@rm -rf $(OBJD)

install:
	@echo "installing editor"
	@cp bin/mpe /usr/local/bin/mpe
