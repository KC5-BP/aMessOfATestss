
EXE=cliClient
EXE_DBG=$(EXE).dbg
IP=127.0.0.1
PORT=5000

.PHONY: clean run run_dbg

all: $(EXE)

$(EXE): socketClient.c
	gcc -Wall -o $@ $<

run: $(EXE)
	./$< $(IP) $(PORT)

$(EXE_DBG): socketClient.c
	gcc -Wall -DENA_DBG=1 -o $@ $<

run_dbg: $(EXE_DBG)
	./$< $(IP) $(PORT)

clean:
	rm -rf $(EXE) $(EXE_DBG)

