//-----------------------------LICENSE NOTICE------------------------------------
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

// ver. 1.2 rev.3 (10/05/18)

#include <cpctelera.h>			// funciones propias de CPCtelera

#include "tiles.h"				// tiles de 4x4 px para componer la pantalla
#include "sprites/robbie.h"		// frames del sprite principal (y novieta) de 16x16 px
#include "sprites/gusano.h"		// frames del gusano en movimiento de 16x16 px
#include "sprites/pulgon.h"		// frames del pulgón en movimiento de 16x16 px
#include "sprites/avispa.h"		// frames de la avispa en movimiento de 16x16 px
#include "sprites/explosion.h"	// 2 sprites de explosiones de 16x16 px
#include "sprites/disparo.h"	// 4 sprites de disparo de 8x8 px
#include "sprites/objetos.h"	// 6 objetos de 10x16 px
#include "sprites/planta.h"		// 12 sprites para formar la planta de 12x16 px cada uno
#include "marco.h"				// 8 sprites de 2x2 px para construir marcos en los marcadores
#include "fuente.h"				// letras y números (8x8 px)
#include "fuente2.h"			// números con fondo azul para puntos en pantalla (8x8 px)
#include "rob_img.h"			// imagen de robbie para el marcador (80x50 px)
#include "logo.h"				// logo del juego para el menú inicial (106x36 px)
#include "mapa0.h"				// pantalla de juego
#include "sonido.h"				// música y efectos de sonido













// ******************************************************************************************
// ****************************** DEFINICIONES Y VARIABLES **********************************
// ******************************************************************************************

// direcciones de inicio de los dos buffers de video
u8* const scr_buf[2] = { (u8*)0xC000, (u8*)0x8000 };

// constantes SI/NO para hacer más legible el código 
#define SI 1
#define NO 0

// valores máximos y mínimos de cada eje XY hasta donde pueden moverse los sprites
#define X_MIN 8
#define X_MAX 64
#define Y_MIN 5
#define Y_MAX 120

#define PLANTA_X 37 // posición X donde crece la planta
#define PLANTA_ITER 32 // ritmo de crecimiento de la planta (1 cada X iteraciones)

// tamaño de la pantalla de juego en tiles
#define MAP_W 40
#define MAP_H 36

#define FNT_W 4 // ancho de los caracteres de texto en bytes
#define FNT_H 8 // alto de los caracteres de texto en px

#define SPR_W 8 // ancho de Robbie/insectos en bytes
#define SPR_H 16 // alto de Robbie/insectos en px

// 4 tipos distintos de personajes
#define ROBBIE 0
#define GUSANO 1
#define PULGON 2
#define AVISPA 3

#define OBJ_W 5 // ancho de los objetos en bytes
#define OBJ_H 16 // alto de los objetos en px
#define N_MAX_OBJ 6 // número total de objetos a gestionar

#define PLT_W 6 // ancho de cada sección de la planta en bytes
#define PLT_H 16 // alto de cada sección de la planta en px

#define DSP_W 4 // ancho del disparo en bytes
#define DSP_H 8 // alto del disparo en px

u16 puntos; 		// puntuación de la partida actual
u16 max_puntos;		// puntuación máxima de toda la sesión
u8 ctr_iter; 		// contador de iteraciones del bucle principal
u8 n_nivel; 		// nivel/etapa actual de juego
u8 n_pasadas;		// veces que se han pasado los 5 niveles (aumentará la velocidad cada pasada)
u8 n_spray; 		// insecticida en uso; 0 = verde, 1 = rojo, 2 = amarillo
u8 turno_obj;		// solo se interactua con objetos si este contador = 0
u8 planta_y;		// alto de la planta ("Y" es menor cuanto más alta es la planta)
u8 comiendo;		// algún insecto está comiendo si comiendo = 1 ó "SI"		
u8 valor_rand;		// valor aleatorio de 0 a 255
u8 semilla_rand;	// semilla de aleatoriedad (0 a 255)
u8 extra_grow;		// aumenta x 2 el crecimiento de la planta si > 0
u8 ajuste;			// variable para alterar posiciones XY según determinadas circunstancias
u8 musica;			// "SI" = reproduce la música durante el juego, "NO" = solo efectos

// controles para teclado/joystick
cpct_keyID ctl_arriba;
cpct_keyID ctl_abajo;
cpct_keyID ctl_izda;
cpct_keyID ctl_dcha;
cpct_keyID ctl_disp;
cpct_keyID ctl_abort;
cpct_keyID ctl_musica;  


// estructuras

typedef struct // posibles posiciones iniciales para los insectos y objetos
{
	u8 x;
	u8 y;
	u8 libre; // la posición está libre? SI/NO
} TPosIni;

// cinco posiciones a la izquierda (de 0 a 4) y cinco a la derecha (de 5 a 9)
TPosIni pos_ini[10];

typedef struct // frame del sprite a pintar
{
	//u8 dir;
	u8* spr;
} TFrm;

typedef struct // gestión de los sprites (Robbie e insectos)
{
	u8 x[3];	// coordenadas X del sprite. [0]actual, [1]previa [2]anterior a la previa.
	u8 y[3];	// coordenadas Y del sprite. [0]actual, [1]previa [2]anterior a la previa.
	u8 est;		// estado actual; parado, en movimiento, etc...
	TFrm* frm;	// imagen de la secuencia de la animación
	u8 n_frm;	// número de frame de la animación
	u8 dir;		// dirección del sprite; derecha, izda., arriba, abajo...
	u8 vidas;	// vidas restantes
	u8 ident;	// identificador del sprite (0:ROBBIE 1:GUSANO 2:PULGON 3:AVISPA)
	u8 pausa;	// ciclos que lleva el sprite enemigo en pausa
	u8 tocado;	// ciclos que lleva el sprite tocado (para animar explosiones)
} TSpr;

TSpr spr[5];	// 0) robbie
				// 1) sprite enemigo #1
				// 2) sprite enemigo #2
				// 3) sprite enemigo #3
				// 4) sprite enemigo #4

typedef struct // estructura necesaria para controlar los disparos
{
	u8 x[3];	// coordenadas X del sprite. [0]actual, [1]previa [2]anterior a la previa.
	u8 y;		// coordenada Y del disparo
	u8 dir;		// dirección del disparo; derecha o izda.
	u8 est;		// estado de Robbie al disparar; parado, en movimiento, etc...
	u8 act;		// el disparo está activo si act = 1 ó "SI"
	u8 ret;		// retardo, el disparo solo se reactiva si ret = 0
} TDsp;

TDsp dsp, dsp2; // dos disparos a la vez en pantalla

typedef struct // estructura necesaria para controlar los objetos a recoger
{
	u8 x[3];	// coordenadas X del objeto. [0]actual, [1]previa [2]anterior a la previa.
	u8 y[3];	// coordenada Y del objeto. [0]actual, [1]previa [2]anterior a la previa.
	u8 pos_ini;	// posición de salida
	u8 visible;	// visible en pantalla? SI/NO (los insecticidas siempre serán visibles)
} TObj; 

TObj obj[N_MAX_OBJ];	// 0) insecticida verde
						// 1) insecticida rojo
						// 2) insecticida amarillo
						// 3) matamoscas
						// 4) regadera
						// 5) fertilizante


// enumeraciones

// dirección del movimiento que llevan los sprites
enum 
{
	M_dcha = 0, 
	M_izda, 
	M_arriba, 
	M_abajo, 
	M_dcha_arriba, 
	M_dcha_abajo, 
	M_izda_arriba, 
	M_izda_abajo
} enum_dir;

// estados de los sprites
enum 
{
	E_parado = 0, 
	E_andando, 
	E_disparando, 
	E_tocado, 
	E_comiendo
} enum_est;


// animaciones

#define ANIM_PAUSA 3 // ciclos por cada cambio de frame
#define SPR0_N_FRAMES 3 // nº de frames para el movimiento de Robbie
#define SPR_N_FRAMES 4 // nº de frames para el movimiento de los enemigos
#define ANDAR_N_FRAMES 2 // frames necesarios para la animación andar

// 3 frames de Robbie de 16*16; 
const TFrm frm_robbie[SPR0_N_FRAMES] = {{g_robbie_0}, {g_robbie_1}, {g_robbie_2}};
// 2 dcha. + 2 izda. frames de gusanos en movimiento de 16*16
const TFrm frm_gusano[SPR_N_FRAMES] = {{g_gusano_0}, {g_gusano_1}, {g_gusano_2}, {g_gusano_3}};
// 2 dcha. + 2 izda. frames de pulgones en movimiento de 16*16
const TFrm frm_pulgon[SPR_N_FRAMES] = {{g_pulgon_0}, {g_pulgon_1}, {g_pulgon_2}, {g_pulgon_3}};
// 2 dcha. + 2 izda. frames de avispas en movimiento de 16*16
const TFrm frm_avispa[SPR_N_FRAMES] = {{g_avispa_0}, {g_avispa_1}, {g_avispa_2}, {g_avispa_3}};

// secuencia de animaciones
TFrm* const anim_robbie[ANDAR_N_FRAMES] = {&frm_robbie[1], &frm_robbie[2]};
TFrm* const anim_gusano_dcha[ANDAR_N_FRAMES] = {&frm_gusano[0], &frm_gusano[1]};
TFrm* const anim_gusano_izda[ANDAR_N_FRAMES] = {&frm_gusano[2], &frm_gusano[3]};
TFrm* const anim_pulgon_dcha[ANDAR_N_FRAMES] = {&frm_pulgon[0], &frm_pulgon[1]};
TFrm* const anim_pulgon_izda[ANDAR_N_FRAMES] = {&frm_pulgon[2], &frm_pulgon[3]};
TFrm* const anim_avispa_dcha[ANDAR_N_FRAMES] = {&frm_avispa[0], &frm_avispa[1]};
TFrm* const anim_avispa_izda[ANDAR_N_FRAMES] = {&frm_avispa[2], &frm_avispa[3]};















// **********************************************************************************************
// ******************************** DECLARACIÓN PREVIA DE FUNCIONES *****************************
// **********************************************************************************************

