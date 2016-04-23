close all

%%
[data,fs ] = audioread('pianoNote088.wav');
soundsc(data,fs);

latency = 0.003; %seconds


s = 10;
x = 700;
SNR = 40;
wave = data(30:latency*fs+30);
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


