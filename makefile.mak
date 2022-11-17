project: Simulador Monitor

cleanAndBuild: clean Simulador Monitor

Simulador: Simulador.c config.h
		gcc -c -g Simulador.c && gcc -g -o Simulador Simulador.o -lpthread

Monitor: Monitor.c config.h
		gcc -c -g Monitor.c && gcc -g -o Monitor Monitor.o -lpthread

clean:
		rm *.o simulador monitor 