void gestionarObjetos();
void borrarObjeto(TObj *p_obj, u8 n_pos, u8 n_buf);
void iniciarDisparo(TDsp *p_dsp) __z88dk_fastcall;
void borrarDisparo(TDsp *p_dsp, u8 n_pos, u8 n_buf);
void pintarPuntuacion(TSpr *p_spr) __z88dk_fastcall;
void explosionarRobbie();
void explosionarEnemigos();
void siguienteNivel();
void gameOver();
void cpct_scanKeyboard_if();
















// ***********************************************************************************************
// ************************************ FUNCIONES DE USO GENÉRICO ********************************
// ***********************************************************************************************

// player para la música de Arkos tracker
void playmusic() 
{
   __asm 
      exx
      .db #0x08
      push af
      push bc
      push de
      push hl
      call _cpct_akp_musicPlay
      pop hl
      pop de
      pop bc
      pop af
      .db #0x08
      exx
   __endasm;
}


// cada 5 llamadas, hace sonar la música y/o FX y lee el teclado
void interrupcion()  
{
   static u8 n_inter;

   if (++n_inter == 5) 
   {
      playmusic();
      cpct_scanKeyboard_if();
      n_inter = 0;
   }
}


// devuelve el código de tecla pulsada o cero
unsigned char compruebaTeclas(const cpct_keyID* k, u8 numk)
{
   u8 i;
   if (cpct_isAnyKeyPressed()) 
   {
      for(i=1; i <= numk; i++, k++) 
      {
         if (cpct_isKeyPressed(*k))
            return i;
      }
   }
   return 0;
}


// obtiene la longitud de una cadena
u8 strlen(const unsigned char *str) __z88dk_fastcall 
{
    const unsigned char *s;
    for (s = str; *s; ++s);
    return (s - str);
}


// convierte a ASCII un entero (Lukás Chmela / GPLv3)
char* itoa(u16 value, char* result, int base) 
{    
    int tmp_value;
    char* ptr = result, *ptr1 = result, tmp_char;
    
    if (base < 2 || base > 36)
    { 
        *result = '\0'; 
        return result; 
    }
    
    do 
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while (value);
    
    if (tmp_value < 0) 
        *ptr++ = '-';
    *ptr-- = '\0';
    
    while(ptr1 < ptr) 
    {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return result;
}


// pinta una cadena de texto convertida previamente de un número
void pintarNumero(u16 num, u8 lon, u8 x, u8 y, u8 n_buf, u8 fuente)
{ 
	u8 cad[6];
	u8 ceros;
	u8 pos = 0;
	u8 n_aux;

	itoa(num, cad, 10);    
	ceros = lon - strlen(cad);
	n_aux = cad[pos];

	while(n_aux != '\0')
	{
		if (fuente == 2) // números para zona de juego
			cpct_drawSprite(g_fuente2[n_aux - 48], cpct_getScreenPtr(scr_buf[n_buf], (ceros + pos) * FNT_W + x, y), 
							FNT_W, FNT_H);
		else // números para marcadores
 			cpct_drawSprite(g_fuente[n_aux - 48], cpct_getScreenPtr(scr_buf[n_buf], (ceros + pos) * FNT_W + x, y), 
 							FNT_W, FNT_H);
		n_aux = cad[++pos];
	}
}


// pinta una cadena de caracteres en las coordenadas XY
void pintarTexto(u8 txt[], u8 x, u8 y)
{
	u8 pos = 0;
	u8 car = txt[pos];

 	while(car != '\0')
	{    
		// "@" = espacio en blanco    ";" = "-"    "?" = "!!"
		cpct_drawSprite(g_fuente[car - 48], cpct_getScreenPtr(scr_buf[1], (pos * FNT_W) + x, y), FNT_W, FNT_H);
		car = txt[++pos];
	}
}


// genera una pequeña pausa
void microPausa(u16 value) __z88dk_fastcall
{
    u16 i;
    for(i=0; i < value; i++);      
}


// pone a cero la memoria de video y el backbuffer
void limpiarBuffersVideo()
{
	cpct_memset(scr_buf[0], cpct_px2byteM0(1, 1), 16384);
	cpct_memset(scr_buf[1], cpct_px2byteM0(1, 1), 16384);
}


// pasa el contenido del backbuffer (1) al buffer de video, mostrándolo en pantalla
void volcarBuffer(u8** scr_buf) __z88dk_fastcall
{
	u8* aux;
	cpct_setVideoMemoryPage((u16)(scr_buf[1]) >> 10);
	aux = scr_buf[0];
	scr_buf[0] = scr_buf[1];
	scr_buf[1] = aux;
}


// pinta la pantalla de juego
void pintarMapa()
{
	// le pasamos el ancho y alto en tiles, la posición de memoria inicial y el array de tiles a pintar 
	cpct_etm_drawTilemap2x4(MAP_W, MAP_H, cpct_getScreenPtr(scr_buf[1], 0, 0), mapa0);
}


// obtiene el número de tile del mapa a partir de una posición XY
u8* obtenerTilePtr(u8 x, u8 y)
{
	return mapa0 + y / 4 * MAP_W + x / 2;
}


// obtiene el nº de posición inicial más cercana, a partir de unas coordenadas XY
u8 obtener_pos_ini(u8 x, u8 y)
{
	u8 pos_ini = 4;

	if (y > 123) pos_ini = 0;
	else if (y > 95) pos_ini = 1;
	else if (y > 67) pos_ini = 2;
	else if (y > 39) pos_ini = 3;
	// si X > 40 las posiciones son las de la derecha (5 a 9)
	if (x > 40) pos_ini += 5;

	return pos_ini;
}















// ********************************************************************************************
// **************************** FUNCIONES PARA EL MANEJO DE SPRITES ***************************
// ********************************************************************************************

// pintamos el sprite en las coordenadas XY actuales
void pintarSprite(TSpr *p_spr) __z88dk_fastcall
{
	cpct_drawSprite(p_spr->frm->spr, 
					cpct_getScreenPtr(scr_buf[1], p_spr->x[0], p_spr->y[0]), 
					SPR_W, SPR_H);
}


// borramos el sprite con un cuadrado del color del fondo en el buffer pasado como parámetro
void borrarSprite(TSpr *p_spr, u8 n_pos, u8 n_buf)
{
	cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], p_spr->x[n_pos], p_spr->y[n_pos]), 
					  cpct_px2byteM0(6,6), 
					  SPR_W, SPR_H);	
}


// desplazamos las coordenadas actuales a la anterior para que los dos buffers
// se sincronicen correctamente
void actualizarCoordenadasSprite(TSpr *p_spr) __z88dk_fastcall
{
	u8* x = p_spr->x + 2;
	u8* y = p_spr->y + 2;
	*x = *(x-1); --x; *x = *(x-1); 
	*y = *(y-1); --y; *y = *(y-1);
}


// asigna el frame correspondiente a la secuencia de animación del sprite
void andandoSprite_animar(TSpr *p_spr) __z88dk_fastcall
{
	if(++p_spr->n_frm == ANDAR_N_FRAMES * ANIM_PAUSA) p_spr->n_frm = 0;
}


// devuelve TRUE si el sprite solo se posiciona sobre tiles del fondo, y no sobre plataformas
// tiles permitidos: > 4
u8 sobreFondo(u8 x, u8 y)
{
	u8 sobreFondo = SI;
	u8 i = 0;
	u8 j = 0;

	while (i <= 16 && sobreFondo == SI)
	{
		while (j <= 8 && sobreFondo == SI)
		{
			if (*obtenerTilePtr(x + j, y + i) <= 4) sobreFondo = NO;
			j += 2;
		}
		i += 4;
		j = 0;
	}
	return sobreFondo;
}


// asignamos el frame correspondiente a la secuencia de animación del sprite
void selecFrameSprite(TSpr *p_spr) __z88dk_fastcall
{
	if (ctr_iter % ANIM_PAUSA == 0)
	{
		if (p_spr->ident == ROBBIE)
		{
			p_spr->frm = anim_robbie[p_spr->n_frm / ANIM_PAUSA];
		}
		// animaciones de enemigos andando hacia la derecha
		else if (p_spr->dir == M_dcha || p_spr->dir == M_dcha_arriba || p_spr->dir == M_dcha_abajo)
		{
			if (p_spr->ident == GUSANO) p_spr->frm = anim_gusano_dcha[p_spr->n_frm / ANIM_PAUSA];
			else if (p_spr->ident == PULGON) p_spr->frm = anim_pulgon_dcha[p_spr->n_frm / ANIM_PAUSA];
			else p_spr->frm = anim_avispa_dcha[p_spr->n_frm / ANIM_PAUSA];
		}
		else // animaciones de enemigos andando hacia la izquierda
		{
			if (p_spr->ident == GUSANO) p_spr->frm = anim_gusano_izda[p_spr->n_frm / ANIM_PAUSA];
			else if (p_spr->ident == PULGON) p_spr->frm = anim_pulgon_izda[p_spr->n_frm / ANIM_PAUSA];
			else p_spr->frm = anim_avispa_izda[p_spr->n_frm / ANIM_PAUSA];
		}
	}
}


// pintamos las distintas fases de una explosión en las coordenadas XY del sprite
void pintarExplosion(TSpr *p_spr, u8 n_pos, u8 n_buf, u8 n_fase)
{
	cpct_drawSprite(g_explosion[n_fase], 
					cpct_getScreenPtr(scr_buf[n_buf], p_spr->x[n_pos], p_spr->y[n_pos]), 
					SPR_W, SPR_H);
}


