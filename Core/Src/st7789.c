#include "st7789.h"
#include "string.h"
#include "stdlib.h"

#include "font.h"

extern SPI_HandleTypeDef hspi1;
static SPI_HandleTypeDef *hspi = &hspi1;

static DisplayRotation g_rotation = DISPLAY_ROTATION_0;
uint8_t madctl; // ʾ����0x00Ϊ������0x20Ϊ������MV=1��

#define DC_SET_LOW() 		do{HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_RESET);}while(0);
#define DC_SET_HIGH() 	do{HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_SET);}while(0);
#define CS_SET_LOW() 		do{HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);}while(0);
#define CS_SET_HIGH() 	do{HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);}while(0);
#define BLK_SET_LOW() 	do{HAL_GPIO_WritePin(BLK_PORT, BLK_PIN, GPIO_PIN_RESET);}while(0);
#define BLK_SET_HIGH() 	do{HAL_GPIO_WritePin(BLK_PORT, BLK_PIN, GPIO_PIN_SET);}while(0);
#define RST_SET_LOW() 	do{HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_RESET);}while(0);
#define RST_SET_HIGH() 	do{HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);}while(0);

// ˽�к���
static void WriteCommand(uint8_t cmd) {
	CS_SET_LOW();
	DC_SET_LOW();
	HAL_SPI_Transmit(hspi, &cmd, 1, 100);
	CS_SET_HIGH();
}

static void WriteData(uint8_t data){
	CS_SET_LOW();
	DC_SET_HIGH();
	HAL_SPI_Transmit(hspi, &data, 1, 100);
	CS_SET_HIGH();
}

