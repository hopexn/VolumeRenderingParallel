## VolumeRendering
### 一、存在的BUG
1. 拖动会有残影 -- 已修复

### 二、遇到的问题与解决方案
1. Ray Casting
- back to front
```
r = r * (1 - color_sample.a) + color_sample.r * color_sample.a;
g = g * (1 - color_sample.a) + color_sample.g * color_sample.a;
b = b * (1 - color_sample.a) + color_sample.b * color_sample.a;
a += opacity_increment;
```
- front to back
```
float opacity_increment = (1 - a) * color_sample.a;
r += color_sample.r * opacity_increment;
g += color_sample.g * opacity_increment;
b += color_sample.b * opacity_increment;
```      

### 三、TODO
0. 实现八叉树空白空间跳跃(ok)
1. 使用ispc进行优化(ok)
2. 前后端分离(ok)
3. 根据采样率与角度确定采样步长(ok)
