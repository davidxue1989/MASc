clear all

%%%%%%%%%original reference frame: x, y, z%%%%%%%%%%%
xaxis = [linspace(0, 100, 11); zeros(1,11); zeros(1,11)]
yaxis = [zeros(1,11); linspace(0, 100, 11); zeros(1,11)]
zaxis = [zeros(1,11); zeros(1,11); linspace(0, 100, 11)]

a1 = [xaxis, yaxis, zaxis]
scatter3(a1(1,:), a1(2,:), a1(3,:))

% scatter3(xaxis(1,:), xaxis(2,:), xaxis(3,:))
% hold on
% scatter3(yaxis(1,:), yaxis(2,:), yaxis(3,:))
% hold on
% scatter3(zaxis(1,:), zaxis(2,:), zaxis(3,:))

%%%%%%%%%rotated frame: rotate on z for c, then on y for b, then on x for a%%%%%%%%%%%%%%%%%%%%%%
a = -pi/2-0.08;
b = 0;
c = pi/2-0.08;

Rx = [  1   0       0;
        0   cos(a)  -sin(a);
        0   sin(a)  cos(a)  ]

Ry = [  cos(b)  0   sin(b);
        0       1   0;
        -sin(b) 0   cos(b)  ]

Rz = [  cos(c)  -sin(c) 0;
        sin(c)  cos(c)  0;
        0       0       1   ]

Rx1 = [  1   0       0;
        0   cos(a)  sin(a);
        0   -sin(a)  cos(a)  ]

Ry1 = [  cos(b)  0   -sin(b);
        0       1   0;
        sin(b) 0   cos(b)  ]

Rz1 = [  cos(c)  sin(c) 0;
        -sin(c)  cos(c)  0;
        0       0       1   ]    
    
a2 = Rx*Ry*Rz*a1;
hold on
scatter3(a2(1,:), a2(2,:), a2(3,:))

%%%%%%%%%%rotated frame based on euler angles: rotate on z for psi, y' for the, x'' for phi%%%%%%%%%%
phi = 0; the = -pi/2-0.08; psi = -pi/2-0.08;
A = [cos(the)*cos(psi)  cos(phi)*sin(psi)+sin(phi)*sin(the)*cos(psi)    sin(phi)*sin(psi)-cos(phi)*sin(the)*cos(psi);
    -cos(the)*sin(psi)  cos(phi)*cos(psi)-sin(phi)*sin(the)*sin(psi)    sin(phi)*cos(psi)+cos(phi)*sin(the)*sin(psi);
    sin(the)    -sin(phi)*cos(the)  cos(phi)*cos(the)]
a3 = A*a1;
hold on
scatter3(a3(1,:), a3(2,:), a3(3,:))
