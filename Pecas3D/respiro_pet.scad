

dint_cano_saida = 2;
dext_cano_saida = 4.5;
h_cano_saida = 5;



dint_cano_interno = 4;
dext_cano_interno = 8;
h_cano_interno = 5;


dint_base = 2;
dext_base = 20;
h_base = 2.5;
d_furo_fixacao = 2.8;

dint_anel_fix = 8.5;
dext_anel_fix = dext_base;
h_anel = 2.5;

$fn = 128;
peca = ;
anel = 1;


dext_t = dext_cano_saida * 1.4;
h_t = 3;

if( peca == 1 ){
//Cano interno
difference(){

	cylinder(h=h_cano_interno, r=dext_cano_interno/2);

	cylinder(h=h_cano_interno, r1=dint_cano_interno/2, r2=dint_cano_saida/2);

}


//base
translate([0,0,h_cano_interno-0.01])
difference(){
	cylinder(h=h_base, r=dext_base/2);

	cylinder(h=h_base, r=dint_base/2);

	//furos de fixação
	furos_fixacao();
}


//cano de saida
//------------------------
//base conica
translate([0,0,h_cano_interno+h_base-0.01])
difference(){
	cylinder(h=h_t, r1=dext_t/2, r2=dext_cano_saida/2);

	cylinder(h=h_t, r=dint_cano_saida/2);

}

//cano
translate([0,0,h_cano_interno+h_base+h_t-0.01])
difference(){
	cylinder(h=h_cano_saida, r=dext_cano_saida/2);

	cylinder(h=h_cano_saida, r=dint_cano_saida/2);

}

//calombo
translate([0,0,h_cano_interno+h_base+h_t+4-0.01])
difference(){


	rotate_extrude()
	translate([ 1.5, 0, 0])
	circle( r=1);

	translate([0,0,-3])
	cylinder(h=6, r=dint_cano_saida/2 );

}












}



//anel de fixacao
if( anel == 1 ){
translate([30,0,5])
difference(){
	cylinder(h=h_anel, r=dext_anel_fix/2);

	cylinder(h=h_anel, r=dint_anel_fix/2);

	furos_fixacao();
}
}





p=(dext_base/2)-3.5;

module furos_fixacao(){

translate([ p,0,0  ])
	cylinder(h=10, r= d_furo_fixacao/2);

translate([ -p,0,0  ])
	cylinder(h=10, r= d_furo_fixacao/2);

translate([ 0,p,0  ])
	cylinder(h=10, r= d_furo_fixacao/2);

translate([ 0,-p,0  ])
	cylinder(h=10, r= d_furo_fixacao/2);


}