// verifica si se ha producido alguna colisión del sprite principal o disparo con los enemigos
void gestionarColisiones(TSpr *p_spr) 
{
	u8 disparoAcertado = NO;

	// colisión entre sprites
	if ((spr[0].x[0] + SPR_W) > (p_spr->x[0] + 2) && (spr[0].x[0] + 2) < (p_spr->x[0] + SPR_W))
	{  
		if ((spr[0].y[0] + SPR_H) > (p_spr->y[0] + 2) && (spr[0].y[0] + 2) < (p_spr->y[0] + SPR_H))
		{
			// un enemigo ha tocado a robbie
			spr[0].vidas--;
			explosionarRobbie();
			explosionarEnemigos();
			gameOver();
		}
	}
	else
	{
		// colisión disparo 1 con sprites?
		if (dsp.act == SI)
		{
			if (dsp.x[0] > p_spr->x[0] && dsp.x[0] < (p_spr->x[0] + SPR_W))
				if (dsp.y > (p_spr->y[0] - DSP_H) && dsp.y < (p_spr->y[0] + SPR_H))
				{
					// acertó el disparo...
					disparoAcertado = SI;
					// borra el disparo de ambos buffers y lo elimina
					dsp.act = NO;
					borrarDisparo(&dsp, 1, 1);
					borrarDisparo(&dsp, 0, 0);
				}
		}
		// colisión disparo 2 con sprites?
		else if (dsp2.act == SI)
		{
			if (dsp2.x[0] > p_spr->x[0] && dsp2.x[0] < (p_spr->x[0] + SPR_W))
				if (dsp2.y > (p_spr->y[0] - DSP_H) && dsp2.y < (p_spr->y[0] + SPR_H))
				{
					// acertó el disparo...
					disparoAcertado = SI;
					// borra el disparo de ambos buffers y lo elimina
					dsp2.act = NO;
					borrarDisparo(&dsp2, 1, 1);	
					borrarDisparo(&dsp2, 0, 0);
				}
		}
		// si uno de los dos disparos ha acertado...
		if (disparoAcertado == SI)
		{			
			// insecticida verde mata a gusano; rojo mata a pulgón; amarillo mata a avispa 
			if ((n_spray == 0 && p_spr->ident == GUSANO) ||
				(n_spray == 1 && p_spr->ident == PULGON) ||
				(n_spray == 2 && p_spr->ident == AVISPA))
			{
				cpct_akp_SFXPlay (4, 15, 40, 0, 0, AY_CHANNEL_C); // sonido explosión
				p_spr->vidas = 0;
				p_spr->pausa = 0;
				p_spr->tocado = 20;
				p_spr->est = E_tocado;
				// actualiza puntuación de la partida actual
				if (p_spr->ident == GUSANO)	puntos += 35;
				else if (p_spr->ident == PULGON) puntos += 50;
				else puntos += 75;
				// actualiza máxima puntuación
				if (max_puntos < puntos) max_puntos = puntos;
			}
			// insecticida rojo pausa a gusano; amarillo pausa a pulgón; verde pausa a avispa
			else if ((n_spray == 1 && p_spr->ident == GUSANO) ||
					 (n_spray == 2 && p_spr->ident == PULGON) ||
					 (n_spray == 0 && p_spr->ident == AVISPA))
			{
				// aumenta el contador decreciente de pausa
				p_spr->pausa = 150;
			}	
		}
	}
}















// ***********************************************************************************************
// ************************ FUNCIONES ESPECÍFICAS PARA EL CONTROL DE ROBBIE **********************
// ***********************************************************************************************

// asignamos un frame o secuencia de frames a cada estado
void selecFrameSPR0()
{
	switch(spr[0].est) 
	{
		case E_parado:		{spr[0].frm = &frm_robbie[0]; break;}
		case E_andando:		{selecFrameSprite(&spr[0]); break;}
		case E_disparando:	{spr[0].frm = &frm_robbie[0];}
	}
}


// mueve Robbie a la derecha
void moverDerechaSPR0() 
{
	// cambia la dirección a la derecha
	spr[0].dir = M_dcha;
	// tope valor X
	ajuste = X_MAX;
	// si lleva un insecticida, le resta el ancho del insecticida al tope X
	if (n_spray < 3) ajuste -= OBJ_W;
	// si no ha llegado al extremo derecho aumenta X
	if (spr[0].x[0] < ajuste) spr[0].x[0]++;
	// si ha llegado al extremo derecho y el hueco está libre
	else if (pos_ini[obtener_pos_ini(spr[0].x[0], spr[0].y[0] + 20)].libre == SI)
	{
		if (n_spray < 3) // si lleva el insecticida
			gestionarObjetos(); // deja el insecticida
		else
			if (sobreFondo(spr[0].x[0], spr[0].y[0])) spr[0].x[0]++; // sigue aumentando X
	}
	// el hueco está ocupado, recoge el objeto
	else gestionarObjetos();
}


// mueve Robbie a la izquierda
void moverIzquierdaSPR0() 
{
	// cambia la dirección a la izquierda
	spr[0].dir = M_izda;
	// tope valor X por la izda.
	ajuste = X_MIN;
	// si lleva un insecticida, le suma el ancho del insecticida al tope X
	if (n_spray < 3) ajuste += OBJ_W;
	// si no ha llegado al extremo izquierdo disminuye X
	if (spr[0].x[0] > ajuste) spr[0].x[0]--;
	// si ha llegado al extremo izquierdo y el hueco está libre
	else if (pos_ini[obtener_pos_ini(spr[0].x[0], spr[0].y[0] + 20)].libre == SI)
	{
		if (n_spray < 3) // si lleva insecticida
			gestionarObjetos(); // deja el insecticida
		else
			if (sobreFondo(spr[0].x[0] - 1, spr[0].y[0])) spr[0].x[0]--; // sigue disminuyendo X
	}
	// si el hueco está ocupado, recoge el objeto
	else gestionarObjetos();
}


// mueve Robbie hacia arriba
void moverArribaSPR0() 
{
	ajuste = 0;
	if (spr[0].dir == M_dcha) ajuste++;

	if (n_spray < 3) // lleva un insecticida
	{
		// toma como referencia la posición del insecticida
		if (sobreFondo(obj[n_spray].x[0] - (4 * ajuste), obj[n_spray].y[0] - 2)) spr[0].y[0] -= 2;	
		// cuidado con el techo, se queda en Y_MIN si se pasa
		if (spr[0].y[0] < Y_MIN) spr[0].y[0]++;
	}
	else // toma como referencia la posición de Robbie
		if (sobreFondo(spr[0].x[0] - ajuste, spr[0].y[0] - 2)) spr[0].y[0] -= 2;
}


// mueve Robbie hacia abajo
void moverAbajoSPR0() 
{
	ajuste = 0;
	if (spr[0].dir == M_dcha) ajuste++;

	if (n_spray < 3) // lleva un insecticida
	{
		// toma como referencia la posición del insecticida
		if (sobreFondo(obj[n_spray].x[0] - (4 * ajuste), obj[n_spray].y[0] + 2)) spr[0].y[0] += 2;	
	}
	else // toma como referencia la posición de Robbie
		if (sobreFondo(spr[0].x[0] - ajuste, spr[0].y[0] + 2)) spr[0].y[0] += 2;
}


// prepara el movimiento a izda. o derecha
void andandoSPR0Entrar(u8 dir) __z88dk_fastcall
{
	spr[0].n_frm = 0;
	spr[0].est = E_andando;
	if (dir == M_izda || dir == M_dcha) 
		spr[0].dir = dir;
}


// prepara el disparo
void prepDisparoSPR0Entrar() 
{
	// comprueba que hay espacio suficiente a izda. o derecha antes de disparar
	if ((spr[0].dir == M_izda && spr[0].x[0] > 17) || (spr[0].dir == M_dcha && spr[0].x[0] < 56))
	{
		// si no hay disparo1 en pantalla y el disparo2 ya lleva 10 ciclos activo...
		if (dsp.act == NO && dsp2.ret == 0)
		{
			iniciarDisparo(&dsp);
			dsp.ret = 10;
		}
		// si no hay disparo2 en pantalla y el disparo1 ya lleva 10 ciclos activo...
		else if (dsp2.act == NO && dsp.ret == 0)
		{
			iniciarDisparo(&dsp2);
			dsp2.ret = 10;
		}
	}
}


// se queda parado 
void paradoSPR0() 
{
	cpct_scanKeyboard_f(); // comprueba las teclas pulsadas
	if(cpct_isKeyPressed(ctl_arriba)) andandoSPR0Entrar(M_arriba);
	else if(cpct_isKeyPressed(ctl_abajo)) andandoSPR0Entrar(M_abajo);
	else if(cpct_isKeyPressed(ctl_izda)) andandoSPR0Entrar(M_izda);
	else if(cpct_isKeyPressed(ctl_dcha)) andandoSPR0Entrar(M_dcha);
	else if(cpct_isKeyPressed(ctl_disp) && n_spray < 3) prepDisparoSPR0Entrar();
	// abandona la partida
	else if(cpct_isKeyPressed(ctl_abort))
	{
		spr[0].vidas = 0; 
		explosionarRobbie();
		explosionarEnemigos();
		gameOver();
	}
	// silenciar música SI/NO
	else if(cpct_isKeyPressed(ctl_musica) && turno_obj == 0)
	{
		turno_obj = 30;

		if (musica == SI) // Si la música está sonando...
		{
			musica = NO;
			cpct_akp_musicInit(FX);
		}
		else // si no había música sonando...
		{
			musica = SI;
			if (n_nivel % 2 == 0)
				cpct_akp_musicInit(Ingame2); // niveles 2-4
			else
				cpct_akp_musicInit(Ingame1); // niveles 1-3-5
		}
	}
}


// prepara el disparo
inline void disparandoSPR0() 
{
	// sonido de disparo según el insecticida
	if (n_spray == 0)
		cpct_akp_SFXPlay (1, 15, 40, 0, 0, AY_CHANNEL_C); // verde, psssst
	else if (n_spray == 1)
		cpct_akp_SFXPlay(2, 15, 40, 0, 0, AY_CHANNEL_C); // rojo, laser
	else
		cpct_akp_SFXPlay(3, 15, 32, 0, 0, AY_CHANNEL_C); // amarillo, gotas

	// devolvemos a Robbie el estado que tenía antes de disparar
	spr[0].est = dsp.est; 
}


