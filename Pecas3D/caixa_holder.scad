
$fn = 64;
xy_ajuste = 0.2;




//----------- Eixo de fixação

dint_eixo = 29 + xy_ajuste;
dext_eixo = dint_eixo + 8; 
h_eixo = 10;
dy_eixo = 10;
dx_fixa = dext_eixo + 20; 
dy_fixa = 6; // espessura do batente de fixacao
d_furo_m3 = 3 + xy_ajuste;
d_cabeca_m3 = 6 + xy_ajuste;


module fixa_eixo( dint, hx ){ 

difference(){
	//furo do eixo e suporte do prensador
	render(){
		cylinder(h=hx, r=dext_eixo/2 );

		translate([-dx_fixa/2, -dy_fixa/2, 0])
		cube([dx_fixa, dy_fixa, hx]);
	}

	//furos m3 do fixador
	translate([(dx_fixa/2)-d_furo_m3*1.5, (dy_fixa/2)+0.1 , hx/2])
	rotate([90,0,0])	
		cylinder(h=dy_fixa+0.2, r=d_furo_m3/2 );	

	translate([-(dx_fixa/2)+d_furo_m3*1.5, (dy_fixa/2)+0.1 , hx/2])
	rotate([90,0,0])	
		cylinder(h=dy_fixa+0.2, r=d_furo_m3/2 );	

	cylinder(h=h_eixo, r=dint/2 );
}
}


dy_corte = 20;

//parte colada ao link
translate([0,dy_eixo,0])
difference(){
	fixa_eixo(dint_eixo, h_eixo);
	translate([-dx_fixa/2, 0, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);
}


//parte separada
translate([0,dy_eixo + 10,0])
difference(){
	fixa_eixo(dint_eixo, h_eixo);

	translate([-dx_fixa/2, -dy_corte, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);

	translate([0,0,h_eixo/2])
	rotate([-90,0,0])
	cylinder(h=h_eixo*2, r=d_cabeca_m3/2);

}



//----------- LINK
dx_link = 10;
dy_link_ajuste = 1;
dy_link = 12;//dy_eixo - (dext_apoio_r/2) - (dext_eixo/2) + 2;
h_link = h_eixo;

difference(){

	translate([ -dx_link/2, 37, 0])
		cube([dx_link, dy_link + dy_link_ajuste, h_link ]);

	translate([0,dint_eixo,h_eixo/2])
	rotate([-90,0,0])
	cylinder(h=h_eixo*3, r=d_furo_m3/2);

}

 
