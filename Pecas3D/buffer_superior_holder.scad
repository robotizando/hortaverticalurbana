
$fn = 128;
xy_ajuste = 0.2;






//----------- Eixo de fixação

dint_eixo = 35 + xy_ajuste;
dext_eixo = dint_eixo + 6; 
h_eixo = 10;
dy_eixo = 10;
dx_fixa = dext_eixo + 20; 
dy_fixa = 6; // espessura do batente de fixacao
d_furo_m3 = 3 + xy_ajuste;


module fixa_eixo(dext, dint, hx ){ 

difference(){
	//furo do eixo e suporte do prensador
	render(){
		cylinder(h=hx, r=dext/2 );

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


dy_corte = 30;

//parte colada ao link
translate([0,dy_eixo,0])
difference(){
	fixa_eixo(dext_eixo, dint_eixo, h_eixo);
	translate([-dx_fixa/2, 0, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);
}


//parte separada
translate([0,dy_eixo + 10,0])
difference(){
	fixa_eixo(dext_eixo, dint_eixo, h_eixo);
	translate([-dx_fixa/2, -dy_corte, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);
}



//----------- LINK
dx_link = 8;
dy_link_ajuste = 1;
dy_link = 18;//dy_eixo - (dext_apoio_r/2) - (dext_eixo/2) + 2;
h_link = 5;

translate([ -dx_link/2, 40, 0])
	cube([dx_link, dy_link + dy_link_ajuste, h_link ]);

translate([-1.5,40,0])
cube([3,dy_link,10]);

//-- garaffa

dint_garrafa = 27;
dext_garrafa = dint_garrafa + 5;
h_eixo_garrafa = 5;
dy_garrafa = dy_eixo + dy_link + 46;

//parte colada ao link
translate([0,dy_garrafa,0])
difference(){
	fixa_eixo(dext_garrafa, dint_garrafa, h_eixo_garrafa);
	translate([-dx_fixa/2, 0, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);
}

//parte separada
translate([0,dy_garrafa + 5,0])
difference(){
	fixa_eixo(dext_garrafa, dint_garrafa, h_eixo_garrafa);
	translate([-dx_fixa/2, -dy_corte, 0]) //corte central
		cube([dx_fixa, dy_corte, h_eixo]);
}




 