// andando
void andandoSPR0() 
{
	cpct_scanKeyboard_f(); // comprueba las teclas pulsadas
	// posibles movimientos a la derecha
	if(cpct_isKeyPressed(ctl_dcha))
	{
		moverDerechaSPR0();
		if (cpct_isKeyPressed(ctl_arriba)) moverArribaSPR0();  // diagonal derecha-arriba
		else if(cpct_isKeyPressed(ctl_abajo)) moverAbajoSPR0(); // diagonal derecha-abajo
		andandoSprite_animar(&spr[0]);
		if(cpct_isKeyPressed(ctl_disp) && n_spray < 3) // dispara si tiene insecticida 
			prepDisparoSPR0Entrar();
	}
	// posibles movimientos a la izquierda
	else if(cpct_isKeyPressed(ctl_izda))
	{
		moverIzquierdaSPR0();
		if (cpct_isKeyPressed(ctl_arriba)) moverArribaSPR0(); // diagonal izda-arriba
		else if(cpct_isKeyPressed(ctl_abajo)) moverAbajoSPR0(); // diagonal izda-abajo
		andandoSprite_animar(&spr[0]);
		if(cpct_isKeyPressed(ctl_disp) && n_spray < 3) // dispara si tiene insecticida
			prepDisparoSPR0Entrar();
	}
	else if(cpct_isKeyPressed(ctl_arriba)) // solo arriba
	{
		moverArribaSPR0(); 
		andandoSprite_animar(&spr[0]); 
		if(cpct_isKeyPressed(ctl_disp) && n_spray < 3) // dispara si tiene insecticida
			prepDisparoSPR0Entrar();
	}
	else if(cpct_isKeyPressed(ctl_abajo)) // solo abajo
	{
		moverAbajoSPR0(); 
		andandoSprite_animar(&spr[0]);
		if(cpct_isKeyPressed(ctl_disp) && n_spray < 3) // dispara si tiene insecticida
			prepDisparoSPR0Entrar();
	}
	else
		spr[0].est = E_parado;
}


// llama a la función apropiada según el estado del sprite principal
void ejecutarEstadoSPR0() 
{
	switch(spr[0].est) 
	{
		case E_parado:       paradoSPR0();			break;
		case E_andando:      andandoSPR0();			break;
		case E_disparando:   disparandoSPR0();
	}
}


// elimina a Robbie mediante una explosión
void explosionarRobbie()
{
	// para visualizar el choque, muestra explosiones con pausas
	cpct_akp_SFXPlay (4, 15, 40, 0, 0, AY_CHANNEL_C); // sonido explosión
	pintarExplosion(&spr[0], 2, 0, 0); microPausa(5000);
	pintarExplosion(&spr[0], 2, 0, 1); microPausa(5000);
	pintarExplosion(&spr[0], 2, 0, 0); microPausa(5000);
	// eliminamos todos los restos de ambos buffers
	borrarSprite(&spr[0], 2, 1);
	borrarSprite(&spr[0], 2, 0); 
	borrarSprite(&spr[0], 1, 1); 
	borrarSprite(&spr[0], 1, 0);
	borrarSprite(&spr[0], 0, 1);
	borrarSprite(&spr[0], 0, 0); 
	// si lleva un insecticida, lo borra también
	if (n_spray < 3)
	{
		borrarObjeto(&obj[n_spray], 2, 1);
		borrarObjeto(&obj[n_spray], 2, 0);
		borrarObjeto(&obj[n_spray], 1, 1);
		borrarObjeto(&obj[n_spray], 1, 0);
		borrarObjeto(&obj[n_spray], 0, 1);
		borrarObjeto(&obj[n_spray], 0, 0);
	}
}















// ***********************************************************************************************
// ******************** FUNCIONES ESPECÍFICAS PARA EL CONTROL DE LOS ENEMIGOS ********************
// ***********************************************************************************************

// actualiza las coordenadas XY de los enemigos diagonales (gusanos y pulgones)
void moverSpriteDiagonal(TSpr *p_spr, u8 vel)
{
	// a la derecha y abajo
	if (p_spr->dir == M_dcha_abajo) 
	{
		if (p_spr->x[0] < X_MAX) // si queda sitio a la derecha mueve el sprite
		{
			p_spr->x[0]++;
			// si queda sitio abajo mueve el sprite
			if (p_spr->y[0] < Y_MAX) p_spr->y[0] += vel;
			else p_spr->dir = M_dcha_arriba;
		}
		else // si no hay sitio cambia el sentido
			p_spr->dir = M_izda_abajo;
	}
	// a la derecha y arriba
	else if (p_spr->dir == M_dcha_arriba)
	{
		if (p_spr->x[0] < X_MAX) // si queda sitio a la derecha mueve el sprite
		{
			p_spr->x[0]++;
			// si queda sitio arriba, mueve el sprite
			if (p_spr->y[0] > Y_MIN) p_spr->y[0] -= vel;
			else p_spr->dir = M_dcha_abajo;
		}
		else // si no hay sitio cambia el sentido
			p_spr->dir = M_izda_arriba;
	}
	// a la izquierda y arriba
	else if (p_spr->dir == M_izda_arriba)
	{
		if (p_spr->x[0] > X_MIN) // si queda sitio a la izquierda mueve el sprite 
		{
			p_spr->x[0]--;
			// si queda sitio arriba, mueve el sprite
			if (p_spr->y[0] > Y_MIN) p_spr->y[0] -= vel;
			else p_spr->dir = M_izda_abajo;
		}
		else // si no hay sitio cambia el sentido
			p_spr->dir = M_dcha_arriba; 
	}
	// a la izquierda y abajo
	else if (p_spr->dir == M_izda_abajo)
	{
		if (p_spr->x[0] > X_MIN) // si queda sitio a la izquierda mueve el sprite 
		{
			p_spr->x[0]--;
			// si queda sitio abajo y no está sobre una plataforma, mueve el sprite
			if (p_spr->y[0] < Y_MAX) p_spr->y[0] += vel;
			else p_spr->dir = M_izda_arriba;
		}
		else // si no hay sitio cambia el sentido
			p_spr->dir = M_dcha_abajo; 
	}
}


// actualiza las coordenadas XY de los enemigos perseguidores (avispas)
void moverSpritePerseguidor(TSpr *p_spr) __z88dk_fastcall
{
	if (p_spr->x[0] <= spr[0].x[0]) // está a la izquierda de Robbie
	{
		if (p_spr->x[0] < X_MAX)
		{
			p_spr->x[0]++;
			p_spr->dir = M_dcha;
		}
	}
	else // está a la derecha de Robbie
	{
		if (p_spr->x[0] > X_MIN) p_spr->x[0]--;
		if (p_spr->x[0] != spr[0].x[0]) p_spr->dir = M_izda;
	}
	// está arriba/abajo de Robbie
	if (p_spr->y[0] <= spr[0].y[0])	p_spr->y[0]++; else p_spr->y[0]--;
}


// actualiza las coordenadas XY de los enemigos en función de su tipo de movimiento
void moverSprite(TSpr *p_spr) __z88dk_fastcall
{
	//si toca la planta detiene al sprite (no actualiza XY)
	if  ( p_spr->y[0] > planta_y && 
		((p_spr->x[0] == 42 && (p_spr->dir == M_izda || p_spr->dir == M_izda_abajo || p_spr->dir == M_izda_arriba)) ||
		( p_spr->x[0] == 30 && (p_spr->dir == M_dcha || p_spr->dir == M_dcha_abajo || p_spr->dir == M_dcha_arriba))))
	{
		p_spr->est = E_comiendo;
		comiendo = SI;
	}
	else
	{
		if (p_spr->ident == GUSANO && ctr_iter % (5 - n_pasadas) == 0) moverSpriteDiagonal(p_spr, 1);
		else if (p_spr->ident == PULGON && ctr_iter % (3 - n_pasadas) == 0) moverSpriteDiagonal(p_spr, 2);
		else if (p_spr->ident == AVISPA && ctr_iter % (7 - n_pasadas) == 0) moverSpritePerseguidor(p_spr);
		p_spr->est = E_andando;
		comiendo = NO;	
	}
}


// asigna propiedades iniciales a los enemigos
void paramEnems(u8 n_spr, u8 ident)
{
	spr[n_spr].est = E_andando;
	spr[n_spr].dir = M_dcha_arriba;
	spr[n_spr].vidas = 0; 
	spr[n_spr].ident = ident; 
	spr[n_spr].pausa = 0;
	spr[n_spr].tocado = 0;
}


// da valores a las propiedades de los enemigos dependiendo del nivel
void inicializarEnemigos()
{
	switch(n_nivel) 
	{
		case 1: // solo gusanos
		{
			//        SPR IDENTIDAD
			paramEnems(1, GUSANO);
			paramEnems(2, GUSANO);
			paramEnems(3, GUSANO);
			paramEnems(4, GUSANO);
			break;
		}
		case 2: // gusanos y pulgones
		{
			//        SPR IDENTIDAD
			paramEnems(1, GUSANO);
			paramEnems(2, GUSANO);
			paramEnems(3, PULGON);
			paramEnems(4, PULGON);
			break;
		}
		case 3: // gusanos y avispas
		{
			//        SPR IDENTIDAD
			paramEnems(1, GUSANO);
			paramEnems(2, GUSANO);
			paramEnems(3, AVISPA);
			paramEnems(4, AVISPA);
			break;
		}
		case 4: // avispas y pulgones
		{
			//        SPR IDENTIDAD
			paramEnems(1, AVISPA);
			paramEnems(2, AVISPA);
			paramEnems(3, PULGON);
			paramEnems(4, PULGON);
			break;
		}
		case 5: // todos
		{
			//        SPR IDENTIDAD
			paramEnems(1, GUSANO);
			paramEnems(2, AVISPA);
			paramEnems(3, PULGON);
			paramEnems(4, PULGON);
		}
	}
}


// lógica de los sprites enemigos para cada iteración del bucle principal
void gestionarSprEnemBucle(TSpr *p_spr) __z88dk_fastcall
{
	if (p_spr->vidas == 1) // Si el enemigo está vivo
	{
		// actualiza las coordenadas XY del sprite o va decreciendo el contador de pausa
		if (p_spr->pausa == 0) moverSprite(p_spr); else p_spr->pausa--;
		// selecciona el frame de la animación y lo aplica	
		selecFrameSprite(p_spr); 
		andandoSprite_animar(p_spr);
		// borra el sprite de la posición anterior y pinta en la actual
		borrarSprite(p_spr, 2, 1);
		pintarSprite(p_spr);
		actualizarCoordenadasSprite(p_spr);
		// verifica si se ha producido alguna colisión
		gestionarColisiones(p_spr);
	}
	else if (p_spr->tocado > 0) // enemigo alcanzado, va a explotar..
	{
		if (p_spr->tocado == 20) {borrarSprite(p_spr, 0, 1); borrarSprite(p_spr, 0, 0);}
		else if (p_spr->tocado > 18) pintarExplosion(p_spr, 2, 1, 0);
		else if (p_spr->tocado > 16) pintarExplosion(p_spr, 2, 1, 1);
		else if (p_spr->tocado > 14) pintarExplosion(p_spr, 2, 1, 0);
		else if (p_spr->tocado > 12) {borrarSprite(p_spr, 0, 1); borrarSprite(p_spr, 2, 1);}
		else pintarPuntuacion(p_spr); // muestra la puntuación en la zona de juego
		p_spr->tocado--;
	}
	else if (p_spr->est == E_tocado) // en este punto ha muerto y explotado...
	{
		p_spr->est = E_andando;
		// borra el sprite de las últimas posiciones y de ambos buffers		
		borrarSprite(p_spr, 0, 1); 
		borrarSprite(p_spr, 0, 0);
	}
}


