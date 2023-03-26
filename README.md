<img src="media/top.jpg"  width="300" height="300"> <img src="media/bottom.jpg"  width="300" height="300"> <img src="media/side.jpg"  width="300" height="300">

# RoboScale - TL;DR

This is a small hobby-scale with a weight capacity of 400g and a resolution of +/- .5 grams.  It is connected through an 18” shielded, twisted-pair RJ-45 cable to a [SparkFun I2C Qwiic Scale breakout board](https://www.sparkfun.com/products/15242).

This is a PERFECT scale for Arduino projects.


## This scale will be custom-built to order (takes about a day to build). 

The scale is built using an amateur 3D printer, so it has some blemishes, but it still looks cool!  Eventually, I might use a professional printing service, but for now, we'll just keep this simple :-)

This scale has been calibrated.  So if you put something on it, say, that weighs 50g, the scale will read 50g +/- .5g. 

I've provided an example [Arduino .INO file](src/roboScale.ino) that demonstrates exactly how to use this scale. Notice the scale factor code:

```
// These values are scale-specific and need to be derived for each scale!
double SCALE_FACTOR = 0.000666;   
double SCALE_OFFSET = 2.91;  
```

These values will be specifically derived for the scale that is custom built for each order. When you receive your scale, you will be given the exact 'SCALE_FACTOR' and 'SCALE_OFFSET' that you need to provide in your Arduino code.



## Price is $39.99 




