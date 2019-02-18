# Simplest Image Library Possible

So you like C++ and you need to change some picture in a relatively trivial way but no image editor has any apparent option for that apart from some scripting that takes hell of a time to learn?

This library is here to help you. It gives low level access to pixel by pixel data without any clutter and it requires no installation, just downloading and including.

Suppose you want to swap blue and green colours in an image. This is all you need to do:

``` C++

Image img("recolouring.bmp");
for (int i = 0; i < img.height(); i++)
	for (int j = 0; j < img.width(); j++) {
		uint8_t buffer = img[i][j].blue;
		img[i][j].blue = img[i][j].green;
		img[i][j].green = buffer;
	}

```

And that's all. Obvious, low level, fast. One RAII-compliant class with some intuitive access functions.

For simplicity of usage, It has no dependencies except some standard C headers and some of the more common C++ standard headers. It can only handle _.bmp_ images because opening them doesn't require much code.

## Example

The following code makes the picture dark grey and makes the picture progressively bluer towards bottom-right.

``` C++

	Image demo("demo.bmp", 100, 100);
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++) {
			demo[i][j].setColour(20, 20, 20);
			demo[i][j].blue += i + j;
		}
  // Saved by the destructor

```

## Longer example

For drawing objects on it, you can use a class that inherits from it that has a `draw()` method that accepts some parent/interface class as an argument:

``` C++

#include "image.hpp"

class Drawable {
public:
	/*!
	* /brief Interface allowing writing image->draw(shape); instead of shape->drawTo(image);
	*
	* /param Reference to the picture
	*/
	virtual void draw(Image& target) = 0;
};

class Canvas : public Image {
	using Image::Image;
	/*!
	* /brief Tool allowing convenient implementation of drawable objects
	*
	* /param Any class inheriting from Drawable
	*/
	void draw(Drawable& drawn) {
		drawn.draw(*this);
	}
};


```

Then you can create a drawable like this (choosing a triangle because all kinds of polygons can be composed of them):

``` C++

class Triangle : public Drawable {
	using Point = std::pair<int, int>;
	Point pt_[3];
	uint8_t colour_[3];
public:
	Point& operator[] (int index) {
		return pt_[index];
	}
	Triangle() = default;
	Triangle(Point pt1, Point pt2, Point pt3) {
		pt_[0] = pt1;
		pt_[1] = pt2;
		pt_[2] = pt3;
	}
	void setColour(uint8_t blue, uint8_t green, uint8_t red) {
		colour_[0] = blue;
		colour_[1] = green;
		colour_[2] = red;
	}

	virtual void draw(Image& target) {
		Point points[3];
		for (int i = 0; i < 3; i++)
			points[i] = pt_[i];
		for (int repeat = 0; repeat < 2; repeat++)
			for (int i = 0; i < 2; i++)
				if (points[i].first > points[i + 1].first)
					std::swap(points[i], points[i + 1]);
		
		// The triangle has two parts, one is interrupted by the point in middle height, the other directly connects the upper and lower points
		std::vector<int> uninterrupted(points[2].first - points[0].first + 1);
		std::vector<int> interrupted(points[2].first - points[0].first + 1);
		float direction = float(points[2].second - points[0].second) / float(points[2].first - points[0].first);
		for (int i = 0; i <= points[2].first - points[0].first; i++) {
			uninterrupted[i] = direction * i + points[0].second;
		}
		direction = float(points[1].second - points[0].second) / float(points[1].first - points[0].first);
		float direction2 = float(points[2].second - points[1].second) / float(points[2].first - points[1].first);
		interrupted.front() = points[0].second;
		interrupted.back() = points[2].second;
		for (int i = 1; i < points[2].first - points[0].first; i++) {
			interrupted[i] = (i < (points[1].first - points[0].first)) ? (direction * i + points[0].second) : (direction2 * (i - points[1].first) + points[2].second);
		}
		if (uninterrupted[uninterrupted.size() / 2] > interrupted[interrupted.size() / 2])
			uninterrupted.swap(interrupted); // Now we know the interrupted values are on the left

		for (int i = 0; i < uninterrupted.size(); i++) {
			for (int j = uninterrupted[i]; j <= interrupted[i]; j++) {
				target[i + points[0].first][j].setColour(colour_[0], colour_[1], colour_[2]);
			}
		}
	}
};


```

Which allows drawing with fairly short code:

``` C++

Canvas img("canvas.bmp", 100, 100);

Triangle tri({10, 10}, {90, 10}, {90, 90});
tri.setColour(0, 200, 0);
tri.draw(img);


```

## More info

The code for reading and writing files was taken from Stack Overflow and edited to fit the style better. This library is intended to wrap it in a clean modern C++ interface.

I intend to make a similar C++ wrapper to _libfreeimage_ whose current C++ wrapper is terrible, however, using that one would necessarily require more than just including a file.