// cada X ciclos, recorre el array de enemigos para ir resucitándolos. 
// La posición de salida es aleatoria pero dependiente de la situación de Robbie en pantalla
void generarEnems()
{
	// contador para bucles
	u8 ctr;
	// enemigo a generar
	u8 z = 1;
	// los siguientes valores delimitan las posiciones de salida
	u8 limites_pos_ini[11] = {0, 25, 51, 77, 103, 129, 154, 180, 205, 230, 255};
	// evita que dos enemigos salgan de la misma posición
	u8 pos_ini_usadas[10] = {NO, NO, NO, NO, NO, NO, NO, NO, NO, NO};

	// recorre los enemigos para comprobar si hay que revivir alguno
	while (z < 5)
	{
		if (spr[z].vidas == 0 && spr[z].est == E_andando) // enemigo inactivo
		{
			// genera una posición de salida
			valor_rand = cpct_getRandom_lcg_u8(semilla_rand);
			if (valor_rand == 255) valor_rand = 254;

			// asigna una posición de salida para el enemigo según el valor obtenido
			//
			// ctr valor_rand posición
			// === ========== ========
			//   0    0 a  24  1 izda.
			//   1   25 a  50  2 izda.		  ---------------------
			//   2   51 a  76  3 izda.		 |  5               10 |
			//   3   77 a 102  4 izda.		 |  4                9 |
			//   4  103 a 128  5 izda.       |  3                8 |
			//   5  129 a 153  6 dcha.       |  2                7 |
			//   6  154 a 179  7 dcha.       |  1                6 |
			//   7  180 a 204  8 dcha.        ---------------------
			//   8  205 a 229  9 dcha.
			//   9  230 a 254 10 dcha.	
			for (ctr = 0; ctr < 10; ctr++)
			{
				if (valor_rand >= limites_pos_ini[ctr] && valor_rand < limites_pos_ini[ctr+1])
				{
						// si no ha salido ya antes otro enemigo en esa posición...
					if 	(pos_ini_usadas[ctr] == NO && 
						// y no sale cercano a Robbie en el eje de las X...
						((spr[0].x[0] > (pos_ini[ctr].x + 18) || spr[0].x[0] < (pos_ini[ctr].x - 20)) ||
						// o no sale cercano a Robbie en el eje de las Y...
						(spr[0].y[0] > (pos_ini[ctr].y + 40) || spr[0].y[0] < (pos_ini[ctr].y - 44))))						
					{
						// X+-8 para no borrar los objetos en los huecos
						if (valor_rand < 129) // lado izquierdo
							spr[z].x[0] = spr[z].x[1] = spr[z].x[2] = pos_ini[ctr].x + 8;
						else // lado derecho
							spr[z].x[0] = spr[z].x[1] = spr[z].x[2] = pos_ini[ctr].x - 8;
						//el valor de Y es común para ambos lados
						spr[z].y[0] = spr[z].y[1] = spr[z].y[2] = pos_ini[ctr].y;
						//revivimos al enemigo
						spr[z].vidas = 1;
						spr[z].dir = M_dcha_arriba;
						pos_ini_usadas[ctr] = SI;
						break;
					}
				}
			}
		}
		z++;
	}	
}


// elimina a todos los enemigos en pantalla mediante una explosión
void explosionarEnemigos()
{
	u8 ctr;

	for (ctr = 1; ctr < 5; ctr++)
	{
		if (spr[ctr].vidas > 0)
		{
			cpct_akp_SFXPlay (4, 15, 40, 0, 0, AY_CHANNEL_C); // sonido de explosión
			pintarExplosion(&spr[ctr], 2, 0, 0); microPausa(5000);
			pintarExplosion(&spr[ctr], 2, 0, 1); microPausa(5000);
			pintarExplosion(&spr[ctr], 2, 0, 0); microPausa(5000);
			spr[ctr].vidas = 0;
			spr[ctr].est = E_andando;
			// eliminamos todos los restos de ambos buffers
			borrarSprite(&spr[ctr], 2, 1);
			borrarSprite(&spr[ctr], 2, 0); 
			borrarSprite(&spr[ctr], 1, 1); 
			borrarSprite(&spr[ctr], 1, 0);
			borrarSprite(&spr[ctr], 0, 1);
			borrarSprite(&spr[ctr], 0, 0); 
		}
	}

	// idem con los disparos
	if (dsp.act == SI)
	{ 
		borrarDisparo(&dsp, 1, 1); 
		borrarDisparo(&dsp, 0, 0);
	}
	if (dsp2.act == SI)
	{
		borrarDisparo(&dsp2, 1, 1); 
		borrarDisparo(&dsp2, 0, 0);
	}
}















// **************************************************************************************************
// ****************************** FUNCIONES PARA LA GESTIÓN DE DISPAROS *****************************
// **************************************************************************************************

// actualiza los parámetros del disparo que se va a iniciar
void iniciarDisparo(TDsp *p_dsp) __z88dk_fastcall
{
	p_dsp->act = SI;
	p_dsp->dir = spr[0].dir; // el sentido del disparo es el de Robbie

	// ajusta a la altura del insecticida
	p_dsp->y = spr[0].y[0] - 1;

	// da al disparo la coordenada X de Robbie, sin que borre el insecticida
	if (p_dsp->dir == M_dcha)
		p_dsp->x[0] = p_dsp->x[1] = p_dsp->x[2] = spr[0].x[0] + SPR_W + OBJ_W;
	else
		p_dsp->x[0] = p_dsp->x[1] = p_dsp->x[2] = spr[0].x[0] - DSP_W - OBJ_W;

    // guardamos el estado de Robbie antes de disparar, para que luego continúe igual
	p_dsp->est = spr[0].est;
	spr[0].est = E_disparando;
}


// desplazamos las coordenadas actuales del disparo a la posición anterior 
// para que los dos buffers se actualicen correctamente
void actualizarCoordenadasDisparo(TDsp *p_dsp) __z88dk_fastcall
{
	u8* x = p_dsp->x + 2;
	*x = *(x-1); --x; *x = *(x-1);
}


// pintamos un recuadro del color del fondo en las coordenadas del disparo (para borrarlo)
void borrarDisparo(TDsp *p_dsp, u8 n_pos, u8 n_buf)
{
	cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], p_dsp->x[n_pos], p_dsp->y), 
					  cpct_px2byteM0(6,6), DSP_W, DSP_H);	
}


// pinta el disparo en las nuevas coordenadas
void pintarDisparo(TDsp *p_dsp) __z88dk_fastcall
{
	if (p_dsp->act == SI && n_spray < 3) // si el disparo está activo y llevamos insecticida
	{
		// borra el anterior disparo del backbuffer
		borrarDisparo(p_dsp, 2, 1);

		if (n_spray < 2) // los disparos del insecticida #0 y #1 son independientes de la dirección
			cpct_drawSprite(g_disparo[n_spray], 
							cpct_getScreenPtr(scr_buf[1], p_dsp->x[0], p_dsp->y), 
							DSP_W, DSP_H);
		else // el disparo del spray #3 (gotas) depende de la dirección
			cpct_drawSprite(g_disparo[n_spray + p_dsp->dir], 
							cpct_getScreenPtr(scr_buf[1], p_dsp->x[0], p_dsp->y), 
							DSP_W, DSP_H); 
	}
}


// mueve el disparo a la posición XY correspondiente si está activo
void moverDisparo(TDsp *p_dsp) __z88dk_fastcall
{
	actualizarCoordenadasDisparo(p_dsp);
	// cambiamos la posición X para el siguiente redibujado
	if (p_dsp->dir == M_dcha) p_dsp->x[0] += 2;	else p_dsp->x[0] -= 2;
	// verifica si debemos seguir dibujando el disparo (ha llegado a los extremos)
	// +-7 pixels para que el disparo no borre los posibles objetos en los extremos de la pantalla
	if (p_dsp->x[0] + DSP_W >= 73 || p_dsp->x[0] <= 7)
	{
		//borra los disparos anteriores de ambos buffers
		borrarDisparo(p_dsp, 2, 1); borrarDisparo(p_dsp, 1, 0);
		// desactiva el disparo
		p_dsp->act = NO;
	}
}















// ***********************************************************************************************
// **************************** FUNCIONES PARA LA GESTIÓN DE LA PLANTA ***************************
// ***********************************************************************************************

// pinta la sección de planta en el backbuffer en las coordenadas XY dadas
void pintarPlanta(u8 ident, u8 x, u8 y)
{
	cpct_drawSprite(g_planta[ident], cpct_getScreenPtr(scr_buf[1], x, y), PLT_W, PLT_H);
}


// pintamos un recuadro con el color de fondo en determinadas coordenadas de la planta (para borrarla)
void borrarPlanta(u8 n_buf) __z88dk_fastcall
{
	// tallo
	cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], PLANTA_X, planta_y), 
					  cpct_px2byteM0(6,6), PLT_W, PLT_H);
	// hoja derecha
	if (planta_y < 80)
		cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], 41, 92), 
						  cpct_px2byteM0(6,6), PLT_W, PLT_H);	
	// hoja izquierda	
	else if (planta_y < 94)
		cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], 33, 106), 
						  cpct_px2byteM0(6,6), PLT_W, PLT_H);
}


void pintarPlantaNivel1()
{
	pintarPlanta(0, PLANTA_X, 120); // tallo
	pintarPlanta(1, PLANTA_X, planta_y); // capullo
}


