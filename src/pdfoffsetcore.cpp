#include "pdfoffsetcore.h"

#include <string>
#include <thread>


void pdf_offset(
	std::string filename, std::string out_filename,
	double odd_distance_x, double even_distance_x, 
	double odd_distance_y, double even_distance_y,
    PDFOffsetProgressCallback callback, void* user_data
)
{
    (*callback)(0, user_data);
    std::this_thread::sleep_for(std::chrono::duration<double>(0.4));
    (*callback)(10, user_data);
    std::this_thread::sleep_for(std::chrono::duration<double>(0.3));
    (*callback)(30, user_data);
    std::this_thread::sleep_for(std::chrono::duration<double>(0.5));
    (*callback)(90, user_data);
    std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
    (*callback)(100, user_data);
}
