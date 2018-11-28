/*
 Copyright (c) 2016 Academia Sinica, Institute of Information Science

 License:

     GPL 3.0 : The content of this file is subject to the terms and
     cnditions defined in file 'COPYING.txt', which is part of this source
     code package.

 Project Name:

     BeDIPS

 File Description:

     In this file, we group all the definition and declarations used in Gateway
     and LBeacon, including.

 File Name:

     bedis.h

 Abstract:

     BeDIPS uses LBeacons to deliver 3D coordinates and textual
     descriptions of their locations to users' devices. Basically, a
     LBeacon is an inexpensive, Bluetooth Smart Ready device. The 3D
     coordinates and location description of every LBeacon are retrieved
     from BeDIS (Building/environment Data and Information System) and
     stored locally during deployment and maintenance times. Once
     initialized, each LBeacon broadcasts its coordinates and location
     description to Bluetooth enabled user devices within its coverage
     area.

 Authors:

     Gary Xiao     , garyh0205@hotmail.com

 */

 #include <stdio.h>
 #include <stdlib.h>

 #ifdef BEDIS_H
 #defin BEDIS_H

 /* Length of coordinates in number of bits */
 #define COORDINATE_LENGTH 64

 typedef struct{

     char X_coordinates[COORDINATE_LENGTH];
     char Y_coordinates[COORDINATE_LENGTH];
     char Z_coordinates[COORDINATE_LENGTH];

 }Coordinates;



 #endif
