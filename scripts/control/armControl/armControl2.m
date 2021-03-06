%attempt 2 to control arm implementing kalman filter on sensors

starting_pos = -1*pi/4;
setpoint = starting_pos - pi/2;

figure();
x = zeros(2,1);
x(1) = starting_pos
x_prev = x;
dt = .02;
time = 0:dt:10;

x_est = x;
x_mea = zeros(2,1);
arm_length = .5;%cg at 16 in ~ 14 in if it is uncurled ~18 in
arm_mass = 5; %9 pounds unloaded 34 pounds loaded
gravity_torque_est = arm_mass * arm_length * cos(x_est(1));


U = 0;

UPlot = zeros(1,length(time));
XPlot = zeros(2,length(time));
MeaPlot = zeros(2,length(time));
EstPlot = zeros(2,length(time));
plotIndex = 1;

A = [0, 1; 0, 0]; %simple model, arm angle is time integral of arm speed in rad/s
B = [0;-1/arm_mass]; %input matrix
C = eye(2); %measurement input matrix
F = diag([1,1]); %state disturbance std. dev input matrix
Q = F*F'; %state disturbance (process noise) covariance matrix
R = zeros(2,2); %measurement covariance matrix
R(1,1) = pi * 1/180; %capacitive sensor has accuracy/noise of about .5 deg
R(2,2) = 800/545 * 2*pi/60; %motor rpm feedback has noise of about 400 rpm, 545:1 gear box, 2pi/60 converstion rpm to rad/s
H = lqr(A', C', Q, R)' %calculate observer gains using linear quadratic regulator methods
observerpoles = eig(A - H*C)
%this is a kalman filter because it solves the same ricatti eqs.

rng(900)

for t = time
    subplot(4,1,1);
    cla;
    x = armSim(x, U, dt, arm_length, arm_mass);
    armDraw(x, arm_length);
    
    x_mea(1) = x(1) + normrnd(0,pi * 1/180); %capacitive sensor has accuracy/noise of about .5 deg
    x_mea(2) = x(2) + normrnd(0, 800/545 * 2*pi/60 ); %motor rpm feedback has noise of about 400 rpm, 545:1 gear box
    
    x_est = x_est + (A * x_est + B*(U+gravity_torque_est) + H * (x_mea - x_est))*dt;
    %P = A * P * A' + Q; %new covariance of new estimate
    %K = (P*H') / (H*P*H' + Q); %new gains based on covariances

    gravity_torque_est = arm_mass * arm_length * cos(x_est(1));
    U = 36*(x_est(1) - setpoint)  + 20*x_est(2) - gravity_torque_est;
    
    UPlot(1,plotIndex) = U;
    XPlot(:,plotIndex) = x;
    EstPlot(:,plotIndex) = x_est;
    MeaPlot(:,plotIndex) = x_mea;
    subplot(4,1,2);
    plot(0:dt:t, UPlot(1,1:plotIndex));
    legend('U');
    subplot(4,1,3);
    plot(0:dt:t, XPlot(1,1:plotIndex), 0:dt:t, zeros(1,plotIndex) + setpoint, '--');
    legend('X', 'Setpoint');
    subplot(4,1,4);
    plot(0:dt:t, XPlot(:,1:plotIndex),'-',0:dt:t, MeaPlot(:,1:plotIndex),'*',0:dt:t, EstPlot(:,1:plotIndex),'--');
    legend('X1','X2', 'mX1', 'mX2', 'eX1', 'eX2');
    plotIndex = plotIndex +1;

end