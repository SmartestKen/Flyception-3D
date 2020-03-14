
% predetermined offset
offset_pos=34290;
offset_voltage=20/2^16*offset_pos-10;

% raw voltage and pos without offset
pos = [31955;32725;33500;34290;35070;35855;36630;];
voltage=20/2^16.*x_g-10;

% offset the raw values and compute respective theta angles
real_voltage=(voltage-offset_voltage);
theta_x=real_voltage/0.5*2;


% distance in real system using chessboard as reference
chessboard_x=[-3:3]'.*0.016;
% linear fit and plot
P_x=polyfit(tand(theta_x),chessboard_x,1);
l_x=P_x(1)
plot(chessboard_x,theta_x)