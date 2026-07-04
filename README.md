
						Acordor Digital pentru Mandolină cu STM32


Autor: Spătan Laurențiu-Otniel
Specializarea: Ingineria Sistemelor (AIA)
Facultatea:Automatică și Calculatoare
Universitatea: Universitatea Politehnica Timișoara

1. Descrierea Livrabilelor:
Acest proiect conține implementarea software (limbaj C) pentru un acordor digital portabil destinat mandolinei. Algoritmul utilizează metoda Zero-Crossing Detection (ZCD) îmbunătățită cu histerezis adaptiv și un sistem de temporizare de înaltă rezoluție (DWT) integrat pe un microcontroler ARM Cortex-M3.
Adresa Repository-ului Git:


2. Pașii de Compilare ai Aplicației:
Proiectul a fost configurat inițial folosind STM32CubeMX și dezvoltat/compilat utilizând Visual Studio Code. Pentru compilarea codului sursă, urmați acești pași:
•	Asigurați-vă că aveți instalat Visual Studio Code împreună cu extensia oficială STM32 VS Code Extension (de la STMicroelectronics).
•	Deschideți Visual Studio Code.
•	Din meniul principal, selectați File -> Open Folder... și navigați către folderul descărcat/clonat din repository-ul Git.
•	La deschiderea folderului, extensia STM32 va recunoaște automat structura proiectului și fișierul CMake. Permiteți extensiei să configureze proiectul.
•	Navigați în meniul lateral din stânga la iconița extensiei STM32.
•	În panoul STM32, secțiunea Build, faceți click pe acțiunea Build (sau Clean and Build pentru o recompilare de la zero).
•	În terminalul din partea de jos a ecranului veți vedea procesul de compilare finalizându-se cu mesajul de generare a fișierelor executabile (ex: [100%] Built target...).

3. Pașii de Instalare și Lansare:
Pentru a lansa aplicația pe hardware-ul fizic, aveți nevoie de placa de dezvoltare STM32 Nucleo-F103RB, microfonul MAX4466 și ecranul LCD 1602 cu interfață I2C.

Conexiunea Hardware:
	•Conectați pinul OUT al microfonului la pinul PA0 al plăcii.
	•Conectați pinii SDA și SCL ai ecranului LCD la pinii PB9 (D14), respectiv PB8 (D15).
	•Asigurați alimentarea modulelor (3.3V pentru microfon, 5V pentru LCD) și conectați masele (GND) în comun.
	•Conectarea la PC: Conectați placa STM32 Nucleo la calculator utilizând un cablu USB.
Instalarea (Flashing): 
	În Visual Studio Code, având panoul lateral al extensiei STM32 deschis, mergeți la secțiunea Flash / Run și apăsați butonul Flash Device (sau Run din meniul Run and Debug). Extensia va încărca automat fișierul binar nou generat direct în memoria microcontrolerului.

Lansarea: 
	Odată ce scrierea s-a finalizat, placa se va restarta automat. Ecranul LCD se va aprinde, va afișa mesajul de pornire "Acordor Mandolină", urmat de starea de repaus "Astept sunet...". Sistemul este acum activ și gata de utilizare.
