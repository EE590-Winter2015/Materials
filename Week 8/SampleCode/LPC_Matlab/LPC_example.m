%% some example IIR filters
figure(1)
% roots of the coefficients are really important...
r1 = rand(1)*exp(-1i*pi/(10*rand(1)));
r2 = rand(1)*exp(-1i*pi/(3*rand(1)));
% use the roots to generate the coefficients
a  = poly([r1 conj(r1) r2 conj(r2)]);
% show the coefficients
disp(a)
% plot the magnitude spectrum and phase of the filter
freqz(1,a);


%% Create A Time Vector, x
fs = 44100; % sampling frequency
t = 0:1/fs:0.015; % 150ms of data

x = 0.35 * sin(2*pi*1500.*t) + 0.25 * sin(2*pi*5500.*t); % sum two sine waves
x = x + randn(size(x))/20;

% plot the time vector
figure(2);
subplot(2,1,1);
plot(1000*t,x);
xlabel('time (ms)')
title('0.35 x sin(2\pi1500t) + 0.25 x sin(2\pi5500t) + noise');
grid on;
x = x.*hamming(length(x))';

%% get the FFT of the data

X = fft(x);
X = abs(X(1:end/2+1)./sqrt(length(X))).^2;%normalize and convert to mag^2

% graph the FFT
subplot(2,1,2); hold off;
freq_vector = (0:length(X)-1)/length(X)*fs/2;
plot(freq_vector/1000,10*log(X));
xlabel('Freq (kHz)');
ylabel('10log(|X|^2)');
grid on;

%% get the LPC
clc;
order = 15;
[a,g] = lpc(x,order);
disp(a)

% now get the frequency points which we want to graph
f = (1:length(X)*5)/length(X)/5*pi;
% go through and calculate what the magnitude of the spectrum is
denominator = ones(size(f));
for i=1:length(a)-1
  denominator = denominator + a(i+1)*exp(-1i * f * i);
end
Xlpc = abs((g)./denominator);

% plot the lpc to see how it looks compared to the FFT
subplot(2,1,2); hold on;
freq_vector = (1:length(Xlpc))/length(Xlpc)*fs/2;
plot(freq_vector/1000,10*log(Xlpc),'k','linewidth',2);
grid on;


%% get the resonances and bandwidth strictly from the roots

% get roots
r = roots(a);
freq = angle(r);
% eliminate the conjugates
r = r(freq>=0);
freq = freq(freq>=0);
% now sort by the magnitude of the root (usually this isn't needed)
mag = ones(size(freq));
for i=1:length(freq)
  mag(i) = g/abs(a*exp(-1i.*freq(i).*(0:length(a)-1))');
end
freq = freq*fs/(2*pi); % convert frequency to Hz
bw   = -fs/pi*log(abs(r)); % this is the bandwidth of resonance


for i=1:length(mag)
  if 10*log(mag(i))>-40
    text(freq(i)/1000,10*log(mag(i)),sprintf('R%d,F=%.0fHz,BW=%.0fHz',i,freq(i),bw(i)));
  end
end

%% sample from the microphone and plot some stats of the recording

r = audiorecorder(fs, 16, 1);
fs = 44100; % sampling frequency
numsamples = 30;
figure(3);
ax1 = subplot(4,1,1);
ax2 = subplot(4,1,2);
ax3 = subplot(4,1,3:4);

my_stft = ones(fs/2,numsamples)*(-200);
for iter=1:numsamples
  
  record(r);     % speak into microphone...
  pause(0.05);
  stop(r);
  mySpeech = getaudiodata(r, 'int16'); % get data as int16 array
  
  x=double(mySpeech)./2^16;
  x = x.*hamming(length(x));
  
  % get the FFT of the data
  X = fft(x);
  X = abs(X(1:end/2+1)./sqrt(length(x))).^2;%normalize and convert to mag^2
  
  % get the LPC
  order = 15;
  [a,g] = lpc(x,order);
  
  % now get the frequency points which we want to graph
  f = (1:fs/2)/(fs/2)*pi;
  % go through and calculate what the magnitude of the spectrum from the
  % equation
  denominator = ones(size(f));
  for i=1:length(a)-1
    denominator = denominator + a(i+1)*exp(-1i * f * i);
  end
  Xlpc = abs((g)./denominator);
  my_stft(:,iter) = 10*log(Xlpc);
  
  % graph out the speech and the frequency analysis
  plot(ax1,1000*(1:length(x))./fs,x);
  xlabel('time (ms)')
  hold(ax2, 'off');
  plot(ax2,(1:length(X))/length(X)*fs/2000,10*log(X)); 
  hold(ax2, 'on');
  plot(ax2,(1:length(Xlpc))/length(Xlpc)*fs/2000,10*log(Xlpc),'k');
  xlabel(ax2,'Freq (kHz)')
  %   xlim(ax2,[15 20]);
  imagesc(my_stft,'parent',ax3);
  axis(ax3, 'xy');
  
  % get roots
  rr = roots(a);
  freq = angle(rr);
  % eliminate the conjugates
  rr = rr(freq>=0);
  freq = freq(freq>=0);
  % now sort by the magnitude of the root (usually this isn't needed)
  mag = ones(size(freq));
  for i=1:length(freq)
    mag(i) = g/abs(a*exp(-1i.*freq(i).*(0:length(a)-1))');
  end
  freq = freq*fs/(2*pi); % convert frequency to Hz
  bw   = -fs/pi*log(abs(rr)); % this is the bandwidth of resonance
  
  [unused idxsort] = sort(mag,'descend');
  
  mag = 10*log(mag);
  idx = 0;
  allfreqs = [];
  for i=idxsort(:)'
    if idx<3 && freq(i) > 50 && all(abs(allfreqs-freq(i))>200)
      str = sprintf('F=%.0fHz,BW=%.0fHz',freq(i),bw(i));
      text(freq(i)/1000,mag(i),str,'parent',ax2);
      disp([num2str(i) ' ' str])
      allfreqs(end+1) = freq(i);
      idx=idx+1;
    end
  end
  drawnow();
end


%% whisper vocoder via the lpc

fs = 44100; % sampling frequency
r = audiorecorder(fs, 16, 1);
figure(4);
ax1 = subplot(2,1,1);
ax2 = subplot(2,1,2);

for iters = 1:4
  fprintf('%d/%d Recording...', iters, 4)
  record(r);     % speak into microphone...
  pause(4);
  stop(r);
  fprintf('Coding...');
  mySpeech = getaudiodata(r, 'int16'); % get data as int16 array
  
  x=double(mySpeech)./2^15;
  
  % get the LPC
  order = 15;
  bufflen = 882;% 15 ms of speech
  xbuff = buffer(x,bufflen);
  [a,g] = lpc(xbuff,order);
  
  % excite each lpc filter in the buffer with white noise
  yrand = rand(size(x))-0.5;
  y = yrand;

  for i=1:length(g)
    y(bufflen*(i-1)+1:bufflen*(i)) =...
      filter(sqrt(g(i))*4,[1 a(i,2:end)], yrand(bufflen*(i-1)+1:bufflen*(i)));

  end
  
  
  fprintf('Playing\n');
  % lip radiation model, smooth the output together
  y = filter([1 -0.9],1,y);
  sound(y,fs);  
  
  plot(ax1,1:length(x),x);
  title(ax1,sprintf('Orginal Audio, %.2f bytes/s',length(x)*2/4));
  plot(ax2,1:length(y),y);
  title(ax2,sprintf('Filtered Noise, %.2f bytes/s',(numel(a)+numel(g))*2/4));

  drawnow();
end

