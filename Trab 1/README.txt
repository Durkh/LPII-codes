Aluno: Egídio Neto Alves de Araújo

Compilação:

    gcc main.c -o main

Execução:

    ./main


Observações:

    Utilizei a função GastarProcessamento retirada dos exemplos assim como foi pedido no enunciado, porém a função para de executar bem antes do processo pai matar o processo filho, que fica com zumbi, fiquei na dúvida se deveria aumentar o GastarProcessamento mas mantive do mesmo jeito que está nos exemplos, como diz o enunciado.
    
    Também utilizei SIGTERM em vez do SIGHUP utilizado no exemplo porque SIGTERM é o sinal padrão do UNIX para terminar processos e pareceu o certo a se usar nesse caso, já que SIGHUP tem um significado atual mais ambíguo para processos e daemons
    
    gcc version 10.2.0 (GCC)
