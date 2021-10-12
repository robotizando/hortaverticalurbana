use <threadlib/threadlib.scad>



$fn = 256;

dBaseAro = 25;
dTopoAro = 45;
hCone = 25;
parede = 1.4;




*nut("PCO-1881", turns=3, Douter=34);



*difference(){
    import("3dponics-drip-nozzle-v2.stl");
    
    difference(){
        cylinder(h=10, r=30);
        cylinder(h=10, r=16.5);
    }    

    translate([0,0,9.95])
    color("red")
        cylinder(h=15.2, r=9);
}


//cone


translate([0,0,10]){

    //cone
    difference(){
        cylinder(h=hCone, r1=dBaseAro/2, r2=dTopoAro/2);
        cylinder(h=hCone, r1=(dBaseAro/2)-parede/2, r2=(dTopoAro/2)-parede/2);
    }


    //base
    dBase = 30;
    hBase = 1;
    dFuro = 2.5;
    difference(){
        cylinder(h=hBase, r=dBase/2);
        cylinder(h=hBase, r=dFuro/2);
        
        for ( i = [0 : 5] ){
            rotate( i * 60, [0, 0, 1])
            translate([-6.5,0,0])
            cylinder(h=hBase, r=dFuro/2);
        }
    }

    hPino = 5;
    dPino = 8;
    
    difference(){
        cylinder(h=hPino, r=dPino/2);
        cylinder(h=hPino, r=dFuro/2);
    }

    
    
}