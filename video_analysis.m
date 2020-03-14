%%
clear all;
clc;

%%
v = VideoReader('demo_at_z0.mp4');
numFrames = ceil(v.FrameRate*v.Duration);
distance_vec1 = zeros(1, numFrames);

i = 1;


while i < numFrames

    
    frame = readFrame(v);
    frame = frame(:,:,3);

    center_row = size(frame,1)/2;
    center_col = size(frame,2)/2;



    [B,I] = max(frame(:));

    sz = [size(frame,1), size(frame,2)];
    [row,col] = ind2sub(sz,I);

    distance_vec1(i) = floor(sqrt((center_row-row)^2+(center_col-col)^2));

    i = i + 1;
end

%%
v = VideoReader('demo_above_z0.mp4');
numFrames = ceil(v.FrameRate*v.Duration);
distance_vec2 = zeros(1, numFrames);

i = 1;


while i < numFrames

    
    frame = readFrame(v);
    frame = frame(:,:,3);

    center_row = size(frame,1)/2;
    center_col = size(frame,2)/2;



    [B,I] = max(frame(:));

    sz = [size(frame,1), size(frame,2)];
    [row,col] = ind2sub(sz,I);

    distance_vec2(i) = floor(sqrt((center_row-row)^2+(center_col-col)^2));

    i = i + 1;
end

%%

timePerFrame = 1/30;
time = [1:12456]*timePerFrame;
figure
plot(time, distance_vec1,'DisplayName','at z=0');
hold on
plot(time, distance_vec2, 'DisplayName','above z=0');
title('time v.s. pixel difference plot')
xlabel('time(s)')
ylabel('pixel distance')
legend show
