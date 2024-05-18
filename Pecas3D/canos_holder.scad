
$fn = 64;
xy_ajuste = 0.2;




//----------- Eixo de fixação

dint_eixo = 26 + xy_ajuste;
dext_eixo = dint_eixo + 6; 
h_eixo = 10;
dy_eixo = 10;
dx_fixa = dext_eixo + 20; 
dy_fixa = 6; // espessura do batente de fixacao
d_furo_m3 = 3 + xy_ajuste;


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
}



//----------- LINK
dx_link = 8;
dy_link_ajuste = 1;
dy_link = 1;
h_link = h_eixo;

translate([ -dx_link/2, 34, 0])
	cube([dx_link, dy_link + dy_link_ajuste, h_link ]);


//barra de canos
dy_barra = 8;
d_cano_aquario = 5.5 + xy_ajuste;
qtd_canos = 7;
dx_barra = qtd_canos * 8;



translate([ -dx_barra/2, 34 + dy_link, 0])
difference(){
	cube([dx_barra, dy_barra, h_link ]);

	for(i=[0:qtd_canos]){
		translate([dx_link/2 + (i*(d_cano_aquario+2.2)),dy_barra/2,0])
			cylinder( h =h_link, r=d_cano_aquario/2);
	}
}

