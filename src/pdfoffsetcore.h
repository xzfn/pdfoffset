#pragma once

#include <string>

typedef void (*PDFOffsetProgressCallback)(int progress, void* user_data);

void pdf_offset(
	std::string filename, std::string out_filename,
	double odd_distance_x, double even_distance_x, 
	double odd_distance_y, double even_distance_y,
	bool skip_first,
	bool skip_second,
	bool skip_last,
    PDFOffsetProgressCallback callback, void* user_data
); 
