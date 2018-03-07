%{
*********************************************************************************************************
*
*                                MATLAB PLAYING A SONG PROTOTYPE CODE
*
*                                         CYCLONE V SOC
*
* Filename      : Synthesizer_Twinkle_Twinkle_Little_Start_Test.m
* Version       : V1.00
* Creation 		: January 28, 2018
* Created by 	: Kevin Wong (kwong4@ualberta.ca), Mingjun Zhao (zhao2@ualberta.ca), Daniel Tran (dtran3@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This file is an example of what a simple song would sound like by using the same 
                  frequencies as those for a piano. The sounds are created as sine waves and played with 
                  a delay of 1 second between each note. This file provides a basis for a wavetable 
                  synthesizer, if the need for one were to arrise. It shows how a sound can be created 
                  using a simple sine wave at a certain frequency. An implementation of this code would 
                  be similar, but would require stepping through the wave (in this, the sound function 
                  is used instead)
*********************************************************************************************************
%} 

% Frequency to Tone
% A - 440Hz
% B - 493.88Hz
% C - 523.25Hz
% D - 587.37Hz
% E - 659.26Hz
% F - 698.46Hz
% G - 783.99Hz

clear all

% Compute Generic Sin waves of each frequency
Fs=8000;
Ts=1/Fs;
t=[0:Ts:1];
F_A=440; %Frequency of note A is 440 Hz
F_B=493.88; %Frequency of note B is 493.88 Hz
F_C=523.25; %Frequency of note C is 523.25 Hz
F_D=587.38; %Frequency of note D is 587.37 Hz
F_E=659.26; %Frequency of note E is 659.26 Hz
F_F=698.46; %Frequency of note F is 698.46 Hz
F_G=783.99; %Frequency of note G is 783.99 Hz
A=sin(2*pi*F_A*t);
B=sin(2*pi*F_B*t);
C=sin(2*pi*F_C*t);
D=sin(2*pi*F_D*t);
E=sin(2*pi*F_E*t);
F=sin(2*pi*F_F*t);
G=sin(2*pi*F_G*t);

% Play sound - Twinkle Twinkle Little Star
sound(C,Fs);
pause(1);
sound(C,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(A,Fs);
pause(1);
sound(A,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(D,Fs);
pause(1);
sound(D,Fs);
pause(1);
sound(C,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(D,Fs);
pause(1);
sound(C,Fs);
pause(1);
sound(C,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(A,Fs);
pause(1);
sound(A,Fs);
pause(1);
sound(G,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(F,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(E,Fs);
pause(1);
sound(D,Fs);
pause(1);
sound(D,Fs);
pause(1);
sound(C,Fs);
pause(1);