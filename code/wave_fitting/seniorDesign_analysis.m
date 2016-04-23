close all

%%

s = 10;
x = 700;
wave = data(1:15462);
wave2 = wave/2;
plot(wave);
soundsc(wave,fs);
pause(1.1)
soundsc(wave2,fs);
sampled = wave(x:x+s);
sampled2 = wave2(x:x+s);

figure
subplot(2,1,1)
plot(sampled);
subplot(2,1,2)
plot(sampled2)

f = fft(sampled);
f = abs(f/s);
f = f(1:s/2+1);
l = fs*(0:(s/2))/s;

f2 = fft(sampled2);
f2 = abs(f2/s);
f2 = f2(1:s/2+1);

figure
subplot(2,1,1)
plot(l,f);
subplot(2,1,2)
plot(l,f2)


