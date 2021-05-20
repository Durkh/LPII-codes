valores podem ser modificados na seção "DEFINES"



o layout do "parque" é mais ou menos esse

fila geral		(indefinidas threads podem esperar aqui)
fila de embarque 	(N_SEATS threads esperam para entrar no carro)
carro			(N_SEATS threads andam no carro)

no melhor caso possível assim que o carro é liberado, todas as threads da fila de embarque entram no carro e mais threads da fila geral passam pra fila de embarque, apesar que sendo sincero, do jeito que implementei a fila de embarque fecha antes chegar perto de lotar



tentei dar uma melhorada nos prompts pra deixar o mais legível, a sintaxe é essa:

*** parque ***
// carro //
-- passageiro --




gcc version 10.2.0 (GCC)

comando de compilação:
gcc -pthread main.c -o main
