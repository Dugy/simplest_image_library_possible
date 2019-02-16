# Simplest Image Library Possible

A header-only C++ image library to be easily included into small experiments with images. It has no dependencies except some standard C headers and the most common C++ standard headers. It can only handle _.bmp_ images because opening them doesn't require much code.

It allows pixel-by-pixel modification of pictures and opening/closing them in a RAII compliant way, nothing else. It's only for making quick experiments in C++.

## Usage

The following code makes the picture dark gray and makes the picture progressively bluer towards bottom-right.

``` C++

	Image demo("demo.bmp", 100, 100);
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++) {
			for (int k = 0; k < 3; k++)
				demo[i][j][k] = 20;
			demo[i][j].blue += i + j;
		}
  // Saved by the destructor

```

## More info

The code for reading and writing files was taken from Stack Overflow and edited to fit the style better. This library is intended to wrap it in a clean modern C++ interface.

I intend to make a similar C++ wrapper to _libfreeimage_ whose current C++ wrapper is terrible, however, using that one would necessarily require more than just including a file.
