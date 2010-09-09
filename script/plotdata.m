function data=plotdata(fname)

titles={'Audio', 'Acceleration', 'Compass', 'Battery', 'Temperature', 'Light'};
colors={'b-', 'r-', 'g-'};



data = cell(1,6);
data{1}=load([fname '_aud.dat']);
data{2}=load([fname '_acc.dat']);
data{3}=load([fname '_hmc.dat']);
data{4}=load([fname '_sys.dat']);
data{5}=load([fname '_tmp.dat']);
data{6}=load([fname '_lht.dat']);

datam=load([fname '_merged.dat']);

data{4}(:,2)=data{4}(:,2)/1023*3.3*2;
data{5}(:,2)=floor(data{5}(:,2)/16)*0.0625;

figure(1);
clf;
for i=1:6
    subplot(6,1,i);
    plot([1:length(data{i}(:,1))],data{i}(:,1));
    title(titles{i});
    
    t = data{i}(:,1);
    dt = t(2:end)-t(1:end-1);
    fprintf(1,'Stream %d. Delta T min/mean/max: %f %f %f\n',i,min(dt),mean(dt),max(dt));
end


figure(2);
clf;
for i=1:6
    subplot(6,1,i);
    for j=2:size(data{i},2)
        plot(data{i}(:,1),data{i}(:,j),colors{j-1});
        hold on;
    end
    title(titles{i});
end


figure(3);
clf;
plot(datam(:,1),datam(:,2),'b-');
hold on;
plot(datam(:,1),datam(:,3),'rx-');
plot(datam(:,1),datam(:,4),'ro-');
plot(datam(:,1),datam(:,5),'r+-');
plot(datam(:,1),datam(:,6),'kx-');
plot(datam(:,1),datam(:,7),'ko-');
plot(datam(:,1),datam(:,8),'k+-');
plot(datam(:,1),datam(:,9),'g-');
plot(datam(:,1),datam(:,10),'m-');
plot(datam(:,1),datam(:,11),'y-');

figure(4);
clf;
plot(data{1}(1:32000,2));
title('Audio channel, 32000 samples');

