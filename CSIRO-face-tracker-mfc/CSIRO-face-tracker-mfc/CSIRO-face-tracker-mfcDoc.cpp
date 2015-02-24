
// CSIRO-face-tracker-mfcDoc.cpp : implementation of the CCSIROfacetrackermfcDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CSIRO-face-tracker-mfc.h"
#endif

#include "CSIRO-face-tracker-mfcDoc.h"

#include <string>
#include "CSIRO-face-tracker\FaceTracker.hpp"
#include "CSIRO-face-tracker\helpers.hpp"
#include "CSIRO-face-tracker\points.hpp"
using namespace FACETRACKER;
#include <opencv2\highgui\highgui.hpp>


#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCSIROfacetrackermfcDoc

IMPLEMENT_DYNCREATE(CCSIROfacetrackermfcDoc, CDocument)

BEGIN_MESSAGE_MAP(CCSIROfacetrackermfcDoc, CDocument)
END_MESSAGE_MAP()


// CCSIROfacetrackermfcDoc construction/destruction

CCSIROfacetrackermfcDoc::CCSIROfacetrackermfcDoc()
{
	// TODO: add one-time construction code here

}

CCSIROfacetrackermfcDoc::~CCSIROfacetrackermfcDoc()
{
}

// CCSIROfacetrackermfcDoc serialization

void CCSIROfacetrackermfcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CCSIROfacetrackermfcDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CCSIROfacetrackermfcDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCSIROfacetrackermfcDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCSIROfacetrackermfcDoc diagnostics

#ifdef _DEBUG
void CCSIROfacetrackermfcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCSIROfacetrackermfcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCSIROfacetrackermfcDoc commands

class user_pressed_escape : public std::exception
{
public:
};

struct Configuration
{
	double wait_time;
	std::string model_pathname;
	std::string params_pathname;
	int tracking_threshold;
	std::string window_title;
	bool verbose;
	bool save_3d_points;

	int circle_radius;
	int circle_thickness;
	int circle_linetype;
	int circle_shift;
};


cv::Mat
compute_pose_image(const Pose &pose, int height, int width)
{
	cv::Mat_<cv::Vec<uint8_t, 3> > rv = cv::Mat_<cv::Vec<uint8_t, 3> >::zeros(height, width);
	cv::Mat_<double> axes = pose_axes(pose);
	cv::Mat_<double> scaling = cv::Mat_<double>::eye(3, 3);

	for (int i = 0; i < axes.cols; i++) {
		axes(0, i) = -0.5*double(width)*(axes(0, i) - 1);
		axes(1, i) = -0.5*double(height)*(axes(1, i) - 1);
	}

	cv::Point centre(width / 2, height / 2);
	// pitch
	cv::line(rv, centre, cv::Point(axes(0, 0), axes(1, 0)), cv::Scalar(255, 0, 0));
	// yaw
	cv::line(rv, centre, cv::Point(axes(0, 1), axes(1, 1)), cv::Scalar(0, 255, 0));
	// roll
	cv::line(rv, centre, cv::Point(axes(0, 2), axes(1, 2)), cv::Scalar(0, 0, 255));

	return rv;
}


void
display_data(const Configuration &cfg,
const cv::Mat &image,
const std::vector<cv::Point_<double> > &points,
const Pose &pose)
{

	cv::Scalar colour;
	if (image.type() == cv::DataType<uint8_t>::type)
		colour = cv::Scalar(255);
	else if (image.type() == cv::DataType<cv::Vec<uint8_t, 3> >::type)
		colour = cv::Scalar(0, 255, 0);
	else
		colour = cv::Scalar(255);

	cv::Mat displayed_image;
	if (image.type() == cv::DataType<cv::Vec<uint8_t, 3> >::type)
		displayed_image = image.clone();
	else if (image.type() == cv::DataType<uint8_t>::type)
		cv::cvtColor(image, displayed_image, CV_GRAY2BGR);
	else
		throw make_runtime_error("Unsupported camera image type for display_data function.");

	for (size_t i = 0; i < points.size(); i++) {
		cv::circle(displayed_image, points[i], cfg.circle_radius, colour, cfg.circle_thickness, cfg.circle_linetype, cfg.circle_shift);
	}

	int pose_image_height = 100;
	int pose_image_width = 100;
	cv::Mat pose_image = compute_pose_image(pose, pose_image_height, pose_image_width);
	for (int i = 0; i < pose_image_height; i++) {
		for (int j = 0; j < pose_image_width; j++) {
			displayed_image.at<cv::Vec<uint8_t, 3> >(displayed_image.rows - pose_image_height + i,
				displayed_image.cols - pose_image_width + j)

				= pose_image.at<cv::Vec<uint8_t, 3> >(i, j);
		}
	}

	cv::imshow(cfg.window_title, displayed_image);

	if (cfg.wait_time == 0)
		std::cout << "Press any key to continue." << std::endl;

	char ch = cv::waitKey(cfg.wait_time * 1000);

	if (ch == 27) // escape
		throw user_pressed_escape();
}



