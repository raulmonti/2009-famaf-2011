/**
*	Centronic interrupt program example 1 (in C).
*
*
* @author	E.E. Javier Gaspar + U.G. Super-Team
* @version	1.0 - October 15th, 2004
*/

#define IRQn	4		/* UART interrupt number */
#define UART	0x3F8		/* UART base port address (COM1) */
#define PIC1	0x20		/* PIC base port address */


#define DATA	(UART+0)	/* Rx buffer (rd only) */
#define IER	(UART+1)	/* Interrupt Enable Register (NOTE 1) */
#define IIR	(UART+2)	/* Interrupt Identification Register (NOTE 2) */
#define LCR	(UART+3)	/* Line Control Register (NOTE 3) */
#define LSR	(UART+5)	/* Line Status Register (NOTE 4) */

/* NOTE
 *
 *	1. _Para habilitar la recepción del UART hay que setear el bit 0 (LSB)
 *	   del IER
 *	   _Si se desea desactivar el modo de transmisión hay que resetear el
 *	   bit 1 del IER.
 *
 *	2. _Para indentificar a qué se debió la interrupción (Rx, Tx, etc)
 *	   _Si se desactivó el modo transimisón se facilita el trabajo de
 *	   recepción, pues sólo se recibirán interrupciones por recepción, y 
 *	   por ende no será necesario estudiar el IIR ante una interrupción 
 *	   para saber si fue por recepción o por transmisión.
 *
 *	3. _Maneja el funcionamiento del puerto serie UART
 *	   _Si se escribe una palabra con el bit 7 (MSB) seteado se estará
 *	   accediendo a los "Divisor Latch Numbers"
 *	   _Si se escribe una palabra con el bit 7 (MSB) en 0 se trabajará luego
 *	   con los registros de Tx/Rx y el IER en las direcciones de puerto
 *	   0x3F8 y 0x3F9 respectivamente
 *
 *	4. _El registro de estado del UART. El bit 0 (LSB) en alto voltaje
 *	   indica que el registro de recepción está lleno (para polling)
 */

/** De acá para arriba ya fue revisado para UART */

/** ### ~~~~~~~~~~~~~~~~~~~~ ### HASTA ACÁ TOQUÉ ### ~~~~~~~~~~~~~~~~~~~~ ### */

/** De acá para abajo aún está en el modo LPT */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

#include "assert.h"
#include "table.h"
#include "strin.h"
#include "timer.h"


#ifdef __cplusplus
	#define __CPPARGS ...
#else
	#define __CPPARGS
#endif

/* char * ingresado por teclado */
#define STRLEN  21
char text[STRLEN];

/* TAD que guarda el char * ingresado */
string* win_string = NULL;
char* str_to_print = NULL; /* Lo que se manda a los display */

int base = 0, offset = 0, picaddr = 0, nxt_to_pr = 0;

/* TAD que regula la velocidad de impresión */
timer *timer_to_print = NULL;
#define DELAY   1000  /* Demora en la impresión */

void interrupt (*oldhandler)(__CPPARGS);


void interrupt lptisr (__CPPARGS) {
/* NOTE
 * En DATA escribimos el dígito a imprimir
 * Con CONTROL seleccionamos el display donde imprimirlo
 * Ver table.h para más info sobre la selección del display
 */
	int ans = 0;
	int ascii_code = 0;
	int display_code = 0;


	disable();

/* Cálculo de la información a mostrar por el display: */
	
	/* Obtenemos el código ascii del caracter a imprimir */
	ascii_code = str_to_print[base + offset];

	/* Traducimos ese caracter según la tablita */
	display_code = map_ascii[ascii_code];

	outport (DATA, display_code);		/* Output data */
	offset = (offset + 1) % DISPLAY_SIZE;	/* Update offset */
	
/* Selección del display por donde se muestra esa información: */
	
	/* NOTE
	 * Lo que sigue es para activar con el reg. CONTROL el display indicado
	 * El inport con la máscara se hace para no deshabilitar por error
	 * la IRQ de la LPT --> ver "Enable parallel port IRQ" en el main
	 */
	ans = inport (CONTROL) & 0x10;
	outport (CONTROL, ans | dir_order[offset]); /* See port02.c */
	
/* Actualización de la información a mostrar por el display: */
	
	inc_timer (timer_to_print);
	if (timeout_timer (timer_to_print)) {
		/* Reinicializamos la cuenta */
		start_timer (timer_to_print);

		nxt_to_pr = (nxt_to_pr+1) % strlen(text);
		str_to_print = string_slice_right (win_string, nxt_to_pr, DISPLAY_SIZE);
	}

	outport(picaddr,0x20);	/* PIC end of interrupt */

	enable();
}


int main(int argc,char* argv[])
{
	int intno = 0, picmask = 0;

	intno = IRQn + 8;
	picaddr = PIC1;
	picmask = 1;
	picmask = picmask << IRQn; /* 1000 0000 == 0x80 */

	/* Para que no se queje el compilador */
	argc = argc;
	argv = argv;
	
	/* Make sure port is in forward direction */
	outport (CONTROL, inp (CONTROL)&0xDF);
	outport (DATA,0xFF);

	/* Save old interrupt vector */
	oldhandler = getvect (intno);

	/* Set new interrupt vector entry */
	disable ();
	setvect (intno,lptisr);
	enable ();

	/* Un-Mask Pic */
	outport (picaddr+1, inp (picaddr+1)&(0xFF-picmask));

	/* Enable parallel port IRQ's */
	outport (CONTROL, inp(CONTROL)|0x10);


	/* --- Setup the string to print --- */
	printf ("Please enter your message (up to 20 letters): ");
	scanf ("%[^\n]", text);  /* Tomamos hasta el \n sin incluirlo */
	win_string = string_create (text);
	ASSERT (win_string != NULL);

	/* Activamos el TAD String con su ventanita */
	str_to_print = string_get_front (win_string, 0, DISPLAY_SIZE);
	free (str_to_print);

	/* Generamos el timer que va a ser manejado por el interrupt_handler */
	timer_to_print = setup_timer (DELAY);
	start_timer (timer_to_print);

	/* Main program */
	printf ("Interrupt is enabled. Main program prints some values.\n");
	for (;!kbhit();) {
	  printf("base = %i\toffset = %i\tvalue[b+o] = 0x%02X\t",
		 base,offset,map_ascii[base+offset]);
	  printf("STATUS = 0x%X\r",inp(STATUS)&0x40);
	}

	/* Disable parallel port IRQ's */
	outport (CONTROL, inp (CONTROL)&0xEF);

	/* Mask Pic */
	outport (picaddr+1, inp (picaddr+1)|picmask);

	/* Restore old interrupt vector before exit */
	disable ();
	setvect (intno, oldhandler);
	enable ();

	/* Output null data */
	outport (DATA, 0x00);

	/* Free all resources */
	win_string = string_destroy (win_string);
	free (str_to_print);
	timer_to_print = stop_timer (timer_to_print);

	/* Quit program */
	printf ("\nExit interrupt successfully. Quit program\n");

	return 0;
}


