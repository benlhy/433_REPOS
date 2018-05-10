lpf = fir1(5,0.2)
figure
plot(conv(lpf,T(:,2)))
hold on
plot(T(:,2))
hold off