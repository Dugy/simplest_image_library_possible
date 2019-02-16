#include "image.hpp"

int main() {
	Image test(20, 20);
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 5; j++)
			for (int k = 0; k < 3; k++)
				test[i][j][k] = 127;
	test.setName("test.bmp");

	Image test2("test.bmp");
	for (int i = 10; i < 15; i++)
		for (int j = 5; j < 10; j++)
			for (int k = 0; k < 3; k++)
				test[i][j].red = 127;

	Image test3;
	Image test4("this_should_not_exist.bmp", 20, 20);
	test4.discard();
}
