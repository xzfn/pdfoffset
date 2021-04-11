#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <future>
#include <atomic>

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include "ui.h"

#include "pdfoffsetcore.h"


std::string utf8_to_mb(std::string s) {
	unsigned int len = fl_utf8to_mb(s.c_str(), s.size(), nullptr, 0);
	std::vector<char> res(len + 1);
	fl_utf8to_mb(s.c_str(), s.size(), res.data(), res.size());
	std::string res_str(res.data());
	return res_str;
}

template<typename R>
bool is_future_ready(std::future<R>& fut) {
	auto status = fut.wait_for(std::chrono::seconds(0));
	return status == std::future_status::ready;
}

bool is_file(std::string filename) {
	std::ifstream infile(filename);
	if (infile.good()) {
		return true;
	}
	return false;
}

const static double check_interval = 0.1;

class PDFOffsetApp {
public:
	PDFOffsetApp()
		:m_ui(new Ui())
	{
		setup();
	}

	void setup() {
		m_ui->m_run->callback(
			call_proxy<PDFOffsetApp, &PDFOffsetApp::on_run>, this);
		m_ui->m_file_chooser->callback(
			call_proxy<PDFOffsetApp, &PDFOffsetApp::on_file_chooser>, this);
		m_ui->m_help->callback(
			call_proxy<PDFOffsetApp, &PDFOffsetApp::on_help>, this);
		m_ui->m_about->callback(
			call_proxy<PDFOffsetApp, &PDFOffsetApp::on_about>, this);

		set_status("Ready.");
	}

	int run() {
		m_ui->m_window->show();
		return Fl::run();
	}

	PDFOffsetApp(const PDFOffsetApp&) = delete;
	PDFOffsetApp& operator=(const PDFOffsetApp&) = delete;

private:
	template<typename T, void(T::*mf)()>
	static void call_proxy(Fl_Widget* w, void* data) {
		PDFOffsetApp* app = (PDFOffsetApp*)data;
		(app->*mf)();
	}


private:
	void on_run() {
		std::cout << "on_run\n";

		// future should not be valid
		if (m_future.valid()) {
			set_status("ERROR internal error. future should not be valid");
			return;
		}

		// input parameters
		std::string filename_utf8 = m_ui->m_input->value();
		bool skip_first = m_ui->m_skip_first->value();
		bool skip_second = m_ui->m_skip_second->value();
		bool skip_last = m_ui->m_skip_last->value();
		double odd_offset_right = m_ui->m_odd_offset_right->value();
		double even_offset_left = m_ui->m_even_offset_left->value();

		// validate input file
		std::string filename(utf8_to_mb(filename_utf8));
		bool is_input_ok = is_file(filename);
		if (is_input_ok) {
			set_status(std::string("Process pdf file: ") + filename_utf8);
		}
		else {
			set_status("ERROR invalid pdf file.");
			return;
		}

		// output file
		std::string out_filename(filename);
		out_filename.insert(filename.size() - 4, ".offset");

		m_ui->m_run->deactivate();

		// run
		m_future = std::async(std::launch::async, [=]() {
			pdf_offset(
				filename, out_filename,
				10, 20,
				0, 0,
				call_progress, this
			);
		});
		Fl::add_timeout(check_interval, call_timeout, this);
	}

	void on_work_done() {
		std::cout << "on_work_done\n";
		m_future.get();
		std::cout << "app ready\n";
		m_ui->m_run->activate();
		set_status("Ready.");
	}

	static void call_progress(int progress, void* data) {
		PDFOffsetApp* app = (PDFOffsetApp*)data;
		app->on_raw_progress(progress);
	}

	static void call_timeout(void* data) {
		PDFOffsetApp* app = (PDFOffsetApp*)data;
		bool should_repeat = app->on_timeout();
		if (should_repeat) {
			Fl::repeat_timeout(check_interval, call_timeout, data);
		}
	}

	bool on_timeout() {
		// return true to repeat
		if (!m_future.valid()) {
			return false;
		}
		if (is_future_ready(m_future)) {
			on_work_done();
			return false;
		}
		return true;
	}

	void set_status(std::string status) {
		std::cout << "STATUS: " << status << "\n";
		m_ui->m_status->value(status.c_str());
	}

	void on_raw_progress(int progress) {
		m_progress.store(progress);
		Fl::awake(awake_progress, this);
	}

	static void awake_progress(void* data) {
		PDFOffsetApp* app = (PDFOffsetApp*)data;
		app->update_ui_progress();
	}

	void update_ui_progress() {
		std::cout << "update_ui_progress\n";
		m_ui->m_progress->value((float)m_progress.load());
		m_ui->m_progress->redraw();
	}
	
	void on_file_chooser() {
		std::cout << "on_file_chooser\n";
		Fl_Native_File_Chooser file_chooser;
		file_chooser.title("Pick a PDF file");
		file_chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
		file_chooser.filter("PDF\t*.pdf");
		switch (file_chooser.show()) {
		case -1:
			std::cout << "ERROR: " << file_chooser.errmsg() << "\n";
			break;
		case 1:
			std::cout << "cancelled\n";
			break;
		default:
			const char* file_utf8 = file_chooser.filename();
			m_ui->m_input->value(file_utf8);
			break;
		}
	}

	void on_help() {
		std::cout << "on_help\n";
	}

	void on_about() {
		std::cout << "on_about\n";
	}

private:
	std::unique_ptr<Ui> m_ui;
	std::future<void> m_future;
	std::atomic_int m_progress;
};

int main() {
	Fl::lock();
	PDFOffsetApp app;
	return app.run();
}