int
run_video_mode(std::string image_argument)
{
	bool lists_mode = false;
	bool video_mode = true;
	bool wait_time_specified = false;
	Configuration cfg;
	cfg.wait_time = 0;
	cfg.model_pathname = DefaultFaceTrackerModelPathname();
	cfg.params_pathname = DefaultFaceTrackerParamsPathname();
	cfg.tracking_threshold = 5;
	cfg.window_title = "CSIRO Face Fit";
	cfg.verbose = false;
	cfg.circle_radius = 2;
	cfg.circle_thickness = 1;
	cfg.circle_linetype = 8;
	cfg.circle_shift = 0;
	cfg.save_3d_points = false;
	if (!wait_time_specified)
		cfg.wait_time = 1.0 / 30;
	
	FaceTracker *tracker = LoadFaceTracker(cfg.model_pathname.c_str());
	FaceTrackerParams *tracker_params = LoadFaceTrackerParams(cfg.params_pathname.c_str());

	assert(tracker);
	assert(tracker_params);

	cv::VideoCapture input(image_argument);
	if (!input.isOpened())
		throw make_runtime_error("Unable to open video file '%s'", image_argument);

	cv::Mat image;

	std::vector<char> pathname_buffer;
	pathname_buffer.resize(1000);

	input >> image;
	double scale = 0.5;
	cv::Size sz = image.size();
	sz.width *= scale;
	sz.height *= scale;
	cv::resize(image, image, sz);
	int frame_number = 1;

	while ((image.rows > 0) && (image.cols > 0)) {
		if (cfg.verbose) {
			printf(" Frame number %d\r", frame_number);
			fflush(stdout);
		}

		cv::Mat_<uint8_t> gray_image;
		if (image.type() == cv::DataType<cv::Vec<uint8_t, 3> >::type)
			cv::cvtColor(image, gray_image, CV_BGR2GRAY);
		else if (image.type() == cv::DataType<uint8_t>::type)
			gray_image = image;
		else
			throw make_runtime_error("Do not know how to convert video frame to a grayscale image.");

		int result = tracker->Track(gray_image, tracker_params);

		std::vector<cv::Point_<double> > shape;
		std::vector<cv::Point3_<double> > shape3D;
		Pose pose;

		if (result >= cfg.tracking_threshold) {
			shape = tracker->getShape();
			shape3D = tracker->get3DShape();
			pose = tracker->getPose();
		}
		else {
			tracker->Reset();
		}

		//if (!have_argument_p(landmarks_argument)) {
			display_data(cfg, image, shape, pose);
		//}
		//else if (shape.size() > 0) {
		//	_snprintf(pathname_buffer.data(), pathname_buffer.size(), landmarks_argument->c_str(), frame_number);

		//	if (cfg.save_3d_points)
		//		save_points3(pathname_buffer.data(), shape3D);
		//	else
		//		save_points(pathname_buffer.data(), shape);

		//	if (cfg.verbose)
		//		display_data(cfg, image, shape, pose);
		//}
		//else if (cfg.verbose) {
		//	display_data(cfg, image, shape, pose);
		//}

		input >> image;
			cv::Size sz = image.size();
			sz.width *= scale;
			sz.height *= scale;
			cv::resize(image, image, sz);

		frame_number++;
	}

	delete tracker;
	delete tracker_params;

	return 0;
}


