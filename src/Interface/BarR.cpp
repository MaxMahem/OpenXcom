#include "BarR.h"

namespace OpenXcom
{

void BarR::draw()
{
	Bar::draw();

	double greaterValue = std::max(_value, _value2);

	SDL_Rect reverseBar = SDL_Rect();
	reverseBar.x = greaterValue - _valueR;
	reverseBar.y = 1;
	reverseBar.w = (Uint16)(_scale * _valueR);
	reverseBar.h = getHeight() - 2;

	Uint8 drawColor = _colorR ? _colorR : _color - 8;

	drawRect(&reverseBar, drawColor);
}

void BarR::setReverseColor(Uint8 color)
{
	_colorR = color;
	_redraw = true;
}

void BarR::setReverseValue(double value)
{
	_valueR = value;
	_redraw = true;
}

}
