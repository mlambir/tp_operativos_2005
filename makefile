todo : equipo ciudad cde router

equipo : equipo.a equipobib.a logs.a configuracion.a
	   gcc -g equipo.a equipobib.a logs.a configuracion.a -o equipo
equipo.a : equipo.c
	   gcc  -g -c equipo.c -o equipo.a
equipobib.a : equipobib.c
	   gcc  -g -c equipobib.c -o equipobib.a

ciudad : ciudad.a ciudadBib.a configuracion.a logs.a ciudadCurs.a
	   gcc   -g ciudad.a ciudadBib.a configuracion.a logs.a ciudadCurs.a -o ciudad -lncurses
ciudad.a : ciudad.c
	   gcc  -g -c ciudad.c -o ciudad.a
ciudadBib.a : ciudadBib.c
	   gcc  -g -c ciudadBib.c -o ciudadBib.a

cde : cde.a cdeCurs.a logs.a
	   gcc   -g cde.a logs.a cdeCurs.a -o cde -lncurses
cde.a : cde.c
	   gcc  -g -c cde.c -o cde.a

router : router.a routerBib.a configuracion.a
	   gcc   -g router.a routerBib.a configuracion.a -o router
router.a : router.c
	   gcc  -g -c router.c -o router.a
routerBib.a : routerBib.c
	   gcc  -g -c routerBib.c -o routerBib.a

configuracion.a : configuracion.c
	   gcc  -g -c configuracion.c -o configuracion.a

logs.a : logs.c
	   gcc  -g -c logs.c -o logs.a

ciudadCurs.a : ciudadCurs.c
	   gcc  -g -c ciudadCurs.c -o ciudadCurs.a

cdeCurs.a : cdeCurs.c
	   gcc  -g -c cdeCurs.c -o cdeCurs.a
