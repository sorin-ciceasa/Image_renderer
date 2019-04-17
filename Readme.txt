		  SSAA 
  

	- programul are ca scop randarea unei imaginii la o rezolutie mai mare
  	pentru a a putea fi micsorata folosind o aproximare. Un pixel din imaginea
  	nou obinuta a fost format dintr-un patrat de pixeli din imaginea originala
  	avand dimensiunea de resize_factor^2. 
  	- am definit o structura imagine care contine informatii despre tip(P5/P6),
  	inaltime, lungime, valoarea maxima a unui pixel si o matrice de pixeli. In
  	cazul imaginii alb-negru matricea va fi de dimensiune inaltime*lungime, iar
  	pentru imaginile colore va contine 3*inaltime*lungime pixeli. 
  	- structura pixel contine un unsigned char ce va retine valoarea unui 
  	punct din imagine.
  	- pentru a oferi scalabilitate aplicatiei, am impartit fiecarui thread un
  	numar egal de linii din noua imagine care sa fie calculate. In cazul in 
  	care dupa resize inaltimea nu se impartea exact la resize_factor am trimis
  	ultimului thread restul liniilor ramase pentru a fi calculate.
  	- pentru imaginile alb-negru au fost pixelii din "matricea sablon"(de dimensiune
  	resize_factor^2) au fost alesi secvential. Se aplica apoi media aritmetica.
  	- pentru imaginile color "matricea sablon" contine numai pixeli de aceeasi
  	culoare, fiind apoi inmultiti cu Kernelul Gaussian.
  	- am definit copy_in si copy_out pointeri globali catre imaginiile din input
  	si output pentru a putea fi folositi in interiorul functiei de thread.
  	- la final se scrie in fisierul de out headerul in format PPM.
  	- timpi pentru scalabilitate (testat pe masinile din coada ibm_nehalem):
	


        Resize/Num_threads  				
  		 Input			 3/1      3/2      3/3      3/4
  		-lenna_bw   : 0.001818 0.000939 0.000757 0.000616
  		-lenna_color: 0.005058 0.002600 0.001868 0.001526
