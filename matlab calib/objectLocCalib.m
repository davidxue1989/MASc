clear
uiopen('C:\Users\David\Downloads\HDFaceBasics-D2D\pose.csv',1)
%%
x = VarName1;
y = VarName2;
z = VarName3;
u = VarName4;
v = VarName5;
w = VarName6;

%from angles to radian
u = u/180*pi
v = v/180*pi
w = w/180*pi
% change u, v, w (i.e. pitch, roll, yaw) angles to a unit vector dv
% in other words, transform [0, 0, -1] by u, then v, then w
%http://mathworld.wolfram.com/EulerAngles.html
for i = 1:length(x)
    the = u(i);
    psi = v(i);
    phi = w(i);
    D = [cos(phi), sin(phi), 0;
        -sin(phi), cos(phi), 0;
        0, 0, 1];
    C = [cos(the), 0, -sin(the);
        0, 1, 0;
        sin(the), 0, cos(the)];
    B = [1, 0 ,0;
        0, cos(psi), sin(psi);
        0, -sin(psi), cos(psi)];
    dv = B*C*D*[0; -1; 0];
    u(i) = dv(1);
    v(i) = dv(2);
    w(i) = dv(3);
end


for i = 1:length(x)
%     hold off;
%     scatter3(x(i),y(i),z(i),6);

    sc = 0.5;
    line([x(i), x(i)+sc*u(i)], [y(i), y(i)+sc*v(i)], [z(i), z(i)+sc*w(i)]);
%     line([x(i), x(i)+s*dv(1)], [y(i), y(i)+s*dv(2)], [z(i), z(i)+s*dv(3)]);

%     view(-306, 1.0145e+03)
%     view(0, 90);
    hold on;
%     line([0 x(i)], [0 y(i)], [0 z(i)]);
%     quiver3(x(i),y(i),z(i),u(i),v(i),w(i),0.8);
%     pause(0.1)
%     waitforbuttonpress;
end
axis equal;

% scatter3(x,y,z, 6)
% axis equal
% hold on
% quiver3(x,y,z,u,v,w, 0.8)

%%
% 
% dx: calculating the center as the closest point to all lines.
% for line given by Po + sU, P the center to be calculated,
% closest point on line to P is when s = (P-P0)'*U
% thus, problem is min_p sum_i { norm_2(P0 + [(P-P0)'*U]*U - P)^2 }
% solution is P = [sum_i (I - U*U')]^-1 * [sum_i (I - U*U')*P0]

C = zeros(3);
CP0 = zeros(3,1);
for i = 1:length(x)
    P0 = [x(i), y(i), z(i)].';
    U = [u(i), v(i), w(i)].';    
%     Temp = (U*U.'-eye(3))^2;
    Temp = (eye(3)-U*U.');
    C = C + Temp;
    CP0 = CP0 + Temp*P0;
end
P = inv(C)*CP0;
scatter3(P(1),P(2),P(3),100, 'FaceColor','r');
hold on
axis equal
scatterpts = zeros(3, length(x));
for i = 1:length(x)
    P0 = [x(i), y(i), z(i)].';
    U = [u(i), v(i), w(i)].';
    s = (P-P0).'*U;
    X = P0+s*U;
    scatterpts(:,i) = X;
    scatter3(X(1), X(2), X(3), 16, 'FaceColor','g');
end
P
% note: P = mean(scatterpts.')
COV_INV = inv(cov(scatterpts.'))

%%
zscores = zeros(1, length(x));
for i = 1:length(x)
    P0 = [x(i), y(i), z(i)].';
    U = [u(i), v(i), w(i)].';
    s = (P-P0).'*U;
    X = P0+s*U;
    
    zscores(i) = (X-P).'*COV_INV*(X-P);
end
%%

% 
% %%
% midpts = zeros(3,floor(length(x)^2/2));
% dist = zeros(1,floor(length(x)^2/2));
% count = 1;
% 
% bckpts = zeros(3,floor(length(x)^2/2));
% bcount = 1;
% 
% for i = 1:length(x)
%     for j = i+1:length(x)
% % for i = 1:length(x)-1
% %     j = i+1;
% 
%         if x(i)==0 || y(i)==0 || z(i)==0 || x(j)==0 || y(j)==0 || z(j)==0
%             continue
%         end
% 
%         P0 = [x(i), y(i), z(i)];
%         Q0 = [x(j), y(j), z(j)];
%         U = [u(i), v(i), w(i)];
%         V = [u(j), v(j), w(j)];
%         W = P0 - Q0;
%         a=U*U'; b=U*V'; c=V*V'; d=U*W'; e=V*W';
%         sc=(b*e-c*d)/(a*c-b^2);
%         tc=(a*e-b*d)/(a*c-b^2);
%         
%         Pc=P0+sc*U;
%         Qc=Q0+tc*V;
% 
% %         hold off
% %         scatter3(P0(1),P0(2),P0(3), 6)
% %         hold on
% %         quiver3(P0(1),P0(2),P0(3),U(1),U(2),U(3),0.3)
% %         scatter3(Q0(1),Q0(2),Q0(3), 6)
% %         quiver3(Q0(1),Q0(2),Q0(3),V(1),V(2),V(3),0.3)
%         
%         
%         if (sc > 0.1 && tc > 0.1)
%             dist(:,count) = norm(W+(sc*U-tc*V));
%             midpts(:,count) = (Pc+Qc)'/2;
%             count = count + 1;
%             
% % %             scatter3(Pc(1),Pc(2),Pc(3), 6);
% % %             scatter3(Qc(1),Qc(2),Qc(3), 6);
% %             midpt = (Pc+Qc)'/2;
% % %             scatter3(midpt(1),midpt(2),midpt(3), 6);
% %             h = scatter3(midpt(1),midpt(2),midpt(3), 500, 'red');
% % %             
% % %             dummy = 3;
%         else
%             bckpts(:,bcount) = (Pc+Qc)'/2;
%             bcount = bcount + 1;
%             
% % %             scatter3(Pc(1),Pc(2),Pc(3), 6);
% % %             scatter3(Qc(1),Qc(2),Qc(3), 6);
% %             midpt = (Pc+Qc)'/2;
% % %             scatter3(midpt(1),midpt(2),midpt(3), 6);
% %             h = scatter3(midpt(1),midpt(2),midpt(3), 500, 'green');
%         end
% %         h1 = line([x(i), x(i)+s*u(i)], [y(i), y(i)+s*v(i)], [z(i), z(i)+s*w(i)]);
% %         set(h1, 'Color', [1, 0, 0]);
% %         set(h1, 'LineWidth', 5);
% %         h2 = line([x(j), x(j)+s*u(j)], [y(j), y(j)+s*v(j)], [z(j), z(j)+s*w(j)]);
% %         set(h2, 'Color', [1, 0, 0]);
% %         set(h2, 'LineWidth', 3);
%         
% % %         waitforbuttonpress;
% %         delete(h);
% %         delete(h1);
% %         delete(h2);
%     end
% end
% midpts = midpts(:, 1:count-1);
% dist = dist(:, 1:count-1);
% bckpts = bckpts(:, 1:bcount-1);
% %%
% % figure
% % hold on
% % hold off
% scatter3(midpts(1,:), midpts(2,:), midpts(3,:), 3);
% axis equal
% hold on
% scatter3(bckpts(1,:), bckpts(2,:), bckpts(3,:), 3);
% axis equal
% 
% % rpts = midpts(:, 1:10:length(midpts));
% % brpts = bckpts(:, 1:10:length(bckpts));
% % hold on
% % scatter3(rpts(1,:), rpts(2,:), rpts(3,:), 6);
% % scatter3(brpts(1,:), brpts(2,:), brpts(3,:), 6);
% % mean(dist)
% % 
