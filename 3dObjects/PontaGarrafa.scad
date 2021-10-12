
use <OpenScad-libraries/threadlib/threadlib.scad>;

$fn=64;

dPInt = 6;
dPExt = 9;
hPonta = 15;


dInt = 4.5;
dExt = 9;
hTubo = 10;

dBInt = 4.5;
dBExt = 15;
hBase = 2;

*difference() {
    cylinder(r = 15, h = 20);
    tap("PCO-1881", 8);
}



difference(){
    cylinder(h=hBase, r=dBExt/2);
    cylinder(h=hBase, r=dBInt/2);
}


translate([0,0,hBase])
difference(){
    cylinder(h=hTubo, r=dExt/2);
    cylinder(h=hTubo, r=dInt/2);
}

translate([0,0,hTubo+hBase])
difference(){
    cylinder(h=hPonta, r=dPExt/2);
    cylinder(h=hPonta, r=dPInt/2);
}