static void WriteMultiData(uint8_t *data, uint16_t len) {
	CS_SET_LOW();
	DC_SET_HIGH();
	HAL_SPI_Transmit(hspi, data, len, 100);
	CS_SET_HIGH();
}
static void Swap(uint16_t *a,uint16_t *b)
{
	uint16_t temp = *a;
	*a = *b;
	*b = temp;
}
//
void ST7789_Init(void) 
{
	// Ӳ����λ
	RST_SET_LOW();
	HAL_Delay(150);
	RST_SET_HIGH();
	HAL_Delay(150);

	// ��ʼ����������
	WriteCommand(0x11); // Sleep Out
	HAL_Delay(120);

	// Frame rate control
	WriteCommand(0xB2);
	uint8_t porch_data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
	WriteMultiData(porch_data, sizeof(porch_data));

	// Display inversion off
	WriteCommand(0x20);

	// Gate control
	WriteCommand(0xB7);
	WriteData(0x56);
	
	// Power settings
	WriteCommand(0xBB); // VCOM
	WriteData(0x18);// ����VCOM��ѹ
	
	WriteCommand(0xC0); // LCM control
	WriteData(0x2C);
	
	WriteCommand(0xC2); // VDV & VRH enable
	WriteData(0x01);
	
	WriteCommand(0xC3); // VRH set
	WriteData(0x1F);
	
	WriteCommand(0xC4); // VDV set
	WriteData(0x20);
	
	WriteCommand(0xC6); // FR Control 2
	WriteData(0x0F);		
	
	WriteCommand(0xD0); // Power Control 1
	uint8_t power_data[] = {0xA6, 0xA1};
	WriteMultiData(power_data, 2);		
	
	// GammaУ��
	uint8_t gamma_positive[] = {0xD0,0x0D,0x14,0x0B,0x0B,0x07,0x3A,0x44,0x50,0x08,0x13,0x13,0x2D,0x32};
	WriteCommand(0xE0);
	WriteMultiData(gamma_positive, sizeof(gamma_positive));

	uint8_t gamma_negative[] = {0xD0,0x0D,0x14,0x0B,0x0B,0x07,0x3A,0x44,0x50,0x08,0x13,0x13,0x2D,0x32};
	WriteCommand(0xE1);
	WriteMultiData(gamma_negative, sizeof(gamma_negative));

	// �ڴ���ʿ���
	WriteCommand(0x36);
	WriteData(0x60); // ��ʾ�������� ��ͬ����ֵ��0x00, 0x60, 0xC0, 0xA0

	// �ӿ����ظ�ʽ
	WriteCommand(0x3A);
	WriteData(0x55);// 16-bit RGB565
	
	// ����SPI˫��ģʽ
	WriteCommand(0xE7);
	WriteData(0x00);

	// ��ʾ����
	WriteCommand(0x21); // ������ʾ��ɫ
	WriteCommand(0x29); // ������ʾ
	HAL_Delay(100);
}
void ST7789_SetRotation(DisplayRotation rotation) 
{
	g_rotation = rotation;
	switch(rotation) {
		case DISPLAY_ROTATION_90:
				madctl = 0x20; 
				break;
		case DISPLAY_ROTATION_180:
				madctl = 0xC0; 
				break;
		case DISPLAY_ROTATION_270:
				madctl = 0x60; 
				break;
		default: // 0��
				madctl = 0x00;
	}
	WriteCommand(0x36); // MADCTL
	WriteData(madctl);
}
void ST7789_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd) {
	
    uint8_t col_data[4], row_data[4];
		uint16_t colStart, colEnd, rowStart, rowEnd;
	
	  // Լ�������������������ֱ���
    xStart = (xStart > COL_MAX) ? COL_MAX : xStart;
    yStart = (yStart > COL_MAX) ? COL_MAX : yStart;	
	
    xEnd = (xEnd > COL_MAX) ? COL_MAX : xEnd;
    yEnd = (yEnd > COL_MAX) ? COL_MAX : yEnd;
		
		switch(g_rotation) {
			case DISPLAY_ROTATION_90:
				colStart = COL_MAX - xEnd;
				rowStart = yStart ; 
				colEnd   = COL_MAX - xStart; 			
				rowEnd	 = yEnd;			
				break;
			case DISPLAY_ROTATION_180:
				colStart = xStart;
				colEnd   = xEnd; 
				rowStart = yStart + Y_OFFSET; 
				rowEnd	 = yEnd + Y_OFFSET;
				break;
			case DISPLAY_ROTATION_270:
				colStart = xStart;
				colEnd   = xEnd; 
				rowStart = yStart; 
				rowEnd	 = yEnd;
				break;
			default: // 0��
				colStart = xStart;
				colEnd   = xEnd; 
				rowStart = yStart; 
				rowEnd	 = yEnd;
		}
		if(colStart > colEnd){ Swap(&colStart,&colEnd); }
		if(rowStart > rowEnd){ Swap(&rowStart,&rowEnd); } 
		col_data[0] = colStart >> 8; 	col_data[1] = colStart & 0xFF;
		col_data[2] = colEnd >> 8; 		col_data[3] = colEnd & 0xFF;
		
		row_data[0] = rowStart >> 8; 	row_data[1] = rowStart & 0xFF;
		row_data[2] = rowEnd >> 8; 		row_data[3] = rowEnd & 0xFF;
    // ��������
    WriteCommand(0x2A); // �е�ַ
    WriteMultiData(col_data, 4);
    WriteCommand(0x2B); // �е�ַ
    WriteMultiData(row_data, 4);
    WriteCommand(0x2C); // д������
}
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color)//���㺯��
{
	if((x > TFT_WIDTH - 1) || (y > TFT_HEIGHT - 1)) return ;
	ST7789_SetWindow(x,y,x,y);
	// ����д���ڴ�����
  WriteCommand(0x2C);
	uint8_t bufff[2] = {color >> 8, color & 0xFF};
	WriteMultiData(bufff,2);
}
void ST7789_DrawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd,uint16_t color)
{
	//Bresenham�㷨
	//ת��Ϊ�з��ű�������ֹ�޷������������
	int16_t x0 = xStart;
	int16_t y0 = yStart;
	int16_t x1 = xEnd;
	int16_t y1 = yEnd;	

	int16_t dx = abs(x1 - x0);// x�����ܲ���ȡ����ֵ
	int16_t dy = abs(y1 - y0);// y�����ܲ���
	int16_t sx = (x0 < x1) ? 1 : -1;
	int16_t sy = (y0 < y1) ? 1 : -1;
	int16_t err = dx - dy;// ��ʼ�����
	int16_t e2;
	while(1)
	{
		ST7789_DrawPixel((uint16_t)x0, (uint16_t)y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 > -dy) {
				err -= dy;
				x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}		
	}
}
void ST7789_DrawChar(uint16_t x,uint16_t y,char c, CharSize size ,uint16_t color,uint16_t bg_color) {
	if (c < 0x20 || c > 0x7E) return;
	uint8_t index = c - 0x20;
	switch(size){
		case CHAR_SIZE_1206: 
		{
			const uint8_t *char_data = char_12x6[index];
			for(int row = 0 ; row < 12 ;row++)
			{
				uint8_t row_data = char_data[row];
				for(int col = 0; col < 6 ;col++)
				{
					if(row_data & (0x01 << col))//��λ��ǰ
					{
						ST7789_DrawPixel( x + col , y + row ,color);
					}
					else if (bg_color != BACKGROUND_COLOR) {
						ST7789_DrawPixel(x + col, y + row, bg_color);
					}					
				}
			}
			break;
		}
		case CHAR_SIZE_1608:
		{
			const uint8_t *char_data = char_16x8[index];
			for (int row = 0; row < 16; row++) 
			{
				uint8_t row_data = char_data[row];
				for (int col = 0; col < 8; col++) 
				{				
					if (row_data & (0x01 << col)) { // ��λ��ǰ
						ST7789_DrawPixel(x + col, y + row, color); 
					} 
					else if (bg_color != BACKGROUND_COLOR) {
						ST7789_DrawPixel(x + col, y + row, bg_color);
					}
				}
			}
			break;
		}		
		case CHAR_SIZE_2412: //
		{
			const uint8_t *char_data = char_24x12[index];
			for( uint16_t row = 0; row < 24 ; row++)//����24��
			{
				uint8_t row_data1 = char_data[row * 2]; //��һ���ֽڣ��洢ǰ8λ����
				uint8_t row_data2 = char_data[row * 2 + 1];//�ڶ����ֽڣ��洢��4λ����
				for(uint16_t col = 0; col < 12 ; col++)
				{
					if(col < 8)
					{					
						if(row_data1 & (0x01 << col)){ST7789_DrawPixel(x + col,y + row,color);} //�ӵ�λ��ʼ����
						else if (bg_color != BACKGROUND_COLOR) {
							ST7789_DrawPixel(x + col, y + row, bg_color);
						}
					}
					else
					{					
						if(row_data2 & (0x01 << (col - 8))){ST7789_DrawPixel(x + col,y + row,color);} //�ӵ�λ��ʼ����
						else if (bg_color != BACKGROUND_COLOR) {
							ST7789_DrawPixel(x + col, y + row, bg_color);
						}				
					}
				}
			}
			break;
		}
		case CHAR_SIZE_3216:
		{	
			const uint8_t *char_data = char_32x16[index];
			for( uint16_t row = 0; row < 32 ; row++)//����32��
			{
				uint8_t row_data1 = char_data[row * 2]; //��һ���ֽڣ��洢ǰ8λ����
				uint8_t row_data2 = char_data[row * 2 + 1];//�ڶ����ֽڣ��洢��8λ����
				for(uint16_t col = 0; col < 16 ; col++)
				{
					if(col < 8)
					{					
						if(row_data1 & (0x01 << col)){ ST7789_DrawPixel(x + col,y + row,color);}//�ӵ�λ��ʼ����
						else if (bg_color != BACKGROUND_COLOR) {
							ST7789_DrawPixel(x + col, y + row, bg_color);
						}					
					}
					else
					{					
						if(row_data2 & (0x01 << (col - 8))){ST7789_DrawPixel(x + col,y + row,color);} //�ӵ�λ��ʼ����
						else if (bg_color != BACKGROUND_COLOR) {
							ST7789_DrawPixel(x + col, y + row, bg_color);
						}
					}
					
				}						
			}
			break;			
		}			
		default:
		{
			const uint8_t *char_data = char_16x8[index];
			for (int row = 0; row < 16; row++) 
			{
				uint8_t row_data = char_data[row];
				for (int col = 0; col < 8; col++) 
				{				
					if (row_data & (0x01 << col)) { // ��λ��ǰ
						ST7789_DrawPixel(x + col, y + row, color); 
					} 
					else if (bg_color != BACKGROUND_COLOR) {
						ST7789_DrawPixel(x + col, y + row, bg_color);
					}
				}
			}
			break;
		}	
	}

}

void ST7789_DrawString(uint16_t x, uint16_t y, const char *str,CharSize size, uint16_t color, uint16_t bg_color)
{
	uint16_t cursor_x = x;
	uint16_t x_offset = 0;
	uint16_t width = 0;
	switch(size)//������������ƫ����
	{
		case CHAR_SIZE_1206:
		{
			width = 6;
			break;
		}
		case CHAR_SIZE_1608:
		{
			width = 8;
			break;
		}
		case CHAR_SIZE_2412:
		{
			width = 12;		
			break;
		}
		case CHAR_SIZE_3216:
		{
			width = 16;		
			break;
		}		
		default:
			width = 8;
	}
	while (*str) 
	{
		ST7789_DrawChar(cursor_x, y, *str,size, color, bg_color);
		cursor_x += (width + x_offset);  // �ַ���� 8 ���أ���� x_offset ����
		str++;
	}
}

// ���ָ������Ϊ��һ��ɫ
void ST7789_SetScreen(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color) {
	if(xStart > xEnd) Swap(&xStart,&xEnd);
	if(yStart > yEnd) Swap(&yStart,&yEnd);
	// ���ô���
	ST7789_SetWindow(xStart, yStart, xEnd, yEnd);

	// ����д���ڴ�����
	WriteCommand(0x2C);

	// ������������
	uint32_t numPixels = (xEnd - xStart + 1) * (yEnd - yStart + 1);

	// �������д����ɫֵ
	for (uint32_t i = 0; i < numPixels; i++) {
		WriteData(color >> 8);      // ��ɫ���ֽ�
		WriteData(color & 0xFF);    // ��ɫ���ֽ�
	}
}
void ST7789_FillScreen(uint16_t color) //ȫ����亯��
{
	uint8_t buff[2] = {color >> 8, color & 0xFF};
	
	ST7789_SetWindow(0, 0, TFT_HEIGHT - 1, TFT_WIDTH - 1);
	
	for(uint32_t i=0; i < TFT_WIDTH * TFT_HEIGHT; i++) 
	{
		WriteMultiData(buff,2);
	}
}
void ST7789_DrawChinese24x24(uint16_t x, uint16_t y, uint16_t color,uint8_t count) {
	for(int i = 0 ; i < count; i++)
	{
		for (int row = 0; row < 24; row++) {        // ����24��
			for (int col = 0; col < 3; col++) {     	// ÿ��3���ֽ�
				uint8_t byte = Chinese_24x24[i][row * 3 + col];  		// ��ȡ��ǰ�ֽ�
				for (int bit = 0; bit < 8; bit++) { 		// �����ֽڵ�ÿ��λ
					if (byte & (0x01 << bit)) {     			// ����λ����λ
						// �����������겢����
						uint16_t x_pixel = x + 24 * i + col * 8 + bit;
						uint16_t y_pixel = y + row;
						ST7789_DrawPixel(x_pixel, y_pixel, color);
					}
				}
			}
		}
	}

}
void ST7789_DrawChinese240x240(uint16_t x, uint16_t y, const uint8_t *font, uint16_t color, uint16_t bg_color) {
	for (int row = 0; row < 240; row++) {          // ����ɨ��
		for (int byte = 0; byte < 30; byte++) {    // ÿ��30�ֽ�
			uint8_t byte_data = font[row * 30 + byte];
			// ������������ֽڸ�λ����λ�����룺1������
			for (int bit = 7; bit >= 0; bit--) {   // bit7->bit0 ��Ӧ��->������
				uint16_t pixel_x = x + byte * 8 + bit; // ����X����
				uint16_t pixel_y = y + row;        // Y����
				// �߽���
				if (pixel_x >= 240 || pixel_y >= 240) continue;
				// ���������ж�����״̬
				if (byte_data & (1 << bit)) {
					ST7789_DrawPixel(pixel_x, pixel_y, color);
				} else if (bg_color != BACKGROUND_COLOR) {
					ST7789_DrawPixel(pixel_x, pixel_y, bg_color);
				}
			}
		}
	}
}
void ST7789_DrawImage_40x40(uint16_t x, uint16_t y, const unsigned char *img) 
{
	for (uint16_t y0 = 0; y0 < 40; y0++) {
		for (uint16_t x0 = 0; x0 < 40; x0++) {
			// ���������������е�λ�� (ÿ������ռ2�ֽ�)
			uint32_t index = (y0 * 40 + x0) * 2;
			
			// ���RGB565��ɫֵ����λ��ǰ��
			uint16_t color = (img[index] << 8) | img[index + 1];
			
			// ���û��㺯��
			ST7789_DrawPixel(x0 + x, y0 + y, color);
		}
	}
}
void ST7789_DrawImage_80x80(uint16_t x, uint16_t y, const unsigned char *img)
{
	for (uint16_t y0 = 0; y0 < 80; y0++) {
		for (uint16_t x0 = 0; x0 < 80; x0++) {
			uint32_t index = (y0 * 80 + x0) * 2;
			
			uint16_t color = (img[index + 1] << 8) | img[index]; // С�������
			
			ST7789_DrawPixel(x + x0, y + y0, color);
		}
	}
}

void ST7789_DrawImage_240x240(uint16_t x, uint16_t y, const unsigned char *img, uint16_t color)
{
	for(uint16_t row = 0 ; row < 240 ;row++)
	{
		for(uint16_t col = 0; col < 30 ;col++)
		{
			uint8_t data = img[row * 30 + col];
			for(uint16_t bit = 0;bit < 8 ;bit++)
			{
				if(data & (0x80 >> bit))//��λ����
				{
					ST7789_DrawPixel(x + col * 8 + bit, y + row,color);
				}
			}
		}
	}
}

void ST7789_RotationDisplayTest(void)
{
	ST7789_SetRotation(DISPLAY_ROTATION_0);
	ST7789_FillScreen(COLOR_RGB(0,0,0));
	HAL_Delay(1000);

	ST7789_SetScreen(19,0,239,119,COLOR_WHITE);
	ST7789_FillScreen(COLOR_WHITE);
	HAL_Delay(1000);

	ST7789_SetRotation(DISPLAY_ROTATION_90);
	ST7789_SetScreen(19,0,239,119,COLOR_RED);
	ST7789_FillScreen(COLOR_RED);
	HAL_Delay(1000);

	ST7789_SetRotation(DISPLAY_ROTATION_180);
	ST7789_SetScreen(19,0,239,119,COLOR_GREEN);
	ST7789_FillScreen(COLOR_GREEN);
	HAL_Delay(1000);		

	ST7789_SetRotation(DISPLAY_ROTATION_270);
	ST7789_SetScreen(19,0,239,119,COLOR_BLUE);
	ST7789_FillScreen(COLOR_BLUE);
	HAL_Delay(1000);
}

