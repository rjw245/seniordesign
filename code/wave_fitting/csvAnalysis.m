close all

%%
M = csvread('EfromGuitarPickup.csv',16,1,[16,1,125015,1]);
M = M';
plot(M);
fs_orig = 1/0.00000032;
fs = 10000;
M = downsample(M,floor(fs_orig/fs));
length(M)
soundsc(M,fs);

latency = 0.003; %seconds
data = M;


s = 10;
x = 700;
SNR = 40;
wave = data(1:latency*fs);
wave = [zeros(1, 1000) wave];
wave2 = wave/2;
wave = awgn(wave,SNR);
wave2 = awgn(wave2,SNR);
figure;
subplot(2,1,1)
plot(wave);
subplot(2,1,2)
[upper_env, lower_env] = envelope(diff(wave),floor(latency*fs),'rms');
plot(upper_env);
hold;
plot(lower_env);
rms(wave);

figure;
subplot(2,1,1);
plot(wave2);
subplot(2,1,2)
[upper_env, lower_env] = envelope(wave2,floor(latency*fs),'rms');
plot(upper_env);
hold;
plot(lower_env);
rms(wave2);

% %soundsc(wave,fs);
% pause(1.1)
% %soundsc(wave2,fs);
% sampled = wave(x:x+s);
% sampled2 = wave2(x:x+s);
% 
% figure
% subplot(2,1,1)
% plot(sampled);
% subplot(2,1,2)
% plot(sampled2)
% 
% f = fft(sampled);
% f = abs(f/s);
% f = f(1:s/2+1);
% l = fs*(0:(s/2))/s;
% 
% f2 = fft(sampled2);
% f2 = abs(f2/s);
% f2 = f2(1:s/2+1);
% 
% figure
% subplot(2,1,1)
% plot(l,f);
% subplot(2,1,2)
% plot(l,f2)