void pintarPlantaNivel2()
{
	pintarPlanta(0, PLANTA_X, 106); // tallo
	pintarPlantaNivel1(); // tallo + capullo
}


void pintarPlantaNivel3()
{
	pintarPlanta(0, PLANTA_X, 92); // tallo
	pintarPlantaNivel2(); // tallo + tallo + capullo
	if (planta_y < 86) 
		pintarPlanta(2, 33, 106); //hoja grande izquierda
	else
		pintarPlanta(7, 33, 106); //hoja pequeña izquierda
}


void pintarPlantaNivel4()
{
	pintarPlanta(0, PLANTA_X, 78); //tallo
	pintarPlantaNivel3(); // tallo + tallo + tallo + capullo
	if (planta_y < 72)
		pintarPlanta(6, 41, 92); //hoja grande derecha
	else
		pintarPlanta(8, 41, 92); //hoja pequeña derecha	
}


// pinta toda la planta, o parte, en función de planta_y
void repintarPlanta()
{
	if (planta_y < 62) 
	{
		//borra a los enemigos y a Robbie de la pantalla
		explosionarEnemigos();
		explosionarRobbie();

		// la planta florece
		pintarPlantaNivel4();
		pintarPlanta( 3, PLANTA_X - 6, 54);	
		pintarPlanta( 4, PLANTA_X,     54);	
		pintarPlanta( 5, PLANTA_X + 6, 54);	
		pintarPlanta( 9, PLANTA_X - 6, 70);	
		pintarPlanta(10, PLANTA_X,     70);
		pintarPlanta(11, PLANTA_X + 6, 70);
		siguienteNivel();
	}
	else if (planta_y < 79) pintarPlantaNivel4();
	else if (planta_y < 93) pintarPlantaNivel3();
	else if (planta_y < 107) pintarPlantaNivel2();
	else if (planta_y < 121) pintarPlantaNivel1();
}


void gestionarPlanta()
{
	u8 ctr;
	comiendo = NO;

	// por cada enemigo...
	for (ctr = 1; ctr < 5; ctr++)
		// si algún insecto está comiendo disminuye la planta
		if (spr[ctr].est == E_comiendo)
		{
			// borra la parte superior de la planta de ambos buffers
			borrarPlanta(1); borrarPlanta(0);
			planta_y++;
			// baja la posición Y del insecto junto con la planta, hasta el límite Y_MAX
			spr[ctr].y[0]++;
			if (spr[ctr].y[0] > Y_MAX) spr[ctr].y[0] = Y_MAX;
			// hay al menos un insecto comiendo
			comiendo = SI;
		}

	// si ningún insecto está comiendo aumenta la planta
	if (comiendo == NO)
	{
		planta_y--;
		// crecimiento extra por obtención de objetos
		if (extra_grow > 0)
		{
			planta_y--;
			extra_grow--;
		}
	}
	
	// si la planta ha llegado al límite inferior muestra un mensaje y perdemos una vida
	else if (planta_y >= Y_MAX && comiendo == SI)
	{
		spr[0].vidas--;
		explosionarRobbie();
		explosionarEnemigos();
		pintarTexto("@@@@@@@@@@@@@@@@", 8, 62);
		pintarTexto("@@@@@@@@@@@@@@@@", 8, 75);
		pintarTexto("@@@@@@@@@@@@@@@@", 8, 88);
		pintarTexto("@YOUR@PLANT@HAS@", 8, 70);
		pintarTexto("@@BEEN@EATEN@?@@", 8, 80);
		volcarBuffer(scr_buf);
		microPausa(50000);
		microPausa(50000);
		microPausa(50000);
		gameOver();
	}
}















// ***********************************************************************************************
// ***************************** FUNCIONES PARA LA GESTIÓN DE OBJETOS ****************************
// ***********************************************************************************************

// pintamos el objeto en las coordenadas XY pasadas como parámetros
void pintarObjeto(u8 ident, u8 x, u8 y, u8 n_buf)
{
	cpct_drawSprite(g_objetos[ident], 
					cpct_getScreenPtr(scr_buf[n_buf], x, y), 
					OBJ_W, OBJ_H);
}


// pintamos un recuadro con el color de fondo en las coordenadas del objeto (para borrarlo)
void borrarObjeto(TObj *p_obj, u8 n_pos, u8 n_buf)
{
	cpct_drawSolidBox(cpct_getScreenPtr(scr_buf[n_buf], p_obj->x[n_pos], p_obj->y[n_pos]), 
					  cpct_px2byteM0(6,6), 
					  OBJ_W, OBJ_H);	
}


// desplazamos las coordenadas actuales a la anterior para que los dos buffers
// se actualicen correctamente
void actualizarCoordenadasObjeto(TObj *p_obj) __z88dk_fastcall
{
	u8* x = p_obj->x + 2; 
	u8* y = p_obj->y + 2;
	*x = *(x-1); --x; *x = *(x-1); 
	*y = *(y-1); --y; *y = *(y-1);
}


// repinta el insecticida que lleva Robbie en las nuevas coordenadas
void pintarSprayActivo()
{
	// borramos el insecticida de la posición anterior
	borrarObjeto(&obj[n_spray], 2, 1);
	// damos al insecticida las coordenadas actuales de Robbie, con alguna variación en X
	obj[n_spray].y[0] = spr[0].y[0];
	// si vamos hacia la izquierda y no hemos llegado al extremo
	if (spr[0].dir == M_izda)
	{
		if (spr[0].x[0] > 12) 
			obj[n_spray].x[0] = spr[0].x[0] - 5;
	}
	// si vamos hacia la derecha y no hemos llegado al extremo
	else
	{
		if (spr[0].x[0] < 60)
			obj[n_spray].x[0] = spr[0].x[0] + 8;
	}
	// pintamos en las nuevas coordenadas
	pintarObjeto(n_spray, obj[n_spray].x[0], obj[n_spray].y[0], 1);
	actualizarCoordenadasObjeto(&obj[n_spray]);
}


// repinta los insecticidas al comenzar partida
void pintarSprays()
{
	u8 z = 0;
	while (z < 3)
	{
		pintarObjeto(z, obj[z].x[0], obj[z].y[0], 1);
		z++;
	}
}


// Gestiona la recogida/entrega de objetos
void gestionarObjetos()
{
	u8 ctr;
	u8 n_pos_ini;

	if (turno_obj == 0) // si tenemos turno para interactuar con objeto...
	{
		ajuste = 7;
		
		// obtenemos el nº de hueco más cercano
		n_pos_ini = obtener_pos_ini(spr[0].x[0], spr[0].y[0] + 20);
		
		// si llevamos un insecticida...
		if (n_spray < 3)
		{
			if (pos_ini[n_pos_ini].libre == SI) // si el hueco está libre...
			{
				// borramos el insecticida de ambos buffers
				borrarObjeto(&obj[n_spray], 2, 1); borrarObjeto(&obj[n_spray], 1, 0);
				// ocupamos el hueco
				pos_ini[n_pos_ini].libre = NO; 
				// damos al insecticida la posición y nº del hueco
				obj[n_spray].x[0] = obj[n_spray].x[1] = obj[n_spray].x[2] = pos_ini[n_pos_ini].x; 
				obj[n_spray].y[0] = obj[n_spray].y[1] = obj[n_spray].y[2] = pos_ini[n_pos_ini].y;
				obj[n_spray].pos_ini = n_pos_ini;
				// sonido de dejar un objeto
				cpct_akp_SFXPlay(7, 15, 45, 0, 0, AY_CHANNEL_C);
				// pintamos el insecticida en la nueva posición
				pintarObjeto(n_spray, obj[n_spray].x[0], obj[n_spray].y[0], 1); 
				pintarObjeto(n_spray, obj[n_spray].x[0], obj[n_spray].y[0], 0);
				// ya no llevamos insecticida
				n_spray = 255;
				// ponemos el turno a 20 para no interactuar con más objetos hasta pasados unos ciclos
				turno_obj = 20;
				//eliminamos los disparos activos disparados con el insecticida dejado
				if (dsp.act == SI)
				{ 
					borrarDisparo(&dsp, 1, 1); 
					borrarDisparo(&dsp, 0, 0);
				}
				if (dsp2.act == SI)
				{
					borrarDisparo(&dsp2, 1, 1); 
					borrarDisparo(&dsp2, 0, 0);
				}
			}
		}
		else // si no llevamos insecticida
		{
			ctr = 0;
			while (ctr < N_MAX_OBJ) // procesa todos los objetos
			{
				if (obj[ctr].visible == SI) // si el objeto está en la pantalla...
				{
					if (spr[0].dir == M_dcha) ajuste += SPR_W;
					// si nos situamos en la proximidad del objeto...
					if (spr[0].x[0] >= obj[ctr].x[0] - ajuste && spr[0].x[0] <= obj[ctr].x[0] + ajuste &&	
						spr[0].y[0] >= obj[ctr].y[0] - 4 && spr[0].y[0] <= obj[ctr].y[0])
					{
						// sonido de coger un objeto
						cpct_akp_SFXPlay(8, 15, 45, 0, 0, AY_CHANNEL_C);
						// dejamos libre la posición de salida
						pos_ini[n_pos_ini].libre = SI;
						// ponemos el turno a 20 para no interactuar con más objetos hasta pasados unos ciclos
						turno_obj = 20;						
						if (ctr > 2) // si es un objeto que no se guarda, matamoscas, regadera o fertilizante
						{
							// borramos el objeto del mapa en ambos buffers
							borrarObjeto(&obj[ctr], 2, 1); 
							borrarObjeto(&obj[ctr], 2, 0);
							// ponemos el objeto como recogido para no volver a interactuar con él
							obj[ctr].visible = NO;
							// aumentamos la puntuación
							puntos += 100;
							if (max_puntos < puntos) max_puntos = puntos;
							// premios adicionales
							if (ctr == 3) explosionarEnemigos(); // matamoscas elimina enemigos
							else if (ctr == 4) extra_grow = 5; // regadera aumenta crecimiento x 2 cinco veces
							else extra_grow = 10; // fertilizante aumenta crecimiento x 2 diez veces
						}
						else // si es un insecticida...
						{
							// ponemos el insecticida en uso
							n_spray = ctr;
						}
						break;
					}
				}
				ctr++;
			}
		}
	}
}


