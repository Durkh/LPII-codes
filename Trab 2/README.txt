+-+-+-+-+-+-+-+-+-+-+-+- INFORMAÇÕES ÚTEIS +-+-+-+-+-+-+-+-+-+-+-+-

	Valores podem ser modificados na seção "DEFINES", não recomendo mexer em outras seções.

	Tentei dar uma melhorada nos prompts pra deixar o mais legível, a sintaxe é essa:

*** parque ***
// carro //
-- passageiro --



+-+-+-+-+-+-+-+-+-+-+-+- NOTAS SOBRE O FUNCIONAMENTO DO PROGRAMA +-+-+-+-+-+-+-+-+-+-+-+-

	O layout do "parque" é mais ou menos esse:

fila geral		(indefinidas threads podem esperar aqui)
fila de embarque 	(N_SEATS threads esperam para entrar no carro)
carro			(N_SEATS threads andam no carro)

No melhor caso possível assim que o carro é liberado, todas as threads da fila de embarque entram no carro e mais threads da fila geral passam pra fila de embarque, apesar que sendo sincero, do jeito que implementei a fila de embarque fecha antes chegar perto de lotar.


Já as funções estão com esses papéis:

main 		= entrada do parque, ela quem é responsável pela entrada dos visitantes
ParkManager 	= controla a abertura e fechamento do parque
Passenger	= comportamento de um passageiro da montanha russa
Car		= carro da montanha russa

	Criei o ParkManager por causa de um problema, como a main é a responsável por crias as threads dos passageiros e ela só passa a checar o fechamento do parque após a criação dos passageiros, se colocar um número muito grande de passageiros a main vai ficar presa muito tempo no loop criando as threads (e eu ainda adicionei um sleep opcional), nisso, se colocar números pequenos nos outros parâmetros, como "viagens para o parque fechar", "número de assentos" e "tempo da viagem", o programa completa as viagens para o parque fechar msa a main ainda está no loop criando as threads dos passageiros, logo não tem como checar a variável do limite de viagens e o programa continua rodando quando deveria ter parado. Nisso a criação do ParkManager garante que mesmo com apenas 1 viagem e muitos passageiros o parque ainda é fechado.


	Além disso, quando o parque fecha, os visitantes que estão em fila não andam no brinquedo, só quem ficou no brinquedo, pois como o parque fechou os visitantes que estão passeando devem se retirar do parque e o carro apenas sai quando estiver cheio, portanto os que estão na fila não conseguem encher o carro e devem se retirar também.



+-+-+-+-+-+-+-+-+-+-+-+- NOTAS DE COMPILAÇÃO +-+-+-+-+-+-+-+-+-+-+-+-

comando de compilação:
gcc -pthread main.c -o main


C11 Standard
gcc version 10.2.0 (GCC)


