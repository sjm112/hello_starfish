function []=ACCTEST()
% basic DNA data logging with primary measurement data, secondary control
% data and pixel images stored for analysis

clear all;
close all;
clc;
global meas_flag h_stop_meas InstrumentTypeEsplanade
%% settings
% port settings
port_prompt = {'Enter port number:','Enter baud rate:'};
portr_dlg_title = 'Port setting';
port_num_lines = 1;
port_def = {'13','115200'};
port_data = inputdlg(port_prompt,portr_dlg_title,port_num_lines,port_def);
if not(isempty(port_data));
    gcom=openGeoComPort(str2double(port_data{1}),str2double(port_data{2}));
else
    return
end
t_init=clock;
count=1;
count_1=0;
count_2=0;
%  x=00238,y=00364,z=-4048
diary off;
diary('e:\1.txt');
diary on;
while 1
res = fscanf(gcom);
if isempty(res)
    ;
else
%fprintf(res);
if res(1:1)=='x'
 if length(res)>=23
     start=3;
strx= res(start:start+4);start=start+8; x(count)=str2double(strx)*0.00025;
stry= res(start:start+4);start=start+8; y(count)=str2double(stry)*0.00025;
strz= res(start:start+4);start=start+12; z(count)=str2double(strz)*0.00025;
strangle=res(start:start+4);start=start+9; zangle(count)=str2double(strangle);
cri=sqrt((x(count))^2+(z(count))^2+(y(count))^2);
if cri>0.9
    if cri<1.1
 %%fprintf('cri:%d\r\n',cri);
%AAa(count)=atan( (x(count))^2+(z(count))^2/x(count))*180/pi;
%AAb(count)=atan( (x(count))^2+(z(count))^2/y(count))*180/pi;
%AAc(count)=acos(z(count))*180/pi;
temp_time(count)=etime(clock,t_init);
pause(0.001);
cla;
% h(1)=subplot(2,2,1);
% plot(temp_time,x,'-r',temp_time,y,'-.',temp_time,z,'-');
% xlabel('Time [S]');
% ylabel('Gravity [g]');
% 
% 
% h(1)=subplot(2,2,2);
% plot(temp_time,AAa,'-r');
% xlabel('Time [S]');
% ylabel('Angle X [degree]');
% 
% h(1)=subplot(2,1,1);
% plot(temp_time,AAc,'-r');
% xlabel('Time [S]');
% ylabel('MAT Z [degree]');
% 
% 
% h(1)=subplot(2,2,4);
%%%zga only for output z
%%plot(temp_time,AAc,'-r');
%%h(1)=subplot(2,1,2);
plot(temp_time,zangle,'-r');
xlabel('Time [S]');
ylabel('Array Z [degree]');
%%%%%%%%%output z is done


%a=atan( (x(count))^2+(z(count))^2/x(count))*180/pi;
%b=atan( (x(count))^2+(z(count))^2/y(count))*180/pi;
%c=acos(z(count))*180/pi;
%%%fprintf('%08f   %08f   %08f\r\n',a,b,c);
%fprintf('%08f\r\n',zangle);
fprintf('%d,%d\r\n',zangle(count),fix(temp_time(count)));
if fix(temp_time(count))>120
    diary off;
end
count=count+1;
count_1=count_1+1;
else 
    count_2=count_2+1;
    end;
else 
    count_2=count_2+1;
end;

%%%fprintf('count1_1 %d count_2 %d\r\n',count_1,count_2);
%pause(0.5);
 end
 end
end
res= [];


end


while 1
res = fscanf(gcom);
if isempty(res)
    ;
else
 fprintf(res);
 if length(res)>=93
     start=5;
strxy= res(start:start+4);start=start+11; xy(count)=str2double(strxy);
stryz= res(start:start+4);start=start+11; yz(count)=str2double(stryz);
strxz= res(start:start+4);start=start+11; xz(count)=str2double(strxz);
strx= res(start:start+4);start=start+11; gx(count)=str2double(strx);
stry= res(start:start+4);start=start+11; gy(count)=str2double(stry);
strz= res(start:start+4);start=start+11; gz(count)=str2double(strz);
strrx= res(start:start+4);start=start+11;rx(count)=str2double(strrx);
strry= res(start:start+4);start=start+11;ry(count)=str2double(strry);
strrz= res(start:start+4);start=start+11;rz(count)=str2double(strrz);
temp_time(count)=etime(clock,t_init);
pause(0.1);
cla;
h(1)=subplot(2,2,1);
plot(temp_time,xy,'-r',temp_time,yz,'-.',temp_time,xz,'-');
grid on;

h(1)=subplot(2,2,2);
plot(temp_time,gx,'-r',temp_time,gy,'-.',temp_time,gz,'-');
grid on;

h(1)=subplot(2,2,3);
plot(temp_time,rx,'-r',temp_time,ry,'-.',temp_time,rz,'-');
grid on;
count=count+1;
pause(0.5);
     
 end
end
res= [];


end


