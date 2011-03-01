#ifndef MEMBERSHIP_FUNCTIONS_H
#define MEMBERSHIP_FUNCTIONS_H


/* NOTE: Aquí definimos las modificaciones arbitrarias que se realizarán
 *	 sobre los parámetros iniciales de las funciones membresía
 */


/* Pendiente de las funciones membresía (parámetro 'b' de la MF tipo "bell") */
#define  SLOPE		2.0


/* Modificación del rango de datos de entrada                              *
 * RANGEXP = 1.0: se cubrirá exactamente el rango de los datos de entrada  *
 * RANGEXP < 1.0: se cubrirá menos rango                                   *
 * RANGEXP > 1.0: se cubrirá más rango                                     */
#define  RANGEXP	1.5


/* Modificación del ancho de cada función membresía  *
 * AWIDTH = 1.0: se mantiene el ancho "óptimo"       */
 #define  AWIDTH	1.06


/* Desplazamiento del centro de cada función membresía  *
 * en exactamente 'DECENTRE' unidades                   *
 * DECENTRE = 0.0: se mantiene el centrado "óptimo"     */
#define  DECENTRE	0.16


/* Modificación del solapamiento entre funciones membresía  *
 * JUNCTION = 0.0: se mantiene el solapamiento "óptimo"     *
 * JUNCTION > 0.0: aumenta   el solapamiento entre MF's     *
 * JUNCTION < 0.0: disminuye el solapamiento entre MF's     */
#define  JUNCTION	0.0


#endif
