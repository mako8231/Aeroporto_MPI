## Aeroporto 

Trabalho de Sistema distribuídos: um projeto de controle de decolagem e pouso usando a biblioteca **openMPI**.

## Critério de Arquivos:
O sistema de arquivos que carregam a configuração de cada aeroporto baseia-se:

| Código | 1 |   |   |   |
|--------|---|---|---|---|
|Pousos:        |3   |Decolagens:   |4   |   
|Pousos        |Origem   |Horário de Chegada   |Tempo de Voo   |   
|   21     | 2  | 2  | 2  |   
|   35     | 3  | 7  | 6  |   
|   23     |  2 | 5  | 4  |
|Decolagens        |Origem   |Horário de Partida|Tempo de Voo   |
|   11     | 2  | 0  | 3  |   
|   12     | 3  | 1  | 4  |   
|   13     |  2 | 3  | 1  |
|   14     |  4 | 4  | 2  |

A formatação para um arquivo de configuração do aeroporto acima deve ser:
### Sistema de Arquivos: 
```
\projeto
   aeroporto1.txt
   aeroporto2.txt
   aeroporto3.txt
```

### Conteúdo do Arquivo:
```
<Código> <Número de pousos> <Número de decolagens>
<Tempo de pouso> <Aeroporto de origem> <Horário de Chegada> <Tempo de voo>
<Tempo de pouso> <Aeroporto de da origem> <Horário de Chegada> <Tempo de voo>
<Tempo de pouso> <Aeroporto de da origem> <Horário de Chegada> <Tempo de voo>
.
.
.

<Tempo de decolagem> <Aeroporto de destino> <Horário de Partida> <Tempo de voo>
<Tempo de decolagem> <Aeroporto de destino> <Horário de Partida> <Tempo de voo>
<Tempo de decolagem> <Aeroporto de destino> <Horário de Partida> <Tempo de voo>
<Tempo de decolagem> <Aeroporto de destino> <Horário de Partida> <Tempo de voo>
.
.
.
```

Portanto, o conteúdo do arquivo seria:

```
1 3 4
21 2 2 2
35 3 7 6 
23 2 5 4
11 2 0 3
12 3 1 4
13 2 3 1
14 4 4 1
```