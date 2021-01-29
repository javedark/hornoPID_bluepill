
//--> Imagenes.
void mi_marca(void);
void frame(void);
void opciones(void);

//--> Fución para limpiar la pantalla.
void limpia_pantalla(void);// Declaración.

void limpia_pantalla(void){// Definición.
  UG_FillScreen(pen_to_ug(0));
  mapa_to_oled();
}
//--> Función para limpiar la sección.
void limpia_seccion(void);

void limpia_seccion(void){
  UG_FillFrame(1,1,88,62,pen_to_ug(0));
  mapa_to_oled();
}
//--> Imagen de mi marca;
void mi_marca(void){
oled_init();
UG_Init(&gui,local_draw_point,128,64);
//Dibujo del libro
UG_DrawFrame(18,16,39,45,pen_to_ug(1));
UG_DrawLine(15,11,35,11,pen_to_ug(1));
UG_DrawLine(14,13,37,13,pen_to_ug(1));
UG_DrawLine(14,13,14,40,pen_to_ug(1));
UG_DrawLine(14,13,18,16,pen_to_ug(1));
UG_DrawLine(37,13,37,15,pen_to_ug(1));
UG_DrawLine(14,40,18,45,pen_to_ug(1));
UG_DrawPixel(14,12,pen_to_ug(1));
UG_DrawPixel(35,12,pen_to_ug(1));
//Dibujo del encabezado
UG_DrawRoundFrame(21,19,37,27,3,pen_to_ug(1));
//Dibujo del simbolo master.
UG_DrawCircle(23,30,1,pen_to_ug(1));
UG_DrawLine(23,32,23,38,pen_to_ug(1));
UG_DrawCircle(23,40,1,pen_to_ug(1));
UG_DrawLine(30,34,30,39,pen_to_ug(1));
UG_DrawCircle(30,41,1,pen_to_ug(1));
UG_DrawLine(36,34,36,40,pen_to_ug(1));
UG_DrawCircle(36,42,1,pen_to_ug(1));
UG_DrawLine(30,34,36,34,pen_to_ug(1));
UG_DrawLine(24,34,27,31,pen_to_ug(1));
UG_DrawLine(27,31,30,34,pen_to_ug(1));
UG_FontSelect(&FONT_8X12);
UG_PutString(45,30,"Master");
mapa_to_oled();
vTaskDelay(pdMS_TO_TICKS(5000));
limpia_pantalla();
}

// Dibujo del frame para la aplicación.
void frame(void){
  UG_DrawFrame(0,0,127,63,pen_to_ug(1));
  UG_DrawLine(89,0,89,63,pen_to_ug(1));
  UG_DrawLine(89,11,127,11,pen_to_ug(1));
  UG_DrawLine(89,31,127,31,pen_to_ug(1));
  UG_DrawLine(89,43,127,43,pen_to_ug(1));
  UG_FontSelect(&FONT_8X8);
  UG_PutString(91,2,"Temp");
  UG_PutString(91,33,"Lamp");
  UG_FontSelect(&FONT_8X14);
  UG_PutString(91,15,"   C"); // Coordenadas de la impresión de la temperatura
  UG_DrawCircle(114,15,2,pen_to_ug(1)); // Símbolo de grados.
  mapa_to_oled();
  vTaskDelay(200);
//  limpia_pantalla();
}

void opciones(void) {
  UG_FontSelect(&FONT_5X8);
  UG_PutString(3,6,"M. de control?");
  UG_FontSelect(&FONT_8X12);
  UG_PutString(25,25,"ON-OFF");
  UG_PutString(25,45,"PID");
  mapa_to_oled();
}