// asigna propiedades a un objeto
inline void paramObj(u8 n_obj, u8 x, u8 y, u8 pos_ini, u8 visible)
{
	obj[n_obj].x[0] = obj[n_obj].x[1] = obj[n_obj].x[2] = x; 
	obj[n_obj].y[0] = obj[n_obj].y[1] = obj[n_obj].y[2] = y;
	obj[n_obj].pos_ini = pos_ini;
	obj[n_obj].visible = visible;
}


// reinicializa las propiedades de los objetos
void inicializarObjetos()
{
	/*	0) Spray verde
		1) Spray rojo
		2) Spray amarillo
		------------------
		3) Matamoscas
		4) Regadera
		5) Fertilizante */

	//      OBJ X  Y  POS  VISIBLE
	paramObj(0, 0, 0,   4, SI);
	paramObj(1, 0, 0,   8, SI);
	paramObj(2, 0, 0,   1, SI);
	paramObj(3, 0, 0, 255, NO);
	paramObj(4, 0, 0, 255, NO);
	paramObj(5, 0, 0, 255, NO);
}


// vuelve a poner el insecticida que está en uso, en la anterior posición
// cuando se reinicia la partida
void recolocarSpray(u8 n_obj) __z88dk_fastcall
{
	paramObj(n_obj, pos_ini[obj[n_obj].pos_ini].x, pos_ini[obj[n_obj].pos_ini].y, obj[n_obj].pos_ini, SI);
	pos_ini[obj[n_obj].pos_ini].libre = NO;
}


void generarObjetos()
{
	// contador para bucles
	u8 ctr;
	// objeto a generar
	u8 z = 0;
	// los siguientes valores delimitan las posiciones iniciales
	u8 limites_pos_ini[11] = {0, 25, 51, 77, 103, 129, 154, 180, 205, 230, 255};
	
	// si Robbie está en los extremos laterales no generamos objetos, para que no se superpongan los sprites,
	// así además evitamos que el jugador campée en algún rincón
	if (spr[0].x[0] > X_MIN && spr[0].x[0] < X_MAX - SPR_W)
	{
		// selecciona el objeto de manera aleatoria si no está en pantalla
		valor_rand = cpct_getRandom_lcg_u8(semilla_rand + 1);
		if (valor_rand < 86 && obj[3].visible == NO) z = 3; // matamoscas
		else if ((valor_rand > 85 && valor_rand < 171) && obj[4].visible == NO) z = 4; // regadera
		else if (valor_rand > 170 && obj[5].visible == NO) z = 5; // fertilizante

		if (z > 0) // si tenemos objeto
		{
			valor_rand = cpct_getRandom_lcg_u8(semilla_rand + 2);
			if (valor_rand == 255) valor_rand = 254;

			// asigna una posición para el objeto según el valor obtenido
			//
			// ctr valor_rand posición
			// === ========== ========
			//   0    0 a  24  1 izda.
			//   1   25 a  50  2 izda.		  ---------------------
			//   2   51 a  76  3 izda.		 |  5               10 |
			//   3   77 a 102  4 izda.		 |  4                9 |
			//   4  103 a 128  5 izda.       |  3                8 |
			//   5  129 a 153  6 dcha.       |  2                7 |
			//   6  154 a 179  7 dcha.       |  1                6 |
			//   7  180 a 204  8 dcha.        ---------------------
			//   8  205 a 229  9 dcha.
			//   9  230 a 254 10 dcha.	
			for (ctr = 0; ctr < 10; ctr++)
			{
				if (valor_rand >= limites_pos_ini[ctr] && valor_rand < limites_pos_ini[ctr+1] && 
					pos_ini[ctr].libre == SI)
				{
					obj[z].visible = SI;
					obj[z].x[0] = obj[z].x[1] = obj[z].x[2] = pos_ini[ctr].x;
					obj[z].y[0] = obj[z].y[1] = obj[z].y[2] = pos_ini[ctr].y;
					pos_ini[ctr].libre = NO;
					break;
				}
			}

			if (obj[z].visible == SI) // si tenemos posición para el objeto lo pintamos en ambos buffers
			{
				cpct_akp_SFXPlay (6, 15, 41, 0, 0, AY_CHANNEL_C); // sonido de evento (nuevo objeto)
				pintarObjeto(z, obj[z].x[0], obj[z].y[0], 0);
				pintarObjeto(z, obj[z].x[0], obj[z].y[0], 1);
			}
		}
	}	
}















// ****************************************************************************************
// ************************ FUNCIONES PARA LA GESTIÓN DEL MARCADOR ************************
// ****************************************************************************************

// pintamos la porción de marco en las coordenadas XY pasadas como parámetros
void pintarMarco(u8 ident, u8 x, u8 y)
{
	cpct_drawSprite(g_marco[ident],	cpct_getScreenPtr(scr_buf[1], x, y), 1, 2);
}


// pintamos los marcos blancos para separar y decorar marcadores
void pintarMarcos()
{
	u8 ctr;

	// esquinas redondeadas
	pintarMarco(0, 0, 148);
	pintarMarco(1, 38, 148);
	pintarMarco(2, 0, 197);
	pintarMarco(3, 38, 197);
	pintarMarco(0, 40, 148);
	pintarMarco(1, 79, 148);

	// líneas horizontales
	for(ctr = 1; ctr < 38; ctr++) 
	{
		pintarMarco(4, ctr, 147);
		pintarMarco(5, ctr, 198);
	}
	for(ctr = 41; ctr < 79; ctr++) 
		pintarMarco(4, ctr, 147);

	// líneas verticales
	for(ctr=150; ctr < 197; ctr+=2)
	{
		pintarMarco(6, 0, ctr);
		pintarMarco(7, 38, ctr);
	}
	for(ctr=150; ctr < 180; ctr+=2) 
		pintarMarco(6, 40, ctr);
	for(ctr=150; ctr < 192; ctr+=2) 
		pintarMarco(7, 79, ctr);
}


// pinta la zona del marcador
void inicializarMarcador()
{
	// puntuación
	pintarTexto("SCORE", 2,  151);
	pintarTexto("00000", 2,  162);
	// puntuación máxima
	pintarTexto("HIGH", 2,  177);
	pintarTexto("00000", 2, 188);
	// nivel
	pintarTexto("L", 30,  188);
	// pinta las imágenes de robbie
	cpct_drawSprite(g_rob_img, cpct_getScreenPtr(scr_buf[1], 40, 150), G_ROB_IMG_W, G_ROB_IMG_H);
	cpct_drawSprite(g_robbie_3, cpct_getScreenPtr(scr_buf[1], 26, 154), SPR_W, SPR_H);
	// pinta los marcos blancos para separar los marcadores
	pintarMarcos();
}


// pinta los datos cambiantes del marcador
// al no actualizar durante todos los ciclos del bucle principal hay que pintar en ambos buffers
void actualizarMarcador()
{
	// backbufer
	pintarNumero(puntos, 5, 2, 162, 1, 1); // puntuación actual
	pintarNumero(max_puntos,  5, 2, 188, 1, 1); // puntuación máxima
	pintarNumero(spr[0].vidas, 1, 34, 162, 1, 1); // número de vidas
	pintarNumero(n_nivel, 1, 34, 188, 1, 1); // nivel
	// video
	pintarNumero(puntos, 5, 2, 162, 0, 1); // puntuación actual
	pintarNumero(max_puntos,  5, 2, 188, 0, 1); // puntuación máxima
	pintarNumero(spr[0].vidas, 1, 34, 162, 0, 1); // número de vidas
	pintarNumero(n_nivel, 1, 34, 188, 0, 1); // nivel
}


// pintamos la puntuación obtenida al fumigar a un enemigo en la zona de juego
void pintarPuntuacion(TSpr *p_spr) __z88dk_fastcall
{
	u8 puntos;
	if (p_spr->ident == GUSANO) puntos = 35;
	else if (p_spr->ident == PULGON) puntos = 50;
	else puntos = 75;
	pintarNumero(puntos, 2, p_spr->x[0], p_spr->y[0] + 4, 1, 2);
}















// ***********************************************************************************************
// ***************************************** MENU PRINCIPAL **************************************
// ***********************************************************************************************

// menú inicial
void pintarMenu()
{
	cpct_drawSprite(g_logo, cpct_getScreenPtr(scr_buf[1], 13, 0), G_LOGO_W, G_LOGO_H);
	pintarTexto("STRIKES@BACK?", 14, 40);
	
	pintarTexto("1@KEYBOARD@QAOP", 10, 76);
    pintarTexto("2@KEYBOARD@WSAD", 10, 88);
    pintarTexto("3@JOYSTICK", 10, 100);
	
	pintarTexto("MUSIC@BY@BEYKER", 10, 138);
	
    pintarTexto("WRITTEN@BY", 20, 158);
    pintarTexto("SALVAKANTERO", 16,170);
	pintarTexto("REV:3@2018", 20, 192);

    // pintamos un robbie pequeño
    cpct_drawSprite(g_robbie_3, cpct_getScreenPtr(scr_buf[1], 66, 184),	SPR_W, SPR_H);
}


// asigna valores a las variables que controlan el teclado/joystick
inline void paramCtl(cpct_keyID arriba, cpct_keyID abajo, cpct_keyID izda, 
					 cpct_keyID dcha, cpct_keyID disp)
{
	ctl_arriba = arriba; 
	ctl_abajo = abajo;
	ctl_izda = izda;
	ctl_dcha = dcha;
	ctl_disp = disp;
	ctl_abort = Key_X;
	ctl_musica = Key_M;
}


