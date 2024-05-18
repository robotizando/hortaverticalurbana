
$fn = 64;
xy_ajuste = 0.2;



//----------- Eixo de fixação

dint_eixo = 26 + xy_ajuste; //diametro do cano do tripé
dext_eixo = dint_eixo + 12; 
h_eixo = 18;

d_furo_m6 = 6 + xy_ajuste;


module fixa_eixo(dext, dint, hx ){ 
	difference(){
		cylinder(h=hx, r=dext/2 );

		cylinder(h=hx, r=dint/2 );

		translate([-dext/2, -dext/2, 0]) //corte central
			cube([dext, dext/2, hx]);

	translate([0,0,hx/2])
	rotate([-90,0,0])
	cylinder(h=hx*2, r=d_furo_m6/2 );

	}


}


//apoio horizontal
dint_cano = 16;
dext_cano = dint_cano + 12;
h_cano = 40;


rotate([-90,0,0]){

//apoio vertical
fixa_eixo(dext_eixo, dint_eixo, h_eixo);

//parte colada ao link
translate([h_cano/2, dext_eixo-8, h_eixo/2])
rotate([180,90,0])
	fixa_eixo(dext_cano, dint_cano, h_cano);

}

