
$fn=32;

dIntCone = 34;
dExtCone = dIntCone + 3;
hCone = 43;


hBico=8;
dIntBico=2.5;
dExtBico=5.1;

dIntAba = dExtCone;
dExtAba = dIntAba + 8;
hAba = 1;



*translate([0,0,hCone-0.1])
    tampa(bicos=1);




difference(){
    cylinder(h=hAba, r=dExtAba/2);
    cylinder(h=hAba, r=dIntAba/2);
}

difference(){
    cylinder(h=hCone, r=dExtCone/2);
    cylinder(h=hCone, r=dIntCone/2);

translate([dExtCone/2-3,25,3])
rotate([90,0,0])
cylinder(h=50,r=1);

translate([-dExtCone/2+3,25,3])
rotate([90,0,0])
cylinder(h=50,r=1);

}







module tampa(bicos=1){
  
    if(bicos==1){
        translate([0,0,dExtCone/2-1.7]){
            bico();
        }
    }
    
    if(bicos==2){
        translate([0,0,0]){
            translate([-5,0,dExtCone/2-3])
            bico();

            translate([5,0,dExtCone/2-3])
            bico();
        }
    }   
        
    difference(){

        sphere(r=dExtCone/2);
        sphere(r=dIntCone/2);
        
        if(bicos==2){
            translate([0,0,0]){
                translate([-5,0,dExtCone/2-5])
                bico(false);

                translate([5,0,dExtCone/2-5])
                bico(false);
            }
        }   
         
        if(bicos==1){
            translate([0,0,0]){
                translate([0,0,dExtCone/2-5])
                bico(false);
            }
        }
        
        translate([-dExtCone/2, -dExtCone/2,-dExtCone/2])
            cube([dExtCone,dExtCone,dExtCone/2]);    
    }
}




module bico(furado=true){
    difference(){
        
        render(){
            cylinder(h=hBico, r=dExtBico/2);
        
            translate([0,0, hBico-2])
            rotate_extrude(convexity = 10)
            translate([1.7, 0, 0])
            circle(r = 1, $fn = 100);
        }
        
        if(furado)
            cylinder(h=hBico, r=dIntBico/2);
    }
}