int
run_cam_mode(int cam_id)
{
	bool lists_mode = false;
	bool video_mode = true;
	bool wait_time_specified = false;
	Configuration cfg;
	cfg.wait_time = 0;
	cfg.model_pathname = DefaultFaceTrackerModelPathname();
	cfg.params_pathname = DefaultFaceTrackerParamsPathname();
	cfg.tracking_threshold = 5;
	cfg.window_title = "CSIRO Face Fit";
	cfg.verbose = false;
	cfg.circle_radius = 3;
	cfg.circle_thickness = 1;
	cfg.circle_linetype = 8;
	cfg.circle_shift = 0;
	cfg.save_3d_points = false;
	if (!wait_time_specified)
		cfg.wait_time = 1.0 / 30;

	FaceTracker *tracker = LoadFaceTracker(cfg.model_pathname.c_str());
	FaceTrackerParams *tracker_params = LoadFaceTrackerParams(cfg.params_pathname.c_str());

	assert(tracker);
	assert(tracker_params);

	cv::VideoCapture input(cam_id);
	if (!input.isOpened())
		throw make_runtime_error("Unable to open webcam '%d'", cam_id);

	cv::Mat image;

	std::vector<char> pathname_buffer;
	pathname_buffer.resize(1000);

	int frame_number = 0;

	while (true) {
		input >> image;
		if (image.rows <= 0 || image.cols <= 0){
			continue; //skip frame if it's bad
		}

		cv::flip(image, image, 1);
		frame_number++;

		if (cfg.verbose) {
			printf(" Frame number %d\r", frame_number);
			fflush(stdout);
		}

		cv::Mat_<uint8_t> gray_image;
		if (image.type() == cv::DataType<cv::Vec<uint8_t, 3> >::type)
			cv::cvtColor(image, gray_image, CV_BGR2GRAY);
		else if (image.type() == cv::DataType<uint8_t>::type)
			gray_image = image;
		else
			throw make_runtime_error("Do not know how to convert video frame to a grayscale image.");

		int result = tracker->Track(gray_image, tracker_params);

		std::vector<cv::Point_<double> > shape;
		std::vector<cv::Point3_<double> > shape3D;
		Pose pose;

		if (result >= cfg.tracking_threshold) {
			shape = tracker->getShape();
			shape3D = tracker->get3DShape();
			pose = tracker->getPose();
		}
		else {
			tracker->Reset();
		}

		//if (!have_argument_p(landmarks_argument)) {
		display_data(cfg, image, shape, pose);
		//}
		//else if (shape.size() > 0) {
		//	_snprintf(pathname_buffer.data(), pathname_buffer.size(), landmarks_argument->c_str(), frame_number);

		//	if (cfg.save_3d_points)
		//		save_points3(pathname_buffer.data(), shape3D);
		//	else
		//		save_points(pathname_buffer.data(), shape);

		//	if (cfg.verbose)
		//		display_data(cfg, image, shape, pose);
		//}
		//else if (cfg.verbose) {
		//	display_data(cfg, image, shape, pose);
		//}

	}

	delete tracker;
	delete tracker_params;

	return 0;
}

int
run_image_mode(std::string image_argument)
{
	bool lists_mode = false;
	bool video_mode = true;
	bool wait_time_specified = false;
	Configuration cfg;
	cfg.wait_time = 0;
	cfg.model_pathname = DefaultFaceTrackerModelPathname();
	cfg.params_pathname = DefaultFaceTrackerParamsPathname();
	cfg.tracking_threshold = 5;
	cfg.window_title = "CSIRO Face Fit";
	cfg.verbose = false;
	cfg.circle_radius = 3;
	cfg.circle_thickness = 1;
	cfg.circle_linetype = 8;
	cfg.circle_shift = 0;
	cfg.save_3d_points = false;
	if (!wait_time_specified)
		cfg.wait_time = 1.0 / 30;

	FaceTracker * tracker = LoadFaceTracker(cfg.model_pathname.c_str());
	FaceTrackerParams *tracker_params = LoadFaceTrackerParams(cfg.params_pathname.c_str());

	cv::Mat image;
	cv::Mat_<uint8_t> gray_image = load_grayscale_image(image_argument.c_str(), &image);

	int result = tracker->NewFrame(gray_image, tracker_params);

	std::vector<cv::Point_<double> > shape;
	std::vector<cv::Point3_<double> > shape3;
	Pose pose;

	//if (result >= cfg.tracking_threshold) {
		shape = tracker->getShape();
		shape3 = tracker->get3DShape();
		pose = tracker->getPose();
	//}

	display_data(cfg, image, shape, pose);

	delete tracker;
	delete tracker_params;

	return 0;
}


BOOL CCSIROfacetrackermfcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	//std::string image_argument = "C:\\Codes\\Untitled.png";
	//run_image_mode(image_argument);
	
	std::string image_argument = "C:\\Codes\\Kinect_color.avi";
	run_video_mode(image_argument);

	//int cam_id = 0;
	//run_cam_mode(cam_id);



	return TRUE;
}
