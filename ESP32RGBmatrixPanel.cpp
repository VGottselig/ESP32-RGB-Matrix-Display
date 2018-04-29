#include "ESP32RGBmatrixPanel.h"
#include <esp32-hal-gpio.h>
//G1	R1 |
//GND	B1 |
//G2	R2 |
//GND	B2  |
//B		A   |
//D		C  |
//LAT	CLK|
//GND	OE |
//OE connected with LAT



ESP32RGBmatrixPanel::ESP32RGBmatrixPanel(uint8 oe, uint8 clk, uint8 lat, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 a, uint8 b, uint8 c, uint8 d) : Adafruit_GFX(COLUMNS, ROWS)
{
	OE = oe;
	CLK = clk;
	LAT = lat;
	R1 = r1;
	G1 = g1;
	BL1 = b1;
	R2 = r2;
	G2 = g2;
	BL2 = b2;
	CH_A = a;
	CH_B = b;
	CH_C = c;
	CH_D = d;

	initGPIO();
}


ESP32RGBmatrixPanel::ESP32RGBmatrixPanel() : Adafruit_GFX(COLUMNS, ROWS)
{
	initGPIO();
}


void IRAM_ATTR ESP32RGBmatrixPanel::initGPIO()
{
	pinMode(R1, OUTPUT);
	pinMode(G1, OUTPUT);
	pinMode(BL1, OUTPUT);
	pinMode(R2, OUTPUT);
	pinMode(G2, OUTPUT);
	pinMode(BL2, OUTPUT);

	pinMode(CH_A, OUTPUT);
	pinMode(CH_B, OUTPUT);
	pinMode(CH_C, OUTPUT);
	pinMode(CH_D, OUTPUT);

	pinMode(LAT, OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(OE, OUTPUT);
}


void ESP32RGBmatrixPanel::drawPixel(int16_t x, int16_t y, uint16_t c)
{
	if (x < 0 || x >= COLUMNS) return;
	if (y < 0 || y >= ROWS) return;
	auto pixel = &pixels[y][x];
	pixel->r = ((c & rmask)) << 4;
	pixel->g = ((c & gmask));
	pixel->b = ((c & bmask) >> 4);
}

void ESP32RGBmatrixPanel::drawPixel(int16_t x, int16_t y, uint8 r, uint8 g, uint8 b)
{
	drawPixel(x, y, AdafruitColor(r, g, b));
}



void ESP32RGBmatrixPanel::black()
{
	for (int y = 0; y < ROWS; ++y)
	{
		for (int x = 0; x < COLUMNS; x++)
		{
			auto pixel = &pixels[y][x];
			pixel->r = 0;
			pixel->g = 0;
			pixel->b = 0;
		}
	}
}

#define loops 10
void IRAM_ATTR ESP32RGBmatrixPanel::update()
{
	if (loopNr == 0) drawRow();			//Display OFF-time (25 µs).
	if (loopNr == loopNrOn) on();				//Turn Display ON
	loopNr = loopNr + 1;
	if (loopNr >= loops)
	{
		loopNr = 0;
	}
}

void ESP32RGBmatrixPanel::setBrightness(byte brightness)
{
	if (brightness < 0) brightness = 0;
	if (brightness >= loops) brightness = loops;
	if (brightness > 0)
	{
		loopNrOn = loops - brightness;
	}
	else
	{
		//never ON
		loopNrOn = 255;
	}

}

int16_t ESP32RGBmatrixPanel::AdafruitColor(uint8 r, uint8 g, uint8 b)
{
	int16_t  c = 0;
	c = r >> 4;
	c |= (g >> 4) << 4;
	c |= (b >> 4) << 8;
	return c;
}


//#define GAMMA_CORRECTION

#ifdef GAMMA_CORRECTION
const uint8_t PROGMEM gamma8[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#define SetColorPin(pin,  state) gamma8[state] > layer ? gpio |= 1 << pin : gpio &= ~(1 << pin);
#else
#define SetColorPin(pin,  state) state > layer ? gpio |= 1 << pin : gpio &= ~(1 << pin);
#endif // GAMMA_CORRECTION

#define SetPinFast(pin,  state) state? gpio |= 1 << pin : gpio &= ~(1 << pin);



void IRAM_ATTR ESP32RGBmatrixPanel::drawRow()
{
	gpio = GPIO.out;
	SetPinFast(OE, HIGH);
	GPIO.out = gpio;
	SetPinFast(CH_A, row & 1 << 0);
	SetPinFast(CH_B, row & 1 << 1);
	SetPinFast(CH_C, row & 1 << 2);
	SetPinFast(CH_D, row & 1 << 3);
	GPIO.out = gpio;

	for (column = 0; column < COLUMNS; column++)
	{
		SetColorPin(R1, pixels[row][column].r);
		SetColorPin(G1, pixels[row][column].g);
		SetColorPin(BL1, pixels[row][column].b);
		SetColorPin(R2, pixels[row + 16][column].r);
		SetColorPin(G2, pixels[row + 16][column].g);
		SetColorPin(BL2, pixels[row + 16][column].b);

		//SetPinFast(CLK, 1);
		GPIO.out = gpio;
		//SetPinFast(CLK, 0);
		//GPIO.out = gpio;
		GPIO.out_w1ts = ((uint32_t)1 << CLK);//SetPinFast is to fast!? wtf!?
		GPIO.out_w1tc = ((uint32_t)1 << CLK);
	}
	SetPinFast(LAT, HIGH);
	GPIO.out = gpio;
	SetPinFast(LAT, LOW);
	GPIO.out = gpio;
	row++;
	if (row >= 16)
	{
		row = 0;
		layer += layerStep;
	}
	if (layer + 1 >= layers) layer = layerStep - 1;

}




void IRAM_ATTR ESP32RGBmatrixPanel::on()
{
	GPIO.out_w1tc = ((uint32_t)1 << OE);
}




void ESP32RGBmatrixPanel::drawBitmap(String* bytes)
{
	fillScreen(0);
	uint8 imgWidth = 0;
	uint8 imgHeigth = 0;

	if (bytes->length() > 22)
	{
		imgWidth = (uint8)((*bytes)[18]);
		imgHeigth = (uint8)((*bytes)[22]);
	}

	if (imgWidth != 64 || imgHeigth != 32)
	{
		//error square
		uint8 x0 = (64 - 16) / 2;
		uint8 y0 = (32 - 16) / 2;
		uint8 x1 = x0 + 16;
		uint8 y1 = y0 + 16;
		fillScreen(0);
		drawRect(x0, y0, 17, 17, rmask);
		drawLine(x0, y0, x1, y1, rmask);
		drawLine(x0, y1, x1, y0, rmask);
		return;
	}
	uint8 dataOffset = 54;
	uint8 x = 0;
	uint8 y = 31;
	for (uint16 i = dataOffset; i < bytes->length() - 3; i += 3)
	{

		uint16 b = (uint16)((*bytes)[i]);
		uint16 g = (uint16)((*bytes)[i + 1]);
		uint16 r = (uint16)((*bytes)[i + 2]);
		drawPixel(x, y, r, g, b);

		x++;
		if (x >= 64)
		{
			x = 0;
			y--;
		}
	}
}
