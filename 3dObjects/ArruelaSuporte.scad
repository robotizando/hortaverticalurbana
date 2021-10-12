
$fn=128;

hBase = 5;
dExterno = 45;
dInterno = 28.5;

dFuro = 6.5;


difference(){
    
    
    cylinder(h=hBase, r=dExterno/2);
    cylinder(h=hBase, r=dInterno/2);
    
    translate([-dInterno/2 - (dExterno-dInterno)/4 ,0,0])
    scale([0.6,1,1])
    cylinder(h=hBase, r=dFuro/2);

    translate([dInterno/2 + (dExterno-dInterno)/4 ,0,0])
    scale([0.6,1,1])
    cylinder(h=hBase, r=dFuro/2);

    translate([0 ,-dInterno/2 - (dExterno-dInterno)/4,0])
        scale([1,0.6,1])
    cylinder(h=hBase, r=dFuro/2);
    
    translate([0, dInterno/2 + (dExterno-dInterno)/4,0])
    scale([1,0.6,1])
    cylinder(h=hBase, r=dFuro/2);
    
}


