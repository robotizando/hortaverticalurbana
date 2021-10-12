
$fn=64;

dBase = 17;
dFuro = 3 ;
dFuroBase = 2.5 ;
dPino = 5 ;
hBase = 2.5;
hPino = 6
;

deltaFuro = 5.5;

difference(){

    render(){
        cylinder( r=dBase/2, h=hBase);

        translate([0,0,hBase])
            cylinder( r=dPino/2, h=hPino);

        translate([0,0,hBase + hPino - 2])
        rotate_extrude(convexity = 10)
        translate([1.7, 0, 0])
        circle(r = 1, $fn = 100);
    }

    cylinder( r=dFuro/2, h=hBase+hPino);

    translate([-deltaFuro,0,0]) cylinder( r=dFuroBase/2, h=hBase);
    translate([0,-deltaFuro,0]) cylinder( r=dFuroBase/2, h=hBase);
    translate([deltaFuro,0,0]) cylinder( r=dFuroBase/2, h=hBase);
    translate([0,deltaFuro,0]) cylinder( r=dFuroBase/2, h=hBase);
    
    
            translate([0,0,-0.7])
        rotate_extrude(convexity = 10)
        translate([deltaFuro, 0, 0])
        circle(r = 1, $fn = 100);
    
}



