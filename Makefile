# Andrew Cox
# 5/1/2024


CC = gcc

CFLAGS = -Wall -g

FILES = Simulation.c parser.c queue.c

DERIV = ${FILES:.c=.o}

DEPEND = $(DERIV)

all: Simulation

Simulation: $(DEPEND)
	$(CC) -o Simulation $(CFLAGS) $(DERIV)

# Dependencies
Simulation.o: Simulation.c Simulation.h parser.h queue.h
parser.o: parser.c parser.h
queue.o: queue.c queue.h

clean:
	rm -f $(DERIV) Simulation
