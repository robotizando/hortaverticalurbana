/*
	LCD Faceplate with Sunshade
	Daniel Omar Basconcello Filho - daniel@robotizando.com.br

   --------------------------------------------------------------------
   Copyright (C) 2015 Daniel Omar Basconcello Filho

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

$fn = 12;
ajuste_xy = 0.25;

d_furo_m3 = 3 + ajuste_xy;

x_lcd = 71;
y_lcd = 24;

x_size = 100;
y_size = 50;

h_frente = 2.5;
h_total = 30;
parede = 2;



hipo = sqrt( (y_size*y_size) + (h_total*h_total) );
a = acos( y_size/hipo );
echo(a);



translate([0,0,h_total/2])
difference(){


	cube([x_size, y_size, h_total], center=true);

	translate([0,0,h_frente-1])
		cube([x_size-parede, y_size-parede, h_total], center=true);

	translate([0,0, 0])
	cube([x_lcd, y_lcd, h_total], center=true);

	translate([0,0,-h_total]){
		translate([-x_size/2 + 5, -y_size/2 +5, 0])
			cylinder(h=h_total, r=d_furo_m3/2);

		translate([-x_size/2 + 5, y_size/2 -5, 0])
			cylinder(h=h_total, r=d_furo_m3/2);

		translate([x_size/2 - 5, -y_size/2 +5, 0])
			cylinder(h=h_total, r=d_furo_m3/2);

		translate([x_size/2 - 5, y_size/2 -5, 0])
			cylinder(h=h_total, r=d_furo_m3/2);
	}


translate([0,-7.5, (h_total-parede)/2-0.5  ])
rotate([a,0,0])
	cube([x_size, hipo, h_total], center=true);


}








