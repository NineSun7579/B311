%% RLC 滤波器频率响应仿真
% 低通、高通、带通、带阻 + 求导图像
clear; clc; close all;

%% 电路参数
R = 1000;          % 电阻 1kΩ   (范围 1k-10k, 取最小值以提高品质因数Q)
L = 10e-3;         % 电感 10mH  (范围 1mH-10mH, 取最大值以提高Q)
C = 10e-9;         % 电容 10nF  (范围 10nF-100nF, 取最小值以提高Q)
% 上述组合使 Q = (1/R)*sqrt(L/C) = 1.0, 谐振频率 f0 ≈ 15.9kHz,
% 既落在 1~50kHz 输入频率范围中心, 又能让四种滤波器曲线清晰可分。
Vpp = 2;           % 输入峰峰值 2V

%% 频率范围
f = 1000:200:50000;        % 1kHz ~ 50kHz, 步长200Hz
w = 2 * pi * f;            % 角频率
s = 1j * w;                % 拉普拉斯变量 s = jω

%% 谐振频率
f0 = 1 / (2 * pi * sqrt(L * C));
fprintf('谐振频率 f0 = %.2f kHz\n', f0 / 1000);
fprintf('R = %d Ω, L = %.1f mH, C = %.1f nF\n', R, L*1e3, C*1e9);

%% 传递函数 (串联RLC电路, 输出取自不同元件)
% 低通: H(s) = 1 / (LCs^2 + RCs + 1)    输出取自电容
% 高通: H(s) = LCs^2 / (LCs^2 + RCs + 1)  输出取自电感
% 带通: H(s) = RCs / (LCs^2 + RCs + 1)    输出取自电阻
% 带阻: H(s) = (LCs^2 + 1) / (LCs^2 + RCs + 1) 输出取自RL串联

H_low    = 1 ./ (L*C*s.^2 + R*C*s + 1);
H_high   = (L*C*s.^2) ./ (L*C*s.^2 + R*C*s + 1);
H_bandpass = (R*C*s) ./ (L*C*s.^2 + R*C*s + 1);
H_bandstop = (L*C*s.^2 + 1) ./ (L*C*s.^2 + R*C*s + 1);

%% 幅频特性 (dB)
mag_low    = 20 * log10(abs(H_low));
mag_high   = 20 * log10(abs(H_high));
mag_bp     = 20 * log10(abs(H_bandpass));
mag_bs     = 20 * log10(abs(H_bandstop));

%% 相频特性 (度)
phase_low    = angle(H_low) * 180 / pi;
phase_high   = angle(H_high) * 180 / pi;
phase_bp     = angle(H_bandpass) * 180 / pi;
phase_bs     = angle(H_bandstop) * 180 / pi;

%% 输出电压
Vout_low    = abs(H_low) * Vpp;
Vout_high   = abs(H_high) * Vpp;
Vout_bp     = abs(H_bandpass) * Vpp;
Vout_bs     = abs(H_bandstop) * Vpp;

%% ============ 图1: 四种滤波器幅频特性 ============
figure('Name', 'RLC滤波器幅频特性', 'Position', [50 50 900 700]);

subplot(2,1,1);
plot(f/1000, mag_low, 'b-', 'LineWidth', 1.5); hold on;
plot(f/1000, mag_high, 'r-', 'LineWidth', 1.5);
plot(f/1000, mag_bp, 'g-', 'LineWidth', 1.5);
plot(f/1000, mag_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'k--', 'LineWidth', 1.2, 'Label', sprintf('f_0=%.1fkHz', f0/1000));
xlabel('频率 (kHz)');
ylabel('增益 (dB)');
title('四种滤波器 幅频特性');
legend('低通', '高通', '带通', '带阻', '谐振频率', 'Location', 'best');
grid on; xlim([1 50]); ylim([-40 5]);

