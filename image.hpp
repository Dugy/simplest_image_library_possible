#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <exception>

class Image {
	int height_;
	int width_;
	std::unique_ptr<uint8_t[]> data_;
	std::string name_;
public:
	/*!
	* /brief Default constructor
	*
	* /note Default constructed images contain no data and cannot be used for anything
	*/
	inline Image() : height_(0), width_(0) { }

	/*!
	* /brief Full constructor, the image will be saved by the destructor
	*
	* /param Name of the picture (the extension isn't added automatically)
	* /param The height of the image
	* /param The width of the image
	* /note Use discard() to prevent saving it
	*/
	inline Image(const std::string& name, int height, int width)
			: height_(height), width_(width), data_(std::make_unique<uint8_t[]>(height * width * 3)), name_(name)
	{
		memset(data_.get(), 0, height * width * 3 * sizeof(uint8_t));
	}

	/*!
	* /brief Constructor, the image will not be saved by the destructor
	*
	* /param The height of the image
	* /param The width of the image
	* /note Use setName() to give it a name so that it could be saved properly
	*/
	inline Image(int height, int width) : Image("", height, width) { }

	/*!
	* /brief Constructor from file, the image's dimensions will be set according to the file
	*
	* /param Name of the picture (the extension isn't added automatically)
	* /note Use discard() to prevent the destructor saving with whatever changes that were done
	*/
	inline Image(const std::string& fileName) {
		FILE* file = fopen(fileName.c_str(), "rb");
		if (file == nullptr) {
			throw(std::runtime_error("Unknown image: " + fileName));
		}
		unsigned char info[54];
		fread(info, sizeof(unsigned char), 54, file);

		width_ = *(int*)&info[18];
		height_ = *(int*)&info[22];

		int size = 3 * width_;
		data_ = std::make_unique<uint8_t[]>(size * height_);
		for (int i = height_ - 1; i >= 0; i--)
			fread(operator[] (i).data_, sizeof(unsigned char), size, file);
		fclose(file);
	}

	/*!
	* /brief Destructor, will save the file if it was assigned a name
	*/
	inline ~Image() {
		if (!name_.empty() && data_)
			save();
	}

	/*!
	* /brief Sets the name of the picture, causing it to be saved there when destroyed
	*
	* /param Name of the picture (the extension isn't added automatically)
	* /note Setting this to an empty string has the same effect as calling discard()
	*/
	inline void setName(const std::string& newName) {
		name_ = newName;
	}

	/*!
	* /brief Retrieves the image's name
	*
	* /return Name of the picture
	*/
	inline const std::string& name() {
		return name_;
	}

	/*!
	* /brief Removes the image's name, preventing it from being saved
	*/
	inline void discard() {
		name_.clear();
	}

	/*!
	* /brief Obtain the height of the picture
	*
	* /return The height in pixels
	*/
	inline int height() {
		return height_;
	}

	/*!
	* /brief Obtain the width of the picture
	*
	* /return The width in pixels
	*/
	inline int width() {
		return width_;
	}

	class subArray {
		uint8_t* data_;
		subArray(uint8_t* dat) : data_(dat) {}
		friend class Image;
	public:
		union Point {
			struct {
				uint8_t blue;
				uint8_t green;
				uint8_t red;
			};
			uint8_t colours[3];

			/*!
			* /brief Gain access to a portion of the colour of the pixel
			*
			* /param The colour part of the pixel
			* /return Reference to the colour
			* /note The colours are ordered blue, green, red
			*/
			inline uint8_t& operator[] (int colour) {
				return colours[colour];
			}

			/*!
			* /brief Set the colour of a pixel in one line
			*
			* /param The blue component
			* /param The green component
			* /param The red component
			*/
			void setColour(uint8_t blueValue, uint8_t greenValue, uint8_t redValue) {
				blue = blueValue;
				green = greenValue;
				red = redValue;
			}

			Point() = delete;
		};

		/*!
		* /brief Gain access to a pixel
		*
		* /param Width of the pixel
		* /return An object that gives access to specific colours as members and through the [] operator
		*/
		inline Point& operator[] (int width) {
			return *reinterpret_cast<Point*>(data_ + width * 3);
		}
	};

	/*!
	* /brief Gain access to a pixel
	*
	* /param Height of the pixel
	* /return An object with overloaded [] operator that returns an object representing the colour of pixel at that width that gives access to specific colours as members and through the [] operator
	* /note The pixels are ordered from top to bottom (unlike in BMP), then from left to right, colours are ordered blue, green, red (like in BMP)
	*/
	inline subArray operator[] (int hei) {
		return subArray(data_.get() + hei * width_* 3);
	}

	/*!
	* /brief Saves the picture
	*
	* /note This is automatically returned by the destructor if the image isn't default-constructed and has a name
	*/
	inline void save() {
		struct BMPheader //BMP file header structure
		{
			uint32_t size; // Size of file
			uint16_t reserved1; // Reserved
			uint16_t reserved2; // ...
			uint32_t offBits; // Offset to bitmap data
		};

		struct BMPinfoHeader //BMP file info structure
		{
			uint32_t size; // Size of info header
			int32_t width; // Width of image
			int32_t height; // Height of image
			uint16_t planes; // Number of color planes
			uint16_t bitCount; // Number of ts per pixel
			uint32_t compression; // Type of compression to use
			uint32_t sizeImage; // Size of image data
			int32_t xPixelsPerMetre; // X pixels per metre
			int32_t yPixelsPerMetre; // Y pixels per metre
			uint32_t clrUsed; // Number of colours used
			uint32_t clrImportant; // Number of important colours
		} ;

		BMPheader header;
		BMPinfoHeader infoHeader;

		// Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside
		uint16_t typeMagicNumber = 0x4d42;
		header.reserved1 = 0;
		header.reserved2 = 0;
		header.size = sizeof(typeMagicNumber) + sizeof(BMPheader) + sizeof(BMPinfoHeader) + height_ * width_ * 3;
		header.offBits = 0x36;

		infoHeader.size = sizeof(BMPinfoHeader);
		infoHeader.width = width_;
		infoHeader.height = height_;
		infoHeader.planes = 1;
		infoHeader.bitCount = 24;
		infoHeader.compression = 0;
		infoHeader.sizeImage = 0;
		infoHeader.xPixelsPerMetre = 5000;
		infoHeader.yPixelsPerMetre = 5000;
		infoHeader.clrUsed = 0;
		infoHeader.clrImportant = 0;

		FILE* file = fopen(name_.c_str(), "wb");
		if (!file)
		{
			throw(std::runtime_error("Could not write file: " + name_));
		}

		// Write headers
		fwrite(&typeMagicNumber, 1, sizeof(typeMagicNumber), file);
		fwrite(&header, 1, sizeof(header), file);
		fwrite(&infoHeader, 1, sizeof(infoHeader), file);

		// Write bitmap
		for (int x = height_ - 1; x >= 0; x--)
		{
			for (int y = 0; y < width_; y++)
			{
				for (int colour = 0; colour < 3; colour++)
					fwrite(&operator[] (x)[y][colour], 1, 1, file);
			}
		}
		fclose(file);
	}
};