// pinta el menú y gestiona las pulsaciones del teclado
void menu()
{
	// inicializa la música, tema principal 
	cpct_akp_musicInit(Menu);

	limpiarBuffersVideo(); 
	pintarMenu(); volcarBuffer(scr_buf); pintarMenu();
	semilla_rand = 0;

	while(1)
	{
		// obtenemos la semilla de aleatoriedad en función del tiempo que tardemos en pulsar una tecla
		if (++semilla_rand == 253) semilla_rand = 0;

		cpct_scanKeyboard_f();
   	
   		if(cpct_isKeyPressed(Key_1)) // teclado, combinación por defecto QAOP-X-SPC
   		{ 
   			paramCtl(Key_Q, Key_A, Key_O, Key_P, Key_Space);
        	break;
    	}
   		else if(cpct_isKeyPressed(Key_2)) // teclado, combinación alternativa WSAD-X-SPC
   		{
   			paramCtl(Key_W, Key_S, Key_A, Key_D, Key_Space);
        	break;
    	}
    	else if(cpct_isKeyPressed(Key_3)) // joystick
    	{
    		paramCtl(Joy0_Up, Joy0_Down, Joy0_Left, Joy0_Right, Joy0_Fire1);
        	break;
    	}
	}
	
	// silencia la música
	cpct_akp_musicInit(FX);
	cpct_akp_SFXPlay (6, 14, 41, 0, 0, AY_CHANNEL_B); // sonido de nuevo evento (nueva partida)

	limpiarBuffersVideo();
	// muestra mensaje de inicio en la pantalla
	pintarTexto("SPRAY@THE@BUGS?", 10, 86);
	volcarBuffer(scr_buf);
	microPausa(50000);
	microPausa(50000);
	microPausa(50000);
	// música in-game para el nivel 1
	cpct_akp_musicInit(Ingame1);
}















// ***********************************************************************************************
// **************************************** BUCLE PRINCIPAL **************************************
// ***********************************************************************************************

// valores comunes para las funciones inicializarPartida() y gameOver() 
void reinicializarDatos()
{
	// contador de ciclos del bucle de juego a cero
	ctr_iter = 0;
	// desactiva los disparos en curso
	dsp.act = NO;
	dsp2.act = NO;
	// ningún insecto comiendo
	comiendo = NO;
	// no hay bonus de crecimiento de planta
	extra_grow = 0;
	// Robbie no lleva insecticida
	n_spray = 255;
	// reinicia la posición de Robbie
	spr[0].x[0] = spr[0].x[1] = spr[0].x[2] = 36;
	spr[0].y[0] = spr[0].y[1] = spr[0].y[2] = 25;
	spr[0].dir = M_dcha; spr[0].est = E_parado;
	// datos iniciales para los enemigos
	inicializarEnemigos();
	// dejamos todos los huecos disponibles
	pos_ini[0].libre = pos_ini[1].libre = pos_ini[2].libre = pos_ini[3].libre = pos_ini[4].libre = SI;
	pos_ini[5].libre = pos_ini[6].libre = pos_ini[7].libre = pos_ini[8].libre = pos_ini[9].libre = SI;
	// ponemos los objetos a NO visibles
	obj[3].visible = obj[4].visible = obj[5].visible = NO;
	// reasigna la última posición que tenía cada insecticida en la fase anterior
	recolocarSpray(0);
	recolocarSpray(1);
	recolocarSpray(2);
	// repinta la pantalla actual y los datos del marcador
	pintarMapa(); pintarSprays();
	volcarBuffer(scr_buf);
	pintarMapa(); pintarSprays();
	actualizarMarcador();
}


// inicialización de algunas variables al comenzar la partida
void inicializarPartida() 
{
	// menú de inicio;
	menu();
	// reinicia la puntuación
	puntos = 0;
	// planta al mínimo
	planta_y = 125;
	// nivel de juego inicial
	n_nivel = 1;
	n_pasadas = 0;
	// con música
	musica = SI;
	// datos iniciales para Robbie
	spr[0].ident = ROBBIE; 
	spr[0].vidas = 5;
	// posiciones de salida de enemigos y objetos
	// cálculo de coordenadas: x=(TILED(x)*4)/2  y=TILED(y)*4
	pos_ini[0].x = pos_ini[1].x = pos_ini[2].x = pos_ini[3].x = pos_ini[4].x = 3;
	pos_ini[5].x = pos_ini[6].x = pos_ini[7].x = pos_ini[8].x = pos_ini[9].x = 72;
	pos_ini[0].y = pos_ini[5].y = 120;
	pos_ini[1].y = pos_ini[6].y = 92;
	pos_ini[2].y = pos_ini[7].y = 64;
	pos_ini[3].y = pos_ini[8].y = 36;
	pos_ini[4].y = pos_ini[9].y = 8;
	// pinta el marcador
	inicializarMarcador(); volcarBuffer(scr_buf); inicializarMarcador();
	// asigna valores a otras variables
	inicializarObjetos();
	reinicializarDatos();
}


// Robbie pierde una vida, se reinicializan algunas variables
void gameOver()
{
	// si quedan vidas
	if (spr[0].vidas > 0) 
		reinicializarDatos(); // reasigna datos a algunas variables
	else // prepara una nueva partida
	{
		cpct_akp_musicInit(FX); // detiene la música
		actualizarMarcador();
		repintarPlanta();
		// pinta en el centro de la zona de juego un GAME OVER
		pintarTexto("@@@@@@@@@@@", 18, 62);
		pintarTexto("@GAME@OVER@", 18, 70);
		pintarTexto("@@@@@@@@@@@", 18, 78);
		volcarBuffer(scr_buf);
		microPausa(50000);
		microPausa(50000);
		// espera a la pulsación de una tecla para continuar 
		while (!cpct_isAnyKeyPressed());
		inicializarPartida();
	}
}


void siguienteNivel()
{
	// pinta a la novia de Robbie si está en el nivel 5
	if (n_nivel == 5)
	{
		cpct_drawSprite(g_robbie_4, cpct_getScreenPtr(scr_buf[1], 36, 45), SPR_W, SPR_H);
		cpct_drawSprite(g_robbie_5, cpct_getScreenPtr(scr_buf[1], 31, 28), SPR_W, SPR_H);
		// aumentamos la puntuación
		puntos += 1000;
		if (max_puntos < puntos) max_puntos = puntos;
	}
	// muestra todos los sprites del backbuffer
	volcarBuffer(scr_buf);
	// música de siguiente nivel
	cpct_akp_musicInit(Flor);
	// espera al término de la música
	while (cpct_akp_songLoopTimes == 0);
	
	// avanza nivel
	n_nivel++;
	if (n_nivel > 5)
	{
		n_nivel = 1;
		if (n_pasadas < 2) 
			n_pasadas++; // aumentará la velocidad de la próxima pasada
	}
	
	// reinicia algunas variables
	reinicializarDatos(); 
	planta_y = 125;

	// música in-game
	if (musica == NO) // si no queremos que suene la música
		cpct_akp_musicInit(FX);
	else
	{	// si queremos que suene la música
		if (n_nivel % 2 == 0)
			cpct_akp_musicInit(Ingame2); // niveles 2-4
		else
			cpct_akp_musicInit(Ingame1); // niveles 1-3-5
	}
}


void buclePrincipal() 
{
	// contador para bucles
	u8 ctr;
	// deshabilita el control del firmware
	cpct_disableFirmware();
	// inicializa los efectos de sonido
	cpct_akp_SFXInit(FX);
	// inicializa el gestor de interrupciones (teclado y sonido)
	cpct_setInterruptHandler(interrupcion);	
	// activa el modo 0; 160*200 16 colores
	cpct_setVideoMode(0);
	// asigna la paleta
	cpct_setPalette(g_palette, 16);
	// pinta el borde de negro (color 1 de la paleta)
	cpct_setBorder(g_palette[1]); 
	// guarda en memoria los tiles para los mapas (4*4)	
	cpct_etm_setTileset2x4(g_tileset);
	// asigna valores iniciales
	inicializarPartida();

	while (1) // bucle principal
	{
		// redibuja la planta para no ser borrada por otros sprites
		repintarPlanta();
		// llama a la función apropiada según el estado de Robbie
		ejecutarEstadoSPR0();
		// asignamos el frame siguiente de la animación a Robbie
		selecFrameSPR0();
		// borra el sprite principal
		borrarSprite(&spr[0], 2, 1);
		// cambiamos las coordenadas XY del insecticida si Robbie lo lleva
		if (n_spray < 3) pintarSprayActivo();
		// pinta a Robbie en la nueva posición XY
		pintarSprite(&spr[0]);
		actualizarCoordenadasSprite(&spr[0]);
		
		// sprites enemigos (hasta 4 en pantalla)
		for (ctr = 1; ctr < 5; ctr++)
		{
			gestionarSprEnemBucle(&spr[ctr]);
			// pausa compensatoria si el enemigo no está en pantalla
			if (spr[ctr].vidas == 0) microPausa(300);
		}
		// disparos de robbie
		if (dsp.act == SI)
		{
			// actualiza las coordenadas XY si se disparó
			moverDisparo(&dsp);
			// borra el disparo y lo pinta en la nueva posición XY
			pintarDisparo(&dsp);
		}		
		if (dsp2.act == SI)
		{
			// actualiza las coordenadas XY si se disparó
			moverDisparo(&dsp2);
			// borra el disparo y lo pinta en la nueva posición XY
			pintarDisparo(&dsp2);
		}
		// revisamos los temporizadores de sincronización entre disparos
		if (dsp.ret > 0) dsp.ret--;
		if (dsp2.ret > 0) dsp2.ret--;
		// revisamos el temporizador para interactuar con objetos
		if (turno_obj > 0) turno_obj--;
		// actualizamos el marcador cada X iteraciones del bucle
		if (ctr_iter % 15 == 0) actualizarMarcador();
		// hace crecer o decrecer la planta
		if (ctr_iter % PLANTA_ITER == 0) gestionarPlanta();
		// resucita enemigos si se cumplen los requisitos
		if (ctr_iter == 127) generarEnems();
		// aumentamos el contador de iteraciones del bucle principal
		if (++ctr_iter == 255)
		{
			//nuevamente resucita enemigos si se cumplen los requisitos
			//estamos haciendo respawn dos veces cada 255 iteraciones del bucle principal
			generarEnems();
			// hace aparecer objetos para recolectar puntos
			generarObjetos();
			// restaura el contador
			ctr_iter = 0;
		}
		// espera a la sincronización vertical y vuelca el backbuffer en el video
		cpct_waitVSYNC();
		volcarBuffer(scr_buf);
	}
}


void main(void) 
{
	// bajamos la posición de memoria de la pila para usar la zona más alta como backbuffer 
	cpct_setStackLocation((void*)0x8000);
	buclePrincipal();
}