subplot(2,1,2);
plot(f/1000, Vout_low, 'b-', 'LineWidth', 1.5); hold on;
plot(f/1000, Vout_high, 'r-', 'LineWidth', 1.5);
plot(f/1000, Vout_bp, 'g-', 'LineWidth', 1.5);
plot(f/1000, Vout_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'k--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('输出电压 Vpp (V)');
title('四种滤波器 输出电压 vs 频率');
legend('低通', '高通', '带通', '带阻', '谐振频率', 'Location', 'best');
grid on; xlim([1 50]);

%% ============ 图2: 相频特性 ============
figure('Name', 'RLC滤波器相频特性', 'Position', [100 100 900 500]);

plot(f/1000, phase_low, 'b-', 'LineWidth', 1.5); hold on;
plot(f/1000, phase_high, 'r-', 'LineWidth', 1.5);
plot(f/1000, phase_bp, 'g-', 'LineWidth', 1.5);
plot(f/1000, phase_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'k--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('相位 (度)');
title('四种滤波器 相频特性');
legend('低通', '高通', '带通', '带阻', '谐振频率', 'Location', 'best');
grid on; xlim([1 50]);

%% ============ 图3: 求导图像 (dVout/df) ============
df = f(2) - f(1);  % 频率步长

dVout_low   = diff(Vout_low) / df;
dVout_high  = diff(Vout_high) / df;
dVout_bp    = diff(Vout_bp) / df;
dVout_bs    = diff(Vout_bs) / df;

% 求导对应的频率点
f_diff = f(1:end-1) + df/2;

figure('Name', '输出电压对频率求导', 'Position', [150 150 900 700]);

subplot(2,2,1);
plot(f_diff/1000, dVout_low, 'b-', 'LineWidth', 1.5);
xline(f0/1000, 'r--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('dV_{out}/df (V/kHz)');
title('低通 求导');
grid on; xlim([1 50]);

subplot(2,2,2);
plot(f_diff/1000, dVout_high, 'r-', 'LineWidth', 1.5);
xline(f0/1000, 'b--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('dV_{out}/df (V/kHz)');
title('高通 求导');
grid on; xlim([1 50]);

subplot(2,2,3);
plot(f_diff/1000, dVout_bp, 'g-', 'LineWidth', 1.5);
xline(f0/1000, 'r--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('dV_{out}/df (V/kHz)');
title('带通 求导');
grid on; xlim([1 50]);

subplot(2,2,4);
plot(f_diff/1000, dVout_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'r--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('dV_{out}/df (V/kHz)');
title('带阻 求导');
grid on; xlim([1 50]);

%% ============ 图4: 求导图像汇总对比 ============
figure('Name', '求导图像汇总', 'Position', [200 200 900 500]);

plot(f_diff/1000, dVout_low, 'b-', 'LineWidth', 1.5); hold on;
plot(f_diff/1000, dVout_high, 'r-', 'LineWidth', 1.5);
plot(f_diff/1000, dVout_bp, 'g-', 'LineWidth', 1.5);
plot(f_diff/1000, dVout_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'k--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('dV_{out}/df (V/kHz)');
title('四种滤波器 输出电压对频率求导 对比');
legend('低通导数', '高通导数', '带通导数', '带阻导数', '谐振频率', 'Location', 'best');
grid on; xlim([1 50]);

%% ============ 图5: 二阶求导 ============
d2Vout_low   = diff(dVout_low) / df;
d2Vout_high  = diff(dVout_high) / df;
d2Vout_bp    = diff(dVout_bp) / df;
d2Vout_bs    = diff(dVout_bs) / df;
f_diff2 = f_diff(1:end-1) + df/2;

figure('Name', '二阶求导', 'Position', [250 250 900 500]);

plot(f_diff2/1000, d2Vout_low, 'b-', 'LineWidth', 1.5); hold on;
plot(f_diff2/1000, d2Vout_high, 'r-', 'LineWidth', 1.5);
plot(f_diff2/1000, d2Vout_bp, 'g-', 'LineWidth', 1.5);
plot(f_diff2/1000, d2Vout_bs, 'm-', 'LineWidth', 1.5);
xline(f0/1000, 'k--', 'LineWidth', 1.2);
xlabel('频率 (kHz)');
ylabel('d^2V_{out}/df^2');
title('四种滤波器 二阶求导');
legend('低通二阶导', '高通二阶导', '带通二阶导', '带阻二阶导', '谐振频率', 'Location', 'best');
grid on; xlim([1 50]);

%% ============ 打印关键数据 ============
fprintf('\n===== 关键频率点电压 (Vpp) =====\n');
[~, idx1k] = min(abs(f - 1000));
[~, idx10k] = min(abs(f - 10000));
[~, idx_f0] = min(abs(f - f0));
[~, idx30k] = min(abs(f - 30000));
[~, idx50k] = min(abs(f - 50000));

freq_points = [1000, 10000, round(f0), 30000, 50000];
idx_points = [idx1k, idx10k, idx_f0, idx30k, idx50k];

fprintf('%-10s %-10s %-10s %-10s %-10s\n', '频率', '低通', '高通', '带通', '带阻');
for k = 1:length(freq_points)
    fprintf('%-10.0f %-10.3f %-10.3f %-10.3f %-10.3f\n', ...
        freq_points(k), Vout_low(idx_points(k)), Vout_high(idx_points(k)), ...
        Vout_bp(idx_points(k)), Vout_bs(idx_points(k)));
end

fprintf('\n===== 电路参数 =====\n');
fprintf('R = %d Ω, L = %.1f mH, C = %.1f nF\n', R, L*1e3, C*1e9);
fprintf('谐振频率 f0 = %.2f kHz\n', f0/1000);
fprintf('品质因数 Q = %.2f\n', (1/R)*sqrt(L/C));
fprintf('带宽 BW = %.2f kHz\n', f0/((1/R)*sqrt(L/C))/1000);