void ST7789_DrawStringTest(void)
{
	ST7789_DrawString(0,0,"Hello,World!",CHAR_SIZE_1206,COLOR_BLUE,COLOR_BLACK);
	ST7789_DrawString(0,30,"Hello,World!",CHAR_SIZE_1608,COLOR_GREEN,COLOR_BLACK);
	ST7789_DrawString(0,60,"Hello,World!",CHAR_SIZE_2412,COLOR_RED,COLOR_BLACK);
	ST7789_DrawString(0,90,"Hello,World!",CHAR_SIZE_3216,COLOR_RED,COLOR_BLACK);
}

void ST7789_DrawChineseTest(void)
{
	ST7789_FillScreen(BACKGROUND_COLOR);
	ST7789_DrawChinese240x240(0,0,chinese_240x240_zhong[0],COLOR_GREEN,COLOR_WHITE);
	HAL_Delay(1000);	
	
	ST7789_FillScreen(BACKGROUND_COLOR);
	ST7789_DrawChinese24x24(0,0,COLOR_GREEN,6);
	HAL_Delay(1000);	
	
	ST7789_FillScreen(BACKGROUND_COLOR);
	ST7789_DrawChinese240x240(0,0,chinese_240x240_zhong[1],COLOR_GREEN,COLOR_WHITE);
	HAL_Delay(1000);
	
	ST7789_FillScreen(BACKGROUND_COLOR);
	ST7789_DrawChinese24x24(0,30,COLOR_GREEN,6);
	HAL_Delay(1000);
}
void ST7789_DrawImageTest()
{
	ST7789_DrawImage_240x240(0,0,gImage_bq29,COLOR_GREEN);
}
