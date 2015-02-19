%%
x = VarName1;
y = VarName2;
z = VarName3;
u = VarName4;
v = VarName5;
w = VarName6;
%%

for i = 1:length(x)
%     hold off;
%     scatter3(x(i),y(i),z(i),6);
    s = 1;
    line([x(i), x(i)+s*u(i)], [y(i), y(i)+s*v(i)], [z(i), z(i)+s*w(i)]);
    axis equal;
%     view(-306, 1.0145e+03)
%     view(0, 90);
    hold on;
%     line([0 x(i)], [0 y(i)], [0 z(i)]);
%     quiver3(x(i),y(i),z(i),u(i),v(i),w(i),0.8);
%     pause(0.1)
%     waitforbuttonpress;
end

% scatter3(x,y,z, 6)
% axis equal
% hold on
% quiver3(x,y,z,u,v,w, 0.8)

%%
C = zeros(3);
CP0 = zeros(3,1);
for i = 1:length(x)
    P0 = [x(i), y(i), z(i)].';
    U = [u(i), v(i), w(i)].';    
    Temp = (U*U.'-eye(3))^2;
    C = C + Temp;
    CP0 = CP0 + Temp*P0;
end
P = inv(C)*CP0;
scatter3(P(1),P(2),P(3),100, 'red');
hold on
axis equal
scatterpts = zeros(3, length(x));
for i = 1:length(x)
    P0 = [x(i), y(i), z(i)].';
    U = [u(i), v(i), w(i)].';
    s = (P-P0).'*U;
    X = P0+s*U;
    scatterpts(:,i) = X;
    scatter3(X(1), X(2), X(3), 50, 'green');
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
