#include "pdfoffsetcore.h"

#include <string>
#include <vector>
#include <iostream>
#include <cassert>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>


const std::string KeyMediaBox = "/MediaBox";
const std::string KeyCropBox = "/CropBox";
const std::string KeyBleedBox = "/BleedBox";
const std::string KeyTrimBox = "/TrimBox";
const std::string KeyArtBox = "/ArtBox";

std::vector<std::string> AuxBoxes{ KeyCropBox, KeyBleedBox, KeyTrimBox, KeyArtBox };


QPDFObjectHandle offset_pdf_box(QPDFObjectHandle box, double dx, double dy) {

	double ll_x = box.getArrayItem(0).getNumericValue();
	double ll_y = box.getArrayItem(1).getNumericValue();
	double ur_x = box.getArrayItem(2).getNumericValue();
	double ur_y = box.getArrayItem(3).getNumericValue();

	double new_ll_x = ll_x + dx;
	double new_ll_y = ll_y + dy;
	double new_ur_x = ur_x + dx;
	double new_ur_y = ur_y + dy;

	int decimal_places = 2;
	std::vector<QPDFObjectHandle> new_box_array{
		QPDFObjectHandle::newReal(new_ll_x, decimal_places),
		QPDFObjectHandle::newReal(new_ll_y, decimal_places),
		QPDFObjectHandle::newReal(new_ur_x, decimal_places),
		QPDFObjectHandle::newReal(new_ur_y, decimal_places),
	};

	QPDFObjectHandle new_box = QPDFObjectHandle::newArray(new_box_array);
	return new_box;
}

void check_pdf_box(QPDFObjectHandle& box) {
	if (!(box.isArray() && box.getArrayNItems() == 4)) {
		std::cout << "bad box\n";
		assert(false);
	}
}

void offset_pdf_page(QPDFObjectHandle& page, double dx, double dy) {
	auto media_box = page.getKey(KeyMediaBox);
	auto new_media_box = offset_pdf_box(media_box, dx, dy);
	page.replaceKey(KeyMediaBox, new_media_box);

	for (auto& key_box : AuxBoxes) {
		auto aux_box = page.getKey(key_box);
		if (!aux_box.isNull()) {
			auto new_aux_box = offset_pdf_box(aux_box, dx, dy);
			page.replaceKey(key_box, new_aux_box);
		}
	}
}

bool is_even_page(int page_num) {
	return page_num % 2 == 0;
}

class ProgressReporter : public QPDFWriter::ProgressReporter {
public:
	ProgressReporter(PDFOffsetProgressCallback callback, void* user_data)
		:m_callback(callback), m_user_data(user_data)
	{

	}

	virtual ~ProgressReporter() {
		m_callback = nullptr;
		m_user_data = nullptr;
	}

	void reportProgress(int progress) override {
		if (m_callback) {
			(*m_callback)(50 + progress / 2, m_user_data);
		}
	}

private:
	PDFOffsetProgressCallback m_callback;
	void* m_user_data;
};

void pdf_offset(
	std::string filename, std::string out_filename,
	double odd_distance_x, double even_distance_x,
	double odd_distance_y, double even_distance_y,
	bool skip_first,
	bool skip_second,
	bool skip_last,
	PDFOffsetProgressCallback callback, void* user_data
)
{
	if (callback) {
		(*callback)(5, user_data);
	}
	QPDF qpdf;
	std::cout << "open file\n";
	qpdf.processFile(filename.c_str());
	if (callback) {
		(*callback)(10, user_data);
	}
	std::cout << "process file\n";
	std::vector<QPDFPageObjectHelper> pages = QPDFPageDocumentHelper(qpdf).getAllPages();
	if (callback) {
		(*callback)(30, user_data);
	}

	std::size_t page_count = pages.size();
	int notify_count = 10;
	int notify_gap = page_count / notify_count;
	if (notify_gap == 0) {
		notify_gap = 1;
	}

	for (std::size_t i = 0; i < page_count; ++i) {
		auto& page = pages[i].getObjectHandle();
		int page_num = i + 1;
		if (page_num == 1 && skip_first) {
			continue;
		}
		if (page_num == 2 && skip_second) {
			continue;
		}
		if (page_num == page_count && skip_last) {
			continue;
		}

		if (is_even_page(page_num)) {
			offset_pdf_page(page, even_distance_x, even_distance_y);
		}
		else {
			offset_pdf_page(page, odd_distance_x, odd_distance_y);
		}
		//std::cout << i << "\n";
		if (i % notify_gap == 0) {
			if (callback) {
				(*callback)(30 + i / notify_gap, user_data);
			}
		}
	}
	if (callback) {
		(*callback)(40, user_data);
	}
	std::cout << "write file\n";

	QPDFWriter writer(qpdf);
	writer.setOutputFilename(out_filename.c_str());
	PointerHolder<QPDFWriter::ProgressReporter> progress_reporter(new ProgressReporter(callback, user_data));
	writer.registerProgressReporter(progress_reporter);
	writer.setCompressStreams(false);
	writer.setPreserveUnreferencedObjects(true);
	writer.setDecodeLevel(qpdf_dl_none);
	if (callback) {
		(*callback)(50, user_data);
	}
	writer.write();
}


#if 0
// test ui progress bar
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
#endif
