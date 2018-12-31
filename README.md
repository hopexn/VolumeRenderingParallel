## VolumeRendering
基于空区间跳跃的直接体绘制
该程序需要在曙光3000集群中运行

### Ray Casting
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
