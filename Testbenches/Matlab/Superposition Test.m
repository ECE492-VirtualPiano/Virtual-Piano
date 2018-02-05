 % Test for sounds playing at same time

 % Compute Generic Sin waves of each frequency
 clear all
 Fs=8000;
 Ts=1/Fs;
 t=[0:Ts:1];
 F_A=440; %Frequency of note A is 440 Hz
 A=sin(2*pi*F_A*t);
 
 F_B=750; %Frequency of note B is 750 Hz
 B=sin(2*pi*F_B*t);
 
 % Superposition waves
 C=(A+B)/2;
 
 % Play one note
 sound(A,Fs);
 pause(2);
 
 % Play another note
 sound(B,Fs);
 pause(2);
 
 % Play both ntoes at same time
 sound(A,Fs);
 sound(B,Fs);
 
 pause(2);
 
 % Play superposition of notes and determine if the same
 sound(C,Fs);
