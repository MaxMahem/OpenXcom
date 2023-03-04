#pragma once
#include "Bar.h"

namespace OpenXcom
{


class BarR : public Bar
{
  private:
	Uint8 _colorR;
	double _valueR;

  public:
	BarR(int width, int height, int x = 0, int y = 0) : Bar(width, height, x, y), _colorR(), _valueR() {};

	/// Gets the bar's reverse color.
	Uint8 getReverseColor() const { return _colorR; }
	/// Sets the bar's reverse color.
	void setReverseColor(Uint8 color);

	/// Gets the bar's current value.
	double getReverseValue() const { return _valueR; }
	/// Sets the bar's current value.
	void setReverseValue(double value);
	
	/// Draws the bar.
	void draw() override;
};

}
