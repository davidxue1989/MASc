
/*
Copyright (C) 2014 Steven Hickson

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/
// TestVideoSegmentation.cpp : Defines the entry point for the console application.
//

#include "Microsoft_grabber2.h"

using namespace std;
using namespace cv;


#include <pcl/filters/project_inliers.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/conversions.h>


#include <pcl/common/transforms.h>

//dxinclude
#include <vector>


// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

DWORD ProcessThread(LPVOID pParam) {
	pcl::Microsoft2Grabber *p = (pcl::Microsoft2Grabber*) pParam;
	p->ProcessThreadInternal();
	return 0;
}

template <typename T> inline T Clamp(T a, T minn, T maxx)
{
	return (a < minn) ? minn : ((a > maxx) ? maxx : a);
}

namespace pcl {
	Microsoft2Grabber::Microsoft2Grabber(const int instance, bool usingEyediap, bool trackingOnly) :
		m_headpos_Tracked(TrackingState_NotTracked)
		, viewer(new pcl::visualization::PCLVisualizer("visualizeCloud"))
		, usingEyediap(usingEyediap)
		, frameProcessed(true)
		, frameGrabbed(false)
		, icp_camera_pose(new Eigen::Affine3f(Eigen::Affine3f::Identity()))
		, arrayH(-1)
		, arrayW(-1)
		, mesh_cld_ptr_(new pcl::PointCloud<pcl::PointXYZ>())
		, icp_success(false)
		, icp_success_last_frame(false)
		, trackingOnly(trackingOnly)
	{
		//get eyediap dir
		ifstream infile;
		infile.open("eyediap_dir.txt");
		assert(infile.is_open());
		getline(infile, dir);
		//add coordinate axis to visualizer
		viewer->addCoordinateSystem(1.0, "visualizeCloud");
		//getting calibration params
		if (usingEyediap) {
			m_calib.getParams(dir);
		}

		HRESULT hr;
		int num = 0;
		m_person = m_depthStarted = m_videoStarted = m_audioStarted = m_infraredStarted = false;
		hStopEvent = NULL;
		hKinectThread = NULL;
		m_largeCloud = false;
		//m_largeCloud = true;

		if (!usingEyediap) {

			hr = GetDefaultKinectSensor(&m_pKinectSensor);
			if (FAILED(hr)) {
				throw exception("Error could not get default kinect sensor");
			}

			if (m_pKinectSensor) {
				hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
				hr = m_pKinectSensor->Open();
				if (SUCCEEDED(hr)) {
					hr = m_pKinectSensor->OpenMultiSourceFrameReader(
						//dxchange
						//FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex,
						//&m_pMultiSourceFrameReader);
						FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_BodyIndex | FrameSourceTypes::FrameSourceTypes_Body,
						&m_pMultiSourceFrameReader);
					if (SUCCEEDED(hr))
					{
						m_videoStarted = m_depthStarted = true;
					}
					else
						throw exception("Failed to Open Kinect Multisource Stream");
				}
			}

			if (!m_pKinectSensor || FAILED(hr)) {
				throw exception("No ready Kinect found");
			}
			m_colorSize = Size(cColorWidth, cColorHeight);
			m_depthSize = Size(cDepthWidth, cDepthHeight);
			m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
			m_pColorCoordinates = new ColorSpacePoint[cDepthHeight * cDepthWidth];
			m_pCameraSpacePoints = new CameraSpacePoint[cColorHeight * cColorWidth];

			// create callback signals
			image_signal_ = createSignal<sig_cb_microsoft_image>();
			depth_image_signal_ = createSignal<sig_cb_microsoft_depth_image>();
			image_depth_image_signal_ = createSignal<sig_cb_microsoft_image_depth_image>();
			point_cloud_rgba_signal_ = createSignal<sig_cb_microsoft_point_cloud_rgba>();
			all_data_signal_ = createSignal<sig_cb_microsoft_all_data>();
			/*ir_image_signal_       = createSignal<sig_cb_microsoft_ir_image> ();
			point_cloud_signal_    = createSignal<sig_cb_microsoft_point_cloud> ();
			point_cloud_i_signal_  = createSignal<sig_cb_microsoft_point_cloud_i> ();
			point_cloud_rgb_signal_   = createSignal<sig_cb_microsoft_point_cloud_rgb> ();
			*/
			rgb_sync_.addCallback(boost::bind(&Microsoft2Grabber::imageDepthImageCallback, this, _1, _2));
		}
		else {

			m_videoStarted = m_depthStarted = true;

			m_colorSize = Size(cColorWidth1, cColorHeight1);
			m_depthSize = Size(cDepthWidth1, cDepthHeight1);


			//m_pColorRGBX = new RGBQUAD[cColorWidth1 * cColorHeight1];
			//m_pColorCoordinates = new ColorSpacePoint[cDepthHeight1 * cDepthWidth1];
			//m_pCameraSpacePoints = new CameraSpacePoint[cColorHeight1 * cColorWidth1];

			// create callback signals
			image_signal_ = createSignal<sig_cb_microsoft_image>();
			depth_image_signal_ = createSignal<sig_cb_microsoft_depth_image>();
			image_depth_image_signal_ = createSignal<sig_cb_microsoft_image_depth_image>();
			point_cloud_rgba_signal_ = createSignal<sig_cb_microsoft_point_cloud_rgba>();
			all_data_signal_ = createSignal<sig_cb_microsoft_all_data>();
		}
	}

	void Microsoft2Grabber::start() {
		block_signals();
		//GetCameraSettings();
		/*hDepthMutex = CreateMutex(NULL,false,NULL);
		if(hDepthMutex == NULL)
		throw exception("Could not create depth mutex");
		hColorMutex = CreateMutex(NULL,false,NULL);
		if(hColorMutex == NULL)
		throw exception("Could not create color mutex");*/
		//hFrameEvent = (WAITABLE_HANDLE)CreateEvent(NULL,FALSE,FALSE,NULL);
		if (!usingEyediap) {
			HRESULT hr = m_pMultiSourceFrameReader->SubscribeMultiSourceFrameArrived(&hFrameEvent);
			if (FAILED(hr)) {
				throw exception("Couldn't subscribe frame");
			}
		}
		hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		hKinectThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &ProcessThread, this, 0, NULL);
		//boost::this_thread::sleep (boost::posix_time::seconds (1));
		unblock_signals();
	}

	void Microsoft2Grabber::stop() {
		//stop the ProcessThread
		if (hStopEvent != NULL) {
			//signal the process to stop
			SetEvent(hStopEvent);
			if (hKinectThread != NULL) {
				WaitForSingleObject(hKinectThread, INFINITE);
				CloseHandle(hKinectThread);
				hKinectThread = NULL;
			}
			CloseHandle(hStopEvent);
			hStopEvent = NULL;
			if (!usingEyediap) {
				m_pMultiSourceFrameReader->UnsubscribeMultiSourceFrameArrived(hFrameEvent);
				CloseHandle((HANDLE) hFrameEvent);
				hFrameEvent = NULL;
			}
			/*CloseHandle(hDepthMutex);
			hDepthMutex = NULL;
			CloseHandle(hColorMutex);
			hColorMutex = NULL;*/
		}
		if (m_pColorRGBX) {
			delete [] m_pColorRGBX;
			m_pColorRGBX = NULL;
		}
		if (m_pColorCoordinates) {
			delete [] m_pColorCoordinates;
			m_pColorCoordinates = NULL;
		}
		if (m_pCameraSpacePoints) {
			delete [] m_pCameraSpacePoints;
			m_pCameraSpacePoints = NULL;
		}

		if (arrayH > 0 || arrayW > 0) {
			for (int i = 0; i < arrayH; i++) {
				delete [] d_best[i];
				delete [] s_best[i];
				delete [] visited[i];
			}
			delete [] d_best;
			delete [] s_best;
			delete [] visited;
		}
	}

	bool Microsoft2Grabber::isRunning() const {
		return (!(hKinectThread == NULL));
	}

	Microsoft2Grabber::~Microsoft2Grabber() {
		Release();
	}

	bool Microsoft2Grabber::GetCameraSettings() {
		/*CameraSettings = NULL;
		if(S_OK == kinectInstance->NuiGetColorCameraSettings(&CameraSettings))
		CameraSettingsSupported = true;
		else
		CameraSettingsSupported = false;*/
		return CameraSettingsSupported;
	}

	void Microsoft2Grabber::Calib::getParams(string dir) {
		ifstream infile;
		string STRING;
		int indx;
		//====================== depth =======================
		infile.open(dir + "depth_calibration.txt");
		assert(infile.is_open());

		//[resolution]
		getline(infile, STRING);
		assert(STRING.compare("[resolution]") == 0);
		getline(infile, STRING);
		indx = STRING.find(';');
		resolution_d.width = atoi(STRING.substr(0, indx).c_str());
		resolution_d.height = atoi(STRING.substr(indx + 1, STRING.length()).c_str());

		//[intrinsics]
		getline(infile, STRING);
		assert(STRING.compare("[intrinsics]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = intrinsics_d.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			intrinsics_d(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[R]
		getline(infile, STRING);
		assert(STRING.compare("[R]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = R_d.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			R_d(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[T]
		getline(infile, STRING);
		assert(STRING.compare("[T]") == 0);
		getline(infile, STRING);
		T_d(0, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_d(1, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_d(2, 0) = atof(STRING.c_str());

		//[k_coefficients]
		getline(infile, STRING);
		assert(STRING.compare("[k_coefficients]") == 0);
		for (int i = 0; i < 2; i++) {
			int cols = k_coefficients.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			k_coefficients(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[alpha]
		getline(infile, STRING);
		assert(STRING.compare("[alpha]") == 0);
		for (int i = 0; i < 2; i++) {
			int cols = alpha.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			alpha(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[beta]
		getline(infile, STRING);
		assert(STRING.compare("[beta]") == 0);
		beta.create(resolution_d, DataType<float>::type);
		for (int i = 0; i < resolution_d.area(); i++) {
			int cols = beta.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			beta.at<float>(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}
		//cout << "beta = " << endl << " " << beta(Rect(0, 5, resolution.width, 1)) << endl << endl;

		getline(infile, STRING);
		assert(STRING.compare("") == 0);
		assert(infile.eof());
		infile.close();

		//====================== RGB =======================
		infile.open(dir + "rgb_vga_calibration.txt");
		assert(infile.is_open());

		//[resolution]
		getline(infile, STRING);
		assert(STRING.compare("[resolution]") == 0);
		getline(infile, STRING);
		indx = STRING.find(';');
		resolution_rgb.width = atoi(STRING.substr(0, indx).c_str());
		resolution_rgb.height = atoi(STRING.substr(indx + 1, STRING.length()).c_str());

		//[intrinsics]
		getline(infile, STRING);
		assert(STRING.compare("[intrinsics]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = intrinsics_rgb.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			intrinsics_rgb(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[R]
		getline(infile, STRING);
		assert(STRING.compare("[R]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = R_rgb.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			R_rgb(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[T]
		getline(infile, STRING);
		assert(STRING.compare("[T]") == 0);
		getline(infile, STRING);
		T_rgb(0, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_rgb(1, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_rgb(2, 0) = atof(STRING.c_str());

		getline(infile, STRING);
		assert(STRING.compare("") == 0);
		assert(infile.eof());
		infile.close();


		//====================== RGB HD =======================
		infile.open(dir + "rgb_hd_calibration.txt");
		assert(infile.is_open());

		//[resolution]
		getline(infile, STRING);
		assert(STRING.compare("[resolution]") == 0);
		getline(infile, STRING);
		indx = STRING.find(';');
		resolution_rgbHD.width = atoi(STRING.substr(0, indx).c_str());
		resolution_rgbHD.height = atoi(STRING.substr(indx + 1, STRING.length()).c_str());

		//[intrinsics]
		getline(infile, STRING);
		assert(STRING.compare("[intrinsics]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = intrinsics_rgbHD.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			intrinsics_rgbHD(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[R]
		getline(infile, STRING);
		assert(STRING.compare("[R]") == 0);
		for (int i = 0; i < 9; i++) {
			int cols = R_rgbHD.cols;
			if (i % cols == 0) {
				getline(infile, STRING);
			}
			indx = STRING.find(';');
			R_rgbHD(i / cols, i%cols) = atof(STRING.substr(0, indx).c_str());
			STRING = STRING.substr(indx + 1, STRING.length());
		}

		//[T]
		getline(infile, STRING);
		assert(STRING.compare("[T]") == 0);
		getline(infile, STRING);
		T_rgbHD(0, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_rgbHD(1, 0) = atof(STRING.c_str());
		getline(infile, STRING);
		T_rgbHD(2, 0) = atof(STRING.c_str());

		getline(infile, STRING);
		assert(STRING.compare("") == 0);
		assert(infile.eof());
		infile.close();
	}

	void Microsoft2Grabber::ProcessThreadInternal() {
		if (!usingEyediap) {
			HANDLE handles [] = { reinterpret_cast<HANDLE>(hFrameEvent) };
			int idx;
			bool quit = false;
			frameNum = 0;
			while (!quit) {
				// Wait for any of the events to be signalled
				idx = WaitForMultipleObjects(1, handles, FALSE, 100);
				switch (idx) {
				case WAIT_TIMEOUT:
					continue;
				case WAIT_OBJECT_0:
					IMultiSourceFrameArrivedEventArgs *pFrameArgs = nullptr;
					HRESULT hr = m_pMultiSourceFrameReader->GetMultiSourceFrameArrivedEventData(hFrameEvent, &pFrameArgs);
					//frame arrived
					frameNum++;
					FrameArrived(pFrameArgs);
					pFrameArgs->Release();
					break;
					/*case WAIT_OBJECT_0 + 1:
					quit = true;
					continue;*/
				}
				//if(WaitForSingleObject(hStopEvent,1) == WAIT_OBJECT_0)
				//	quit = true;
				//else {
				//	//Get the newest frame info
				//	GetNextFrame();
				//}
			}
		}
		else {//using eyediap dataset
			//headpose
			ifstream infile;
			string hposestr;
			infile.open(dir + "head_pose.txt");
			if (!infile.is_open()) {
				cout << "head_pose.txt open failed" << endl;
				return;
			}
			getline(infile, hposestr);
			assert(hposestr.compare("Head pose: (Rotation matrix (9 values, row by row);Translation(3 values, x,y,z))") == 0);

			//set up video streams
			VideoCapture capRGB(dir + "rgb_vga.mov");
			if (!capRGB.isOpened())  {
				cout << "capRGB failed" << endl;
				return;
			}
			VideoCapture capDepth;
			capDepth.open(dir + "depth.avi");
			if (!capDepth.isOpened()) {
				cout << "capDepth failed" << endl;
				return;
			}
			VideoCapture capRGBHD;
			capRGBHD.open(dir + "rgb_hd.mov");
			if (!capRGBHD.isOpened()) {
				cout << "capRGBHD failed" << endl;
				return;
			}

			//namedWindow("rgb", 1);
			//namedWindow("depth", 1);
			//namedWindow("rgbHD", 1);


			while (true)
			{
				//wait till processing is done before grabbing a new frame
				if (!frameProcessed)
					continue;

				// get frames
				Mat frameRGB, frameDepth, frameRGBHD;
				capRGB >> frameRGB;
				capDepth >> frameDepth;
				capRGBHD >> frameRGBHD;

				if (frameRGB.empty() || frameDepth.empty() || frameRGBHD.empty())
					break;

				//get headpose
				Matx33f headp_rot;
				Matx31f headp_tran;
				getline(infile, hposestr);
				if (hposestr.empty())
					break;
				int indx;
				indx = hposestr.find(';');
				frameNum = atof(hposestr.substr(0, indx).c_str());
				//if (frameNum < 2250)
				//	continue;

				hposestr = hposestr.substr(indx + 1, hposestr.length());
				for (int i = 0; i < 9; i++) {
					int cols = headp_rot.cols;
					indx = hposestr.find(';');
					headp_rot(i / cols, i%cols) = atof(hposestr.substr(0, indx).c_str());
					hposestr = hposestr.substr(indx + 1, hposestr.length());
				}
				for (int i = 0; i < 3; i++) {
					int cols = headp_tran.cols;
					indx = hposestr.find(';');
					headp_tran(i / cols, i%cols) = atof(hposestr.substr(0, indx).c_str());
					hposestr = hposestr.substr(indx + 1, hposestr.length());
				}
				m_headpos_Tracked = TrackingState_Tracked;
				m_headpos = Eigen::Vector3f(headp_tran(0, 0), headp_tran(1, 0), headp_tran(2, 0));

				double alpha1 = m_calib.alpha(0, 0);
				double alpha2 = m_calib.alpha(0, 1);
				double dc1 = m_calib.k_coefficients(0, 0);
				double dc2 = m_calib.k_coefficients(0, 1);
				Mat frameDepthUSHORT(frameDepth.size(), DataType<ushort>::type);
				for (int i = 0; i < frameDepth.rows; i++) {
					for (int j = 0; j < frameDepth.cols; j++) {
						Vec3b px = frameDepth.at<Vec3b>(i, j);
						uchar B = px[0];
						uchar G = px[1];
						if (px[2] != 0) { //this is the white text for frame number on top left corner
							B = 0;
							G = 0;
							frameDepthUSHORT.at<ushort>(i, j) = 0;
						}
						else if (B == 255 && G == 224) { //this is the NaN pixels
							B = 0;
							G = 0;
							frameDepthUSHORT.at<ushort>(i, j) = 0;
						}
						else {
							//ushort depth = ((ushort) G) << 3 + (ushort) B;
							ushort GG = ((ushort) G) << 3;
							ushort depth = ((ushort) B) + GG; // for some reason, we have to do the shift operation in a separate expression to get it working
#if 0
							frameDepthUSHORT.at<ushort>(i, j) = depth;
#else //undistort
							double d = depth;
							//1. undistort d to get z, i.e. disparity2depth, undistort_disparity
							double beta = m_calib.beta.at<float>(i, j);
							double disp_k = d + beta * exp(alpha1 - alpha2 * d);//undistortion factor
							double z = 1 / (dc2 * disp_k + dc1);
							frameDepthUSHORT.at<ushort>(i, j) = (ushort) (z * 1000);
#endif
						}
					}
				}

				boost::shared_ptr<Mat> img(new Mat());
				*img = frameRGB.clone();
				if (image_signal_->num_slots() > 0) {
					image_signal_->operator()(img);
				}

				MatDepth depth_img = *((MatDepth*) &(frameDepthUSHORT.clone()));
				if (depth_image_signal_->num_slots() > 0) {
					depth_image_signal_->operator()(depth_img);
				}

				boost::shared_ptr<Mat> imgHD(new Mat());
				*imgHD = frameRGBHD.clone();

				//cv::imshow("rgb", frameRGB);
				//cv::imshow("depth", frameDepth);
				//cv::imshow("rgbHD", frameRGBHD);
				//waitKey(1);

				// form color point cloud slots
				m_cloudMutex.lock();
				m_cloud = convertToXYZRGBAPointCloud2(img, depth_img, imgHD, m_calib);
				if (point_cloud_rgba_signal_->num_slots() > 0) {
					point_cloud_rgba_signal_->operator()(m_cloud);
				}
				m_cloudMutex.unlock();
				if (image_depth_image_signal_->num_slots() > 0) {
					float constant = 1.0f;
					image_depth_image_signal_->operator()(img, depth_img, constant);
				}

				frameProcessed = false;
				frameGrabbed = true;
			}
			infile.close();
		}
	}


	void Microsoft2Grabber::Release() {
		try {
			//clean up stuff here
			stop();
			if (m_pKinectSensor) {
				//Shutdown NUI and Close handles
				if (m_pMultiSourceFrameReader)
					SafeRelease(m_pMultiSourceFrameReader);
				if (m_pCoordinateMapper)
					SafeRelease(m_pCoordinateMapper);
				// close the Kinect Sensor
				if (m_pKinectSensor)
					m_pKinectSensor->Close();

				SafeRelease(m_pKinectSensor);
			}
		}
		catch (...) {
			//destructor never throws
		}
	}

	string Microsoft2Grabber::getName() const {
		return std::string("Microsoft2Grabber");
	}

	float Microsoft2Grabber::getFramesPerSecond() const {
		return 30.0f;
	}

	void Microsoft2Grabber::BodyIndexFrameArrived(IBodyIndexFrameReference* pBodyIndexFrameReference) {
		IBodyIndexFrame* pBodyIndexFrame = NULL;
		HRESULT hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
		if (FAILED(hr))
			return;
		//cout << "got a body index frame" << endl;
		IFrameDescription* pBodyIndexFrameDescription = NULL;
		int nBodyIndexWidth = 0;
		int nBodyIndexHeight = 0;
		UINT nBodyIndexBufferSize = 0;
		BYTE *pBodyIndexBuffer = NULL;

		// get body index frame data
		if (SUCCEEDED(hr)) {
			hr = pBodyIndexFrame->get_FrameDescription(&pBodyIndexFrameDescription);
		}
		if (SUCCEEDED(hr)) {
			hr = pBodyIndexFrameDescription->get_Width(&nBodyIndexWidth);
		}
		if (SUCCEEDED(hr)) {
			hr = pBodyIndexFrameDescription->get_Height(&nBodyIndexHeight);
		}
		if (SUCCEEDED(hr)) {
			hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexBufferSize, &pBodyIndexBuffer);
		}
		SafeRelease(pBodyIndexFrameDescription);
		SafeRelease(pBodyIndexFrame);
	}

	//dxchange
	void Microsoft2Grabber::ProcessBody(int nBodyCount, IBody** ppBodies) {
		//loop through all people, find the first one that isn't null
		for (int i = 0; i < nBodyCount; ++i)
		{
			IBody* pBody = ppBodies[i];
			if (pBody) {

				BOOLEAN bTracked = false;
				HRESULT hr = pBody->get_IsTracked(&bTracked);

				if (SUCCEEDED(hr) && bTracked)
				{
					Joint joints[JointType_Count];
					hr = pBody->GetJoints(_countof(joints), joints);
					if (SUCCEEDED(hr))
					{
						//get the head position
						Joint head = joints[JointType_Head];
						m_headpos = Eigen::Vector3f(head.Position.X, head.Position.Y, head.Position.Z);
						m_headpos_Tracked = head.TrackingState;
						return; //no need to look further
					}
				}
			}
		}
		m_headpos_Tracked = TrackingState_NotTracked;
	}

	//dxchange
	void Microsoft2Grabber::BodyFrameArrived(IBodyFrameReference* pBodyFrameReference) {
		IBodyFrame* pBodyFrame = NULL;
		HRESULT hr = pBodyFrameReference->AcquireFrame(&pBodyFrame);
		if (FAILED(hr))
			return;

		IBody* ppBodies[BODY_COUNT] = { 0 };
		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}
		if (SUCCEEDED(hr))
		{
			ProcessBody(BODY_COUNT, ppBodies);
		}
		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
		SafeRelease(pBodyFrame);
	}

	void Microsoft2Grabber::FrameArrived(IMultiSourceFrameArrivedEventArgs *pArgs) {
		HRESULT hr;
		IMultiSourceFrameReference *pFrameReference = nullptr;

		//cout << "got a valid frame" << endl;
		hr = pArgs->get_FrameReference(&pFrameReference);
		if (SUCCEEDED(hr))
		{
			IMultiSourceFrame *pFrame = nullptr;
			hr = pFrameReference->AcquireFrame(&pFrame);
			if (FAILED(hr)) {
				cout << "fail on AcquireFrame" << endl;
			}
			IColorFrameReference* pColorFrameReference = nullptr;
			IDepthFrameReference* pDepthFrameReference = nullptr;
			IBodyIndexFrameReference* pBodyIndexFrameReference = nullptr;
			hr = pFrame->get_DepthFrameReference(&pDepthFrameReference);
			if (SUCCEEDED(hr))
				DepthFrameArrived(pDepthFrameReference);
			SafeRelease(pDepthFrameReference);


			hr = pFrame->get_ColorFrameReference(&pColorFrameReference);
			if (SUCCEEDED(hr))
				ColorFrameArrived(pColorFrameReference);
			SafeRelease(pColorFrameReference);

			hr = pFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
			if (SUCCEEDED(hr))
				BodyIndexFrameArrived(pBodyIndexFrameReference);
			SafeRelease(pBodyIndexFrameReference);

			//dxchange
			IBodyFrameReference* pBodyFrameReference = nullptr;
			hr = pFrame->get_BodyFrameReference(&pBodyFrameReference);
			if (SUCCEEDED(hr))
				BodyFrameArrived(pBodyFrameReference);
			SafeRelease(pBodyFrameReference);

			pFrameReference->Release();
		}
	}

#pragma endregion

	//Camera Functions
#pragma region Camera
	void Microsoft2Grabber::ColorFrameArrived(IColorFrameReference* pColorFrameReference) {
		IColorFrame* pColorFrame = NULL;
		HRESULT hr = pColorFrameReference->AcquireFrame(&pColorFrame);
		if (FAILED(hr)) {
			//cout << "Couldn't acquire color frame" << endl;
			return;
		}
		//cout << "got a color frame" << endl;
		INT64 nColorTime = 0;
		IFrameDescription* pColorFrameDescription = NULL;
		int nColorWidth = 0;
		int nColorHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nColorBufferSize = 0;
		RGBQUAD *pColorBuffer = NULL;

		// get color frame data
		hr = pColorFrame->get_RelativeTime(&nColorTime);
		if (SUCCEEDED(hr)) {
			hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
		}
		if (SUCCEEDED(hr)) {
			hr = pColorFrameDescription->get_Width(&nColorWidth);
		}
		if (SUCCEEDED(hr)) {
			hr = pColorFrameDescription->get_Height(&nColorHeight);
		}
		if (SUCCEEDED(hr)) {
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}
		if (SUCCEEDED(hr)) {
			if (imageFormat == ColorImageFormat_Bgra) {
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
			}
			else if (m_pColorRGBX) {
				pColorBuffer = m_pColorRGBX;
				nColorBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
			}
			else {
				hr = E_FAIL;
			}
			if (SUCCEEDED(hr)) {
				//WaitForSingleObject(hColorMutex,INFINITE);
				//cout << "creating the image" << endl;
				Mat tmp = Mat(m_colorSize, COLOR_PIXEL_TYPE, pColorBuffer, Mat::AUTO_STEP);
				boost::shared_ptr<Mat> img(new Mat());
				*img = tmp.clone();
				m_rgbTime = nColorTime;
				if (image_signal_->num_slots() > 0) {
					//cout << "img signal num slot!" << endl;
					image_signal_->operator()(img);
				}
				if (num_slots<sig_cb_microsoft_point_cloud_rgba>() > 0 || all_data_signal_->num_slots() > 0 || image_depth_image_signal_->num_slots() > 0)
					rgb_sync_.add0(img, m_rgbTime);
				//ReleaseMutex(hColorMutex);
			}
		}
		SafeRelease(pColorFrameDescription);
		SafeRelease(pColorFrame);
	}
#pragma endregion

	//Depth Functions
#pragma region Depth
	void Microsoft2Grabber::DepthFrameArrived(IDepthFrameReference* pDepthFrameReference) {
		IDepthFrame* pDepthFrame = NULL;
		HRESULT hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
		if (FAILED(hr))
			return;
		//cout << "got a depth frame" << endl;
		INT64 nDepthTime = 0;
		IFrameDescription* pDepthFrameDescription = NULL;
		int nDepthWidth = 0;
		int nDepthHeight = 0;
		nDepthBufferSize = 0;

		// get depth frame data
		hr = pDepthFrame->get_RelativeTime(&nDepthTime);
		if (SUCCEEDED(hr)) {
			hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
		}
		if (SUCCEEDED(hr)) {
			hr = pDepthFrameDescription->get_Width(&nDepthWidth);
		}
		if (SUCCEEDED(hr)) {
			hr = pDepthFrameDescription->get_Height(&nDepthHeight);
		}
		if (SUCCEEDED(hr)) {
			hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &m_pDepthBuffer);
			//WaitForSingleObject(hDepthMutex,INFINITE);
			Mat tmp = Mat(m_depthSize, DEPTH_PIXEL_TYPE, m_pDepthBuffer, Mat::AUTO_STEP);
			MatDepth depth_img = *((MatDepth*) &(tmp.clone()));
			m_depthTime = nDepthTime;
			if (depth_image_signal_->num_slots() > 0) {
				depth_image_signal_->operator()(depth_img);
			}
			if (num_slots<sig_cb_microsoft_point_cloud_rgba>() > 0 || all_data_signal_->num_slots() > 0 || image_depth_image_signal_->num_slots() > 0)
				rgb_sync_.add1(depth_img, m_depthTime);
			//ReleaseMutex(hDepthMutex);
		}
		SafeRelease(pDepthFrameDescription);
		SafeRelease(pDepthFrame);
	}
#pragma endregion

#pragma region Cloud
	void Microsoft2Grabber::imageDepthImageCallback(const boost::shared_ptr<Mat> &image,
		const MatDepth &depth_image)
	{
		// check if we have color point cloud slots
		m_cloudMutex.lock();
		if (point_cloud_rgba_signal_->num_slots() > 0 || all_data_signal_->num_slots() > 0)
			m_cloud = convertToXYZRGBAPointCloud(image, depth_image);
		if (point_cloud_rgba_signal_->num_slots() > 0)
			point_cloud_rgba_signal_->operator()(m_cloud);
		if (all_data_signal_->num_slots() > 0) {
			//boost::shared_ptr<KinectData> data (new KinectData(image,depth_image,*cloud));
			//all_data_signal_->operator()(data);
		}
		m_cloudMutex.unlock();

		if (image_depth_image_signal_->num_slots() > 0) {
			float constant = 1.0f;
			image_depth_image_signal_->operator()(image, depth_image, constant);
		}
	}

	void Microsoft2Grabber::GetPointCloudFromData(const boost::shared_ptr<Mat> &img, const MatDepth &depth, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, bool alignToColor, bool preregistered) const
	{
		if (!img || img->empty() || depth.empty()) {
			cout << "empty img or depth" << endl;
			return;
		}

		UINT16 *pDepth = (UINT16*) depth.data;
		int length = cDepthHeight * cDepthWidth, length2;
		HRESULT hr;
		if (alignToColor) {
			length2 = cColorHeight * cColorWidth;
			hr = m_pCoordinateMapper->MapColorFrameToCameraSpace(length, pDepth, length2, m_pCameraSpacePoints);
			if (FAILED(hr))
				throw exception("Couldn't map to camera!");
		}
		else {
			hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(length, pDepth, length, m_pCameraSpacePoints);
			if (FAILED(hr))
				throw exception("Couldn't map to camera!");
			hr = m_pCoordinateMapper->MapCameraPointsToColorSpace(length, m_pCameraSpacePoints, length, m_pColorCoordinates);
			if (FAILED(hr))
				throw exception("Couldn't map color!");
		}

		PointCloud<PointXYZRGBA>::iterator pCloud = cloud->begin();
		ColorSpacePoint *pColor = m_pColorCoordinates;
		CameraSpacePoint *pCamera = m_pCameraSpacePoints;
		float bad_point = std::numeric_limits<float>::quiet_NaN();
		int x, y, safeWidth = cColorWidth - 1, safeHeight = cColorHeight - 1;
		int width = alignToColor ? cColorWidth : cDepthWidth;
		int height = alignToColor ? cColorHeight : cDepthHeight;
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				PointXYZRGBA loc;
				Vec4b color;
				if (!preregistered && !alignToColor) {
					x = Clamp<int>(int(pColor->X), 0, safeWidth);
					y = Clamp<int>(int(pColor->Y), 0, safeHeight);
					//int index = y * cColorHeight + x;
					color = img->at<Vec4b>(y, x);
				}
				else
					color = img->at<Vec4b>(j, i);
				loc.b = color[0];
				loc.g = color[1];
				loc.r = color[2];
				loc.a = 255;
				if (pCamera->Z == 0) {
					loc.x = loc.y = loc.z = bad_point;
				}
				else {
					loc.x = pCamera->X;
					loc.y = pCamera->Y;
					loc.z = pCamera->Z;
				}
				//cout << "Iter: " << i << ", " << j << endl;
				*pCloud = loc;
				++pCamera; ++pCloud; ++pColor;
			}
		}
		img->release();
	}

	boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> Microsoft2Grabber::convertToXYZRGBAPointCloud(const boost::shared_ptr<cv::Mat> &image,
		const MatDepth &depth_image) const {
		boost::shared_ptr<PointCloud<PointXYZRGBA> > cloud(new PointCloud<PointXYZRGBA>);

		cloud->header.frame_id = "/microsoft_rgb_optical_frame";
		cloud->height = m_largeCloud ? cColorHeight : cDepthHeight;
		cloud->width = m_largeCloud ? cColorWidth : cDepthWidth;
		cloud->is_dense = false;
		cloud->points.resize(cloud->height * cloud->width);

		GetPointCloudFromData(image, depth_image, cloud, m_largeCloud, false);
		//GetPointCloudFromDataHD(image, depth_image, cloud, m_calib);

		cloud->sensor_origin_.setZero();
		cloud->sensor_orientation_.w() = 1.0;
		cloud->sensor_orientation_.x() = 0.0;
		cloud->sensor_orientation_.y() = 0.0;
		cloud->sensor_orientation_.z() = 0.0;
		return (cloud);
	}

	void Microsoft2Grabber::GetPointCloudFromDataHD(const MatDepth &depth, const boost::shared_ptr<cv::Mat> &imgHD, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, Calib calib)
	{
		cv::imshow("depth", depth);
		Mat imgHDdisplay;
		cv::resize(*imgHD, imgHDdisplay, Size(imgHD->size().width / 2, imgHD->size().height / 2));
		cv::imshow("rgbHD", imgHDdisplay);
		cv::waitKey(1);

		if (mesh_ptr_ == NULL)
			return;

		if (!icp_success || !icp_success_last_frame)
			return;

		//news and initialize
		{
			//pcl::ScopeTime t1("news and initialize"); //~40ms for news, couple of ms for re-initialize
			bool makeNew = arrayH != imgHD->size().height || arrayW < imgHD->size().width;
			if (makeNew) {
				arrayH = imgHD->size().height;
				arrayW = imgHD->size().width;
				d_best = new double*[arrayH];
				s_best = new Vertices*[arrayH];
				visited = new bool*[arrayH];
			}
			for (int i = 0; i < arrayH; i++) {
				if (makeNew) {
					d_best[i] = new double[arrayW];
					s_best[i] = new Vertices[arrayW];
					visited[i] = new bool[arrayW]; //will be initialized later
				}
				for (int j = 0; j < arrayW; j++) {
					d_best[i][j] = DBL_MAX;
					s_best[i][j].vertices.clear();
					s_best[i][j].vertices.reserve(3);
					visited[i][j] = false;
				}
			}
		}

		//put mesh into the correct orientation given by icp
		getOrientedMesh1();

		Mat projectedFaces = Mat::zeros(imgHD->size(), CV_8UC3);

		double rgbHDcx = calib.intrinsics_rgbHD(0, 2);
		double rgbHDcy = calib.intrinsics_rgbHD(1, 2);
		double rgbHDfx = calib.intrinsics_rgbHD(0, 0);
		double rgbHDfy = calib.intrinsics_rgbHD(1, 1);

		{
			pcl::ScopeTime t1("total run");
			cout << "faces size: " << mesh_ptr_->polygons.size() << endl;

			for (size_t i = 0; i < mesh_ptr_->polygons.size(); ++i) {
				//pcl::ScopeTime t2("loop through a face"); //couple of ms more on top of depth_first_search total
				Eigen::Vector3f pts[3];
				cv::Point2f pts2D[3];
				for (size_t j = 0; j < 3; ++j) {
					PointXYZ p0 = mesh_cld_ptr_->points[mesh_ptr_->polygons[i].vertices[j]];
					pts[j] = Eigen::Vector3f(p0.x, p0.y, p0.z);
					cv::Matx31f pp0(p0.x, p0.y, p0.z);
					//project onto rgbHD frame coord space
					cv::Matx31f tmp = m_calib.R_rgbHD.inv() * (pp0 - m_calib.T_rgbHD);
					cv::Point2f pt2D;
					pt2D.x = (tmp(0, 0) / tmp(2, 0) * rgbHDfx + rgbHDcx);
					pt2D.y = (tmp(1, 0) / tmp(2, 0) * rgbHDfy + rgbHDcy);
					pts2D[j] = pt2D;
					//circle(projectedFaces, pt2D, 1, Scalar(255));
				}
				if (faceNormalDotCentroid(pts[0], pts[1], pts[2], false) <= 0)
					continue; //the face is facing away from camera
				Eigen::Vector3f cent = (pts[0] + pts[1] + pts[2]) / 3.0;
				cv::Matx31f centr(cent.x(), cent.y(), cent.z());
				double dist = cv::norm(m_calib.R_rgbHD.inv() * (centr - m_calib.T_rgbHD)); // distance from centroid of face to camera focal point, used as depth value of the pixels as well as compare which face the pixel belongs to (the closer one)
				for (int ii = 0; ii < arrayH; ++ii) {
					for (int jj = 0; jj < arrayW; ++jj) {
						visited[ii][jj] = false;
					}
				}

				{
					//pcl::ScopeTime t3("depth_first_search total");  //anywhere from few ms to ~30ms
					//pcl::ScopeTime t3("grid search total");  //anywhere from few ms to ~30ms

					//do depth first search starting at each of the three vertices (one is enough usually, but if that one landed outside of the image, we will try another one.
					//if it didn't, then the ones after won't affect anything since the visited array keeps track of what's done already
#if 1
					depth_first_search(true, (int) pts2D[0].x, (int) pts2D[0].y, pts2D[0], pts2D[1], pts2D[2], dist, mesh_ptr_->polygons[i], s_best, d_best, visited, arrayW, arrayH);
					depth_first_search(true, (int) pts2D[1].x, (int) pts2D[1].y, pts2D[0], pts2D[1], pts2D[2], dist, mesh_ptr_->polygons[i], s_best, d_best, visited, arrayW, arrayH);
					depth_first_search(true, (int) pts2D[2].x, (int) pts2D[2].y, pts2D[0], pts2D[1], pts2D[2], dist, mesh_ptr_->polygons[i], s_best, d_best, visited, arrayW, arrayH);
#elif 0 //faster
					depth_first_search1(true, (int) pts2D[0].x, (int) pts2D[0].y, pts2D[0], pts2D[1], pts2D[2], dist, d_best, visited, arrayW, arrayH);
					depth_first_search1(true, (int) pts2D[1].x, (int) pts2D[1].y, pts2D[0], pts2D[1], pts2D[2], dist, d_best, visited, arrayW, arrayH);
					depth_first_search1(true, (int) pts2D[2].x, (int) pts2D[2].y, pts2D[0], pts2D[1], pts2D[2], dist, d_best, visited, arrayW, arrayH);	
#else
					//try gridsearch using for loops on min, max of x, y of p0, p1, p2 instead of recursion
#endif					
				}
			}
		}

		int emptyN = 0;
		int hitN = 0;
		{
			//pcl::ScopeTime t1("visualize s_best"); //couple of ms

			//test by visualize s_best: shading it gray with intensity = cos(face angle)*255
			for (int i = 0; i < arrayH; i++) {
				for (int j = 0; j < arrayW; j++) {
#if 1
					if (s_best[i][j].vertices.empty()) {
						emptyN++;
						continue;
					}
					hitN++;

					PointXYZ p0 = mesh_cld_ptr_->points[s_best[i][j].vertices[0]];
					PointXYZ p1 = mesh_cld_ptr_->points[s_best[i][j].vertices[1]];
					PointXYZ p2 = mesh_cld_ptr_->points[s_best[i][j].vertices[2]];
					Eigen::Vector3f ppp0(p0.x, p0.y, p0.z);
					Eigen::Vector3f ppp1(p1.x, p1.y, p1.z);
					Eigen::Vector3f ppp2(p2.x, p2.y, p2.z);
					double cosA = faceNormalDotCentroid(ppp0, ppp1, ppp2, true);
					//cosA = cosA>0 ? cosA : 0;



					//cosA = cosA>0 ? cosA : -cosA;
					//cosA = 1;
					//projectedFaces.at<uchar>(i, j) = (uchar) (cosA * 255);

					//if (cosA < 0.3)
					//{
					//	projectedFaces.at<Vec3b>(i, j)[0] = 255;
					//	projectedFaces.at<Vec3b>(i, j)[1] = 255;
					//	projectedFaces.at<Vec3b>(i, j)[2] = 0;
					//}



					//	//if (cosA <= 0) {
					//	//	projectedFaces.at<Vec3b>(i, j)[0] = 255;
					//	//	projectedFaces.at<Vec3b>(i, j)[1] = 255;
					//	//	projectedFaces.at<Vec3b>(i, j)[2] = 0;
					//	//}
					//	else 
						{
							projectedFaces.at<Vec3b>(i, j)[0] = 255 * cosA;
							projectedFaces.at<Vec3b>(i, j)[1] = 255 * cosA;
							projectedFaces.at<Vec3b>(i, j)[2] = 255 * cosA;
						}

#else
					if (d_best[i][j] == DBL_MAX)
						//if (!visited[i][j])
						continue;
					projectedFaces.at<uchar>(i, j) = 255;
#endif
				}
			}
		}
		cv::resize(projectedFaces, projectedFaces, Size(imgHD->size().width / 2, imgHD->size().height / 2));
		cv::imshow("projectedFaces", projectedFaces);
		cv::waitKey(1);
		
		cout << "emptyN: " << emptyN << endl;
		cout << "hitN: " << hitN << endl;

		for (int i = 0; i < arrayH; i++) {
			for (int j = 0; j < arrayW; j++) {
				PointXYZRGBA loc;

				//double x, y, z; 
				//z = d_best[i][j];
				//if (z == DBL_MAX)
				//	continue;
				////transform from rgbHD frame coord space into the world base coord
				//cv::Matx31f pt(x, y, z);
				//cv::Matx31f pt_ = m_calib.R_rgbHD * pt + m_calib.T_rgbHD;
				//loc.x = pt_(0, 0);
				//loc.y = pt_(1, 0);
				//loc.z = pt_(2, 0);



				//if (s_best[i][j].vertices.empty())
				//	continue;
				//PointXYZ p0 = mesh_cld_ptr_->points[s_best[i][j].vertices[0]];
				//PointXYZ p1 = mesh_cld_ptr_->points[s_best[i][j].vertices[1]];
				//PointXYZ p2 = mesh_cld_ptr_->points[s_best[i][j].vertices[2]];
				//Eigen::Vector3f ppp0(p0.x, p0.y, p0.z);
				//Eigen::Vector3f ppp1(p1.x, p1.y, p1.z);

				//Eigen::Vector3f ppp2(p2.x, p2.y, p2.z);
				//double ppx, ppy;
				//ppx = (j - rgbHDcx) / rgbHDfx;
				//ppy = (i - rgbHDcy) / rgbHDfy;
				//Eigen::Vector3f c1(ppx, ppy, 1);
				//Eigen::Vector3f c2 = ppp0 - ppp1;
				//Eigen::Vector3f c3 = ppp0 - ppp2;
				//Eigen::Matrix3f m;
				//m  << c1.x(), c2.x(), c3.x()
				//	, c1.y(), c2.y(), c3.y()
				//	, c1.z(), c2.z(), c3.z();
				//Eigen::Vector3f zuv = m.inverse() * ppp0;

				//Eigen::Vector3f pp = c1 * zuv(0, 0);
				//loc.x = pp(0, 0);
				//loc.y = pp(1, 0);
				//loc.z = pp(2, 0);

				//cout << "p0: " << ppp0 << endl;
				//cout << "p1: " << ppp1 << endl;
				//cout << "p2: " << ppp2 << endl;
				//cout << "m: " << m << endl;
				//cout << "zuv: " << zuv << endl;
				//cout << "location: " << pp << endl;
				//cout << "loc: " << loc;

				//system("pause");





				//Eigen::Vector3f cent = (ppp0 + ppp1 + ppp2) / 3.0;
				//loc.x = cent.x();
				//loc.y = cent.y();
				//loc.z = cent.z();



				if (s_best[i][j].vertices.empty())
					continue;
				Eigen::Vector3f pts[3];
				Eigen::Vector2f pts2D[3];
				for (size_t k = 0; k < 3; ++k) {
					PointXYZ p0 = mesh_cld_ptr_->points[s_best[i][j].vertices[k]];
					pts[k] = Eigen::Vector3f(p0.x, p0.y, p0.z);
					cv::Matx31f pp0(p0.x, p0.y, p0.z);
					//project onto rgbHD frame coord space
					cv::Matx31f tmp = m_calib.R_rgbHD.inv() * (pp0 - m_calib.T_rgbHD);
					pts2D[k].x() = (tmp(0, 0) / tmp(2, 0) * rgbHDfx + rgbHDcx);
					pts2D[k].y() = (tmp(1, 0) / tmp(2, 0) * rgbHDfy + rgbHDcy);
				}
				//find (i+0.5, j+0.5) as linear combo of the projected vertices
				//i.e. pts2D[0] + a*(pts2D[1] - pts2D[0]) + b*(pts2D[2] - pts2D[0]) = (i+0.5, j+0.5)
				Eigen::Vector2f c1 = pts2D[1] - pts2D[0];
				Eigen::Vector2f c2 = pts2D[2] - pts2D[0];
				Eigen::Matrix2f m;
				m << c1.x(), c2.x(), c1.y(), c2.y();
				Eigen::Vector2f ab = m.inverse() * (Eigen::Vector2f(j + 0.5, i + 0.5) - pts2D[0]);
				Eigen::Vector3f pp = pts[0] + ab(0, 0)*(pts[1] - pts[0]) + ab(1, 0)*(pts[2] - pts[0]);
				loc.x = pp.x();
				loc.y = pp.y();
				loc.z = pp.z();

				loc.b = imgHD->at<cv::Vec3b>(i, j)[0];
				loc.g = imgHD->at<cv::Vec3b>(i, j)[1];
				loc.r = imgHD->at<cv::Vec3b>(i, j)[2];
				loc.a = 255;
				cloud->push_back(loc);
			}
		}


		//1. remove surfaces not facing the HD camera
		//1.1 put mesh into the correct orientation

		//1.2 calculate dot product of surface centroid (for triangle, it's 1/3(p1+p2+p3)) and surface normal, delete the surface if negative
		//2. perspective project the remaining surfaces onto HD camera plane, marking which surface each pixel belongs to. If two surfaces project to the same pixel, the pixel belongs to the surface closer to HD camera plane
		//3. test by visualizing the projected image, with gray scale intensity proportional to cos(angle) and clamped to minimum zero (i.e. faces facing the other way will be black)
		//4. calculate the 3D coordinate of each pixel and form the HD color point cloud
	}

	bool Microsoft2Grabber::depth_first_search(bool first, int x, int y, Point2f p1, Point2f p2, Point2f p3, double dist, Vertices face, Vertices **s_best, double **d_best, bool** visited, int width, int height) {
		if (x < 0 || x >= width || y < 0 || y >= height)
			return false;
		else if (visited[y][x])
			return false;
		else if (!first && !isInsideTriangle(Point2f(x + 0.5, y + 0.5), p1, p2, p3))
			return false;

		visited[y][x] = true;
		if (dist < d_best[y][x]){
			d_best[y][x] = dist;
			s_best[y][x] = face;
		}

		// try each neighbour
		depth_first_search(false, x - 1, y, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x + 1, y, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x, y - 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x, y + 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x - 1, y - 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x + 1, y - 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x - 1, y + 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);
		depth_first_search(false, x + 1, y + 1, p1, p2, p3, dist, face, s_best, d_best, visited, width, height);

		return true;
	}
	bool Microsoft2Grabber::depth_first_search1(bool first, int x, int y, Point2f p1, Point2f p2, Point2f p3, double dist, double **d_best, bool** visited, int width, int height) {
		if (x < 0 || x >= width || y < 0 || y >= height)
			return false;
		else if (visited[y][x])
			return false;
		else if (!first && !isInsideTriangle(Point2f(x + 0.5, y + 0.5), p1, p2, p3))
			return false;

		visited[y][x] = true;
		if (dist < d_best[y][x]){
			d_best[y][x] = dist;
		}

		// try each neighbour
		depth_first_search1(false, x - 1, y, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x + 1, y, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x, y - 1, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x, y + 1, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x - 1, y - 1, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x + 1, y - 1, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x - 1, y + 1, p1, p2, p3, dist, d_best, visited, width, height);
		depth_first_search1(false, x + 1, y + 1, p1, p2, p3, dist, d_best, visited, width, height);

		return true;
	}

	boost::shared_ptr<pcl::PolygonMesh> Microsoft2Grabber::getOrientedMesh() {
		//pcl::ScopeTime t1("getOrientedMesh total"); //~50ms
		boost::shared_ptr<pcl::PolygonMesh> mesh(new pcl::PolygonMesh(*mesh_ptr_));
		pcl::PointCloud<PointXYZ> cloud;
		{
			//pcl::ScopeTime t1("getOrientedMesh fromPCLPointCloud2");
			pcl::fromPCLPointCloud2(mesh->cloud, cloud);
		}
		//inverse transform
		{
			//pcl::ScopeTime t1("getOrientedMesh inverse transform");
			pcl::transformPointCloud(cloud, cloud, icp_camera_pose->inverse());
		}
		if (!usingEyediap) {
			//pcl::ScopeTime t1("getOrientedMesh mirror x-z plane");
			//mirror on x-z plane (make all y = -y)
			pcl::PointCloud<PointXYZ>::iterator itr = cloud.begin();
			while (itr != cloud.end()) {
				(*itr).y = -(*itr).y;
				itr++;
			}
		}
		else if (usingEyediap) {
			//pcl::ScopeTime t1("getOrientedMesh depth frame coord to base coord");
			//from depth frame coordinate into base coordinate
			Eigen::Affine3f transf_mesh3 = Eigen::Affine3f::Identity();
			transf_mesh3.linear() << m_calib.R_d(0, 0), m_calib.R_d(0, 1), m_calib.R_d(0, 2),
				m_calib.R_d(1, 0), m_calib.R_d(1, 1), m_calib.R_d(1, 2),
				m_calib.R_d(2, 0), m_calib.R_d(2, 1), m_calib.R_d(2, 2);
			transf_mesh3.translation() << m_calib.T_d(0, 0), m_calib.T_d(1, 0), m_calib.T_d(2, 0);
			pcl::transformPointCloud(cloud, cloud, transf_mesh3);
			//dxnote: m_calib.R_d and T_d tranforms from depth frame coordinate space to world (base) coord
		}
		{
			//pcl::ScopeTime t1("getOrientedMesh toPCLPointCloud2");
			pcl::toPCLPointCloud2(cloud, mesh->cloud);
		}
		return mesh;
	}

	void Microsoft2Grabber::getOrientedMesh1() {
		//pcl::ScopeTime t1("getOrientedMesh total"); //~1ms
			{
				//pcl::ScopeTime t1("getOrientedMesh fromPCLPointCloud2");
				pcl::fromPCLPointCloud2(mesh_ptr_->cloud, *mesh_cld_ptr_);
			}
		//inverse transform
		{
			//pcl::ScopeTime t1("getOrientedMesh inverse transform");
			pcl::transformPointCloud(*mesh_cld_ptr_, *mesh_cld_ptr_, icp_camera_pose->inverse());
		}
		if (!usingEyediap) {
			//pcl::ScopeTime t1("getOrientedMesh mirror x-z plane");
			//mirror on x-z plane (make all y = -y)
			pcl::PointCloud<PointXYZ>::iterator itr = mesh_cld_ptr_->begin();
			while (itr != mesh_cld_ptr_->end()) {
				(*itr).y = -(*itr).y;
				itr++;
			}
		}
		else if (usingEyediap) {
			//pcl::ScopeTime t1("getOrientedMesh depth frame coord to base coord");
			//from depth frame coordinate into base coordinate
			Eigen::Affine3f transf_mesh3 = Eigen::Affine3f::Identity();
			transf_mesh3.linear() << m_calib.R_d(0, 0), m_calib.R_d(0, 1), m_calib.R_d(0, 2),
				m_calib.R_d(1, 0), m_calib.R_d(1, 1), m_calib.R_d(1, 2),
				m_calib.R_d(2, 0), m_calib.R_d(2, 1), m_calib.R_d(2, 2);
			transf_mesh3.translation() << m_calib.T_d(0, 0), m_calib.T_d(1, 0), m_calib.T_d(2, 0);
			pcl::transformPointCloud(*mesh_cld_ptr_, *mesh_cld_ptr_, transf_mesh3);
		}
	}

	void Microsoft2Grabber::GetPointCloudFromData2(const boost::shared_ptr<Mat> &img, const MatDepth &depth, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, Calib calib) const
	{
		if (!img || img->empty() || depth.empty()) {
			cout << "empty img or depth" << endl;
			return;
		}

		PointCloud<PointXYZRGBA>::iterator pCloud = cloud->begin();
		float bad_point = std::numeric_limits<float>::quiet_NaN();
		int safeWidth = depth.size().width - 1;
		int safeHeight = depth.size().height - 1;
		ushort minValidD = 500; //minimum valid depth value
		double alpha1 = calib.alpha(0, 0);
		double alpha2 = calib.alpha(0, 1);
		double dc1 = calib.k_coefficients(0, 0);
		double dc2 = calib.k_coefficients(0, 1);
		double dcx = calib.intrinsics_d(0, 2);
		double dcy = calib.intrinsics_d(1, 2);
		double dfx = calib.intrinsics_d(0, 0);
		double dfy = calib.intrinsics_d(1, 1);
		double rgbcx = calib.intrinsics_rgb(0, 2);
		double rgbcy = calib.intrinsics_rgb(1, 2);
		double rgbfx = calib.intrinsics_rgb(0, 0);
		double rgbfy = calib.intrinsics_rgb(1, 1);
		for (int i = 0; i < depth.size().height; i++) {
			for (int j = 0; j < depth.size().width; j++) {
				PointXYZRGBA loc;
				double d = depth.at<ushort>(i, j);
				if (d < minValidD) {
					loc.x = loc.y = loc.z = bad_point;
					loc.r = loc.g = loc.b = loc.a = 0;
					pCloud++;
				}
				else {
					//(u, v, d) depth camera coordinate into (x, y, z) world coordinate based on RGB camera as center basis, i.e. disparity2world
#if 0
					//1. undistort d to get z, i.e. disparity2depth, undistort_disparity
					double beta = calib.beta.at<float>(i, j);
					double disp_k = d + beta * exp(alpha1 - alpha2 * d);//undistortion factor
					double z = 1 / (dc2 * disp_k + dc1);
#else //do the undistortion in DepthMat instead
					double z = d / 1000.0f;
#endif
					//2. (u, v) to (x/z, y/z)*z, i.e. get_dpoint_direction, --distort--X
					//note: no need to distort, since RGB video is given in pre-processed undistorted version
					double x, y;
					x = (j - dcx) / dfx * z;
					y = (i - dcy) / dfy * z;
					//3. transform (x, y, z) into RGB camera as center basis
					Matx31f Xd;
					Xd << x, y, z;
					Matx31f Xw = calib.R_d * Xd + calib.T_d;
					//4.project the point onto rgb camera image plane and get the rgb value
					double rgbx, rgby;
					Matx31f Xc = calib.R_rgb.inv() * (Xw - calib.T_rgb);
					rgbx = Xc(0, 0) / Xc(2, 0) * rgbfx + rgbcx;
					rgby = Xc(1, 0) / Xc(2, 0) * rgbfy + rgbcy;
					int xx = Clamp<int>((int) rgbx, 0, safeWidth);
					int yy = Clamp<int>((int) rgby, 0, safeHeight);
					Vec3b color;
					color = img->at<Vec3b>(yy, xx);

#if 1
					loc.x = Xw(0, 0);
					loc.y = Xw(1, 0);
					loc.z = Xw(2, 0);
#else //just trying some stuffs, don't use this
					loc.x = x;
					loc.y = y;
					loc.z = z;
#endif

					loc.b = color[0];
					loc.g = color[1];
					loc.r = color[2];
					loc.a = 255;
					*pCloud = loc;
					pCloud++;
				}
			}
		}

		img->release();
	}

	boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> Microsoft2Grabber::convertToXYZRGBAPointCloud2(const boost::shared_ptr<cv::Mat> &image,
		const MatDepth &depth_image, const boost::shared_ptr<cv::Mat> &imageHD, Calib calib) {
		boost::shared_ptr<PointCloud<PointXYZRGBA> > cloud(new PointCloud<PointXYZRGBA>);

		cloud->header.frame_id = "/microsoft_rgb_optical_frame";
		cloud->is_dense = false;
#if 0
		cloud->height = cDepthHeight1;
		cloud->width = cDepthWidth1;
		cloud->points.resize(cloud->height * cloud->width);
		GetPointCloudFromData2(image, depth_image, cloud, calib);
		cloud->sensor_origin_.setZero();
		cloud->sensor_orientation_.w() = 1.0;
		cloud->sensor_orientation_.x() = 0.0;
		cloud->sensor_orientation_.y() = 0.0;
		cloud->sensor_orientation_.z() = 0.0;
#else
		if (trackingOnly)
			GetPointCloudFromDataHD(depth_image, imageHD, cloud, calib);
#endif
		return (cloud);
	}

	Eigen::Matrix3f Microsoft2Grabber::alignVectors(Eigen::Vector3f v1, Eigen::Vector3f v2) {
		//http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
		//answer by: Jur van den Berg
		Eigen::Vector3f a = v1 / v1.norm();
		Eigen::Vector3f b = v2 / v2.norm();
		Eigen::Vector3f v = a.cross(b);
		float s = v.norm();
		float c = a.dot(b);
		Eigen::Matrix3f vmtx;
		vmtx <<
			0, -v.z(), v.y(),
			v.z(), 0, -v.x(),
			-v.y(), v.x(), 0;
		Eigen::Matrix3f R = Eigen::Matrix3f::Identity() + vmtx + vmtx*vmtx*(1 - c) / (s * s);
		//cout << "R: " << R << endl;
		return R;
	}

	void Microsoft2Grabber::visualizeCloud(boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> cloud) {
		viewer->removePointCloud("cloud");
		viewer->addPointCloud(cloud, "cloud");
		//viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "cloud");
		viewer->spinOnce();
	}

	void Microsoft2Grabber::transform(double w, double h, double focalLength, Mat &mat, bool selectEyeRegions) {
		//projecting onto a plane of size w X h (in meters) with its location and orientation specified by tf
		//using perspective projection with focalLength (in meters)
		//output into mat

		if (m_cloud == NULL)
			return;
		if (m_cloud->empty() && !trackingOnly)
			return;

		visualizeCloud(m_cloud);
		//return;

		cout << "frameNum: " << frameNum << endl;

		PointCloud<PointXYZRGBA>::iterator pCloud;
		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> transf_cloud(new pcl::PointCloud<pcl::PointXYZRGBA>());
#if 0
		//crop out the head volume from m_cloud and put into transf_cloud, so the computations later on aren't as heavy
		double HEAD_RAD = (w + h)*0.5; //sphere radius in meters
		Point3f headpos(tf->translation().x(), tf->translation().y(), tf->translation().z());
		cout << "headpos: " << headpos << endl;
		m_cloudMutex.lock();
		pCloud = m_cloud->begin();
		while (pCloud != m_cloud->end()) {
			Point3f pos((*pCloud).x, (*pCloud).y, (*pCloud).z);
			if (norm(headpos - pos) < HEAD_RAD)
			{
				transf_cloud->push_back(*pCloud);
			}
			pCloud++;
		}
		m_cloudMutex.unlock();
#else //since we are using mesh projection method for forming the point cloud, it will only contain the head pixels, no need to crop the head again
		m_cloudMutex.lock();
		pcl::copyPointCloud(*m_cloud, *transf_cloud);
		m_cloudMutex.unlock();
#endif

		cout << "face cloud size: " << transf_cloud->size() << endl;
		//save the face cloud points into file for debugging
		//pcl::io::savePCDFileASCII("faceCloud.pcd", *transf_cloud);

		//rotate and translate the cloud so the head is centered and frontal
		Eigen::Affine3f transf;
		Eigen::Affine3f transf1 = Eigen::Affine3f::Identity();
		Eigen::Affine3f transf2 = Eigen::Affine3f::Identity();
		Eigen::Affine3f transf3 = Eigen::Affine3f::Identity();
		Eigen::Affine3f transf4 = Eigen::Affine3f::Identity();
		Eigen::Affine3f transf5 = Eigen::Affine3f::Identity();

#if 0
		transf1.translation().x() = -icp_camera_pose->translation().x();
		transf1.translation().y() = -icp_camera_pose->translation().y();
		transf1.translation().z() = -icp_camera_pose->translation().z();

		if (usingEyediap)
			transf2.rotate(Eigen::AngleAxisf(M_PI, Eigen::Vector3f::UnitY()));//rotate 180 degrees on Y-axis (points up) so the cloud faces us

		pcl::transformPointCloud(*transf_cloud, *transf_cloud, transf2 * transf1);

		if (!useNoseTip) {
			Eigen::Vector3f ea = icp_camera_pose->linear().eulerAngles(0, 1, 2);
			if (usingEyediap) {
				transf3.rotate(Eigen::AngleAxisf(-ea[0], Vector3fMap::UnitX()));
				transf3.rotate(Eigen::AngleAxisf(-ea[1], Vector3fMap::UnitY()));
				transf3.rotate(Eigen::AngleAxisf(ea[2], Vector3fMap::UnitZ()));
			}
			else {
				transf3.rotate(Eigen::AngleAxisf(-ea[0], Vector3fMap::UnitX()));
				transf3.rotate(Eigen::AngleAxisf(ea[1], Vector3fMap::UnitY()));
				transf3.rotate(Eigen::AngleAxisf(-ea[2], Vector3fMap::UnitZ()));
			}
		}
		else {
			getNoseTip(transf_cloud);

			//calculate rotations of the head pose using noseTip
			//Eigen::Vector3f noseTipV = noseTip - m_headpos;
			Eigen::Vector3f noseTipV = noseTip;
			noseTip_head_rot_v = (noseTipV / noseTipV.norm()).cross(-Eigen::Vector3f::UnitZ());
			noseTip_head_rot_v = noseTip_head_rot_v / noseTip_head_rot_v.norm();
			noseTip_head_rot_a = acos(noseTipV.dot(-Eigen::Vector3f::UnitZ()) / noseTipV.norm());
		}
		//transform the reverse of the base head pose so it's frontal
		transf3.rotate(Eigen::AngleAxisf(noseTip_head_rot_a, noseTip_head_rot_v));
		transf3.rotate(Eigen::AngleAxisf(-5 * M_PI / 180, Eigen::Vector3f::UnitX()));
		//translate a little distance from the focal point, so the perspective projection looks good
		transf3.translation() = Eigen::Vector3f(0, 0, 2 * focalLength);

		//transf = transf3 * transf2 * transf1;
		pcl::transformPointCloud(*transf_cloud, *transf_cloud, transf3);

		if (useNoseTip) {
			//display a red nose tip so we are sure nose tip is correct
			pcl::PointXYZ nose_pt(noseTip.x(), noseTip.y(), noseTip.z());
			//nose_pt = pcl::transformPoint(nose_pt, transf);
			nose_pt = pcl::transformPoint(nose_pt, transf3);
			viewer->removeAllShapes();
			viewer->addSphere(nose_pt, 0.01, 1.0, 0.0, 0.0, "nose_pt");

			visualizeCloud(transf_cloud);
		}

#else	
		bool recalculateHeadCenter = selectEyeRegions || !baseHeadPose.isCalculated;
		baseHeadPose.calculate(mesh_ptr_, m_calib, icp_camera_pose, recalculateHeadCenter);
		transf1.translate(-baseHeadPose.headCenter);
		Eigen::Vector3f ea = icp_camera_pose->linear().eulerAngles(0, 1, 2);
		if (usingEyediap) {
			transf2.rotate(Eigen::AngleAxisf(ea[0], Vector3fMap::UnitX()));
			transf2.rotate(Eigen::AngleAxisf(-ea[1], Vector3fMap::UnitY()));
			transf2.rotate(Eigen::AngleAxisf(-ea[2], Vector3fMap::UnitZ()));
		}
		else {
			transf2.rotate(Eigen::AngleAxisf(-ea[0], Vector3fMap::UnitX()));
			transf2.rotate(Eigen::AngleAxisf(ea[1], Vector3fMap::UnitY()));
			transf2.rotate(Eigen::AngleAxisf(-ea[2], Vector3fMap::UnitZ()));
		}

		////transform the reverse of the base head pose
		Eigen::Vector3f noseTipV = baseHeadPose.noseTip - baseHeadPose.headCenter;
		noseTipV = transf2.linear() * noseTipV + transf2.translation();
		Eigen::Matrix3f RNoseT = alignVectors(noseTipV, Vector3fMap::UnitZ());
		noseTipV = RNoseT * noseTipV;
		transf3.rotate(RNoseT);

		Eigen::Vector3f headTopV = baseHeadPose.headTop - baseHeadPose.headCenter;
		headTopV = transf2.linear() * headTopV + transf2.translation();
		headTopV = RNoseT * headTopV;
		headTopV.z() = 0;
		Eigen::Matrix3f RHeadT = alignVectors(headTopV, Vector3fMap::UnitY());
		headTopV = RHeadT * headTopV;
		transf3.rotate(RHeadT);

		//tilt the chin down a bit
		transf4.rotate(Eigen::AngleAxisf(40 * M_PI / 180, Vector3fMap::UnitX()));

		if (usingEyediap) {
			//rotate 180 degrees on Y-axis (points up) so the cloud faces us
			transf5.rotate(Eigen::AngleAxisf(M_PI, Vector3fMap::UnitY()));
		}

		//translate a little distance from the focal point, so the perspective projection looks good
		transf5.translation() = Eigen::Vector3f(0, 0, 2 * focalLength);

		transf = transf5 * transf4 * transf3 * transf2 * transf1;
		pcl::transformPointCloud(*transf_cloud, *transf_cloud, transf);
#endif 

#if 0
		//visualize if the mesh is indeed fitted on the point cloud
		boost::shared_ptr<pcl::PolygonMesh> mesh = getOrientedMesh();
		pcl::PointCloud<PointXYZ> cloud;
		pcl::fromPCLPointCloud2(mesh->cloud, cloud);
		pcl::transformPointCloud(cloud, cloud, transf);
		pcl::toPCLPointCloud2(cloud, mesh->cloud);

	#if 1
			viewer->removePolygonMesh();
			viewer->addPolygonMesh(*mesh);
	#else
			viewer->removeAllShapes();
			viewer->addPolylineFromPolygonMesh(*mesh);
	#endif

	#if 1
			viewer->removeAllShapes();
			//display a red nose tip so we are sure nose tip is correct
			pcl::PointXYZ nose_pt(baseHeadPose.noseTip.x(), baseHeadPose.noseTip.y(), baseHeadPose.noseTip.z());
			nose_pt = pcl::transformPoint(nose_pt, transf);
			viewer->addSphere(nose_pt, 0.02, 1.0, 0.0, 0.0, "nose_pt");

			//display a green head center
			pcl::PointXYZ head_pt(baseHeadPose.headCenter.x(), baseHeadPose.headCenter.y(), baseHeadPose.headCenter.z());
			head_pt = pcl::transformPoint(head_pt, transf);
			viewer->addSphere(head_pt, 0.035, 0.0, 1.0, 0.0, "head_pt");

			//display a green head center
			pcl::PointXYZ head_tp(baseHeadPose.headTop.x(), baseHeadPose.headTop.y(), baseHeadPose.headTop.z());
			head_tp = pcl::transformPoint(head_tp, transf);
			viewer->addSphere(head_tp, 0.02, 1.0, 0.0, 0.0, "head_tp");

			//pcl::PointXYZ head_tpV(headTopV.x(), headTopV.y(), headTopV.z());
			//viewer->addSphere(head_tpV, 0.04, 0.0, 0.0, 1.0, "head_tpV");
			//pcl::PointXYZ noseTip_v(noseTipV.x(), noseTipV.y(), noseTipV.z());
			//viewer->addSphere(noseTip_v, 0.04, 0.0, 1.0, 0.0, "noseTipV");

	#endif

		visualizeCloud(transf_cloud);
		return;
#endif


		////orthogonal projection onto plane at origin for visualization
		//// Create a set of planar coefficients with X=Y=0,Z=1
		//pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients());
		//coefficients->values.resize(4);
		//coefficients->values[0] = coefficients->values[1] = 0;
		//coefficients->values[2] = 1.0;
		//coefficients->values[3] = 0;
		//// Create the filtering object
		//pcl::ProjectInliers<pcl::PointXYZRGBA> proj;
		//proj.setModelType(pcl::SACMODEL_PLANE);
		//proj.setInputCloud(transf_cloud);
		//proj.setModelCoefficients(coefficients);
		//proj.filter(*transf_cloud);

		////perspective project the cloud for visualization
		//pCloud = transf_cloud->begin();
		//while (pCloud != transf_cloud->end()) {
		//	double posX;
		//	double posY;
		//	posX = (*pCloud).x / (*pCloud).z * focalLength; 
		//	posY = (*pCloud).y / (*pCloud).z * focalLength;
		//	(*pCloud).x = posX;
		//	(*pCloud).y = posY;
		//	(*pCloud).z = focalLength;
		//	pCloud++;
		//}

		//turn cloud into mat
		//use image resolution of 100X100 (since we have around 10000 cloud points)
		//int MAT_SZ = 350;
		int MAT_SZ = 700;
		double pxSzW = w / MAT_SZ;
		double pxSzH = h / MAT_SZ;
		if (!mat.empty())
			mat.release();
		mat.create(Size(MAT_SZ, MAT_SZ), CV_8UC3);
		//initialize the mat
		Mat_<Vec3b>::iterator pI = mat.begin<Vec3b>();
		while (pI != mat.end<Vec3b>()) {
			(*pI) = Vec3b(0, 0, 0);
			//(*pI) = Vec3b(0, 255, 0); //to green for debug purpose
			pI++;
		}

		//mask for inpainting
		Mat mask = Mat::zeros(Size(MAT_SZ, MAT_SZ), CV_8U);
		for (int i = 0; i < MAT_SZ; i++)
			for (int j = 0; j < MAT_SZ; j++)
				mask.at<uchar>(Point(i, j)) = 255;
		//double headRadPx = HEAD_RAD / pxSzW * 0.3;
		//for (int i = (int) (MAT_SZ - headRadPx) / 2; i < (int) (MAT_SZ + headRadPx) / 2; i++) {
		//	for (int j = (int) (MAT_SZ - headRadPx) / 2; j < (int) (MAT_SZ + headRadPx) / 2; j++) {
		//		mask.at<uchar>(i, j) = 255;
		//	}
		//}

		//a 2D array saving projected XYZRGBA points for each pixel
		std::vector<PointXYZRGBA> **projected_pts = new std::vector<PointXYZRGBA>*[MAT_SZ];
		for (int i = 0; i < MAT_SZ; i++) {
			projected_pts[i] = new std::vector<PointXYZRGBA>[MAT_SZ];
			for (int j = 0; j < MAT_SZ; j++)
				projected_pts[i][j] = std::vector<PointXYZRGBA>();
		}

		pCloud = transf_cloud->begin();
		while (pCloud != transf_cloud->end()) {
			double posX;
			double posY;
			//posX = (*pCloud).x; 
			//posY = (*pCloud).y;

			//perspective projection
			posX = (*pCloud).x / (*pCloud).z * focalLength;
			posY = (*pCloud).y / (*pCloud).z * focalLength;

			//int pxX = ((-posX + w / 2) / pxSzW + 0.5);
			//int pxY = ((-posY + w / 2) / pxSzH + 0.5);
			int pxX = ((-posX + w / 2) / pxSzW); //dx: don't know why i added 0.5 before but it doesn't seem to make a difference
			int pxY = ((-posY + w / 2) / pxSzH);
			if (pxX >= 0 && pxX < MAT_SZ && pxY >= 0 && pxY < MAT_SZ)
				projected_pts[pxX][pxY].push_back(*pCloud);
			pCloud++;
		}

		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr debugCloud(new pcl::PointCloud<pcl::PointXYZRGBA>());

		int count_hit = 0;
		//dxtodo: can speed this up by only processing the ROI of the eyes instead of the whole face
		for (int i = 0; i< MAT_SZ; i++) {
			for (int j = 0; j< MAT_SZ; j++) {
				//when multiple cloud points map into the same pixel, we have two alternatives:
				//method 1: find the nearest depth pixel and set mat's rgb to it
				std::vector<PointXYZRGBA>::iterator nearestPI;
				float leastZ = FLT_MAX;
				std::vector<PointXYZRGBA>::iterator pI = projected_pts[i][j].begin();
				while (pI != projected_pts[i][j].end()) {
					if ((*pI).z < leastZ) {
						mat.at<cv::Vec3b>(Point(i, j))[0] = (*pI).b;
						mat.at<cv::Vec3b>(Point(i, j))[1] = (*pI).g;
						mat.at<cv::Vec3b>(Point(i, j))[2] = (*pI).r;
						nearestPI = pI;
						leastZ = (*pI).z;
					}
					pI++;
				}
				if (leastZ != FLT_MAX) {
					count_hit++;
					mat.at<cv::Vec3b>(Point(i, j))[0] = (*nearestPI).b;
					mat.at<cv::Vec3b>(Point(i, j))[1] = (*nearestPI).g;
					mat.at<cv::Vec3b>(Point(i, j))[2] = (*nearestPI).r;

					mask.at<uchar>(Point(i, j)) = 0;

					debugCloud->push_back(*nearestPI);
				}
				////method 2: set mat's rgb to the average color of the points
				//double sumb = 0, sumg = 0, sumr = 0;
				//std::vector<PointXYZRGBA>::iterator pI = count[i][j].begin();
				//while (pI != count[i][j].end()) {
				//	sumb += (*pI).b;
				//	sumg += (*pI).g;
				//	sumr += (*pI).r;
				//	pI++;
				//}
				//if (sumb != 0 || sumg != 0 || sumr != 0) {
				//	mat.at<cv::Vec3b>(i, j)[0] = sumb / count[i][j].size();
				//	mat.at<cv::Vec3b>(i, j)[1] = sumg / count[i][j].size();
				//	mat.at<cv::Vec3b>(i, j)[2] = sumr / count[i][j].size();
				//}
			}
		}
		cout << "count_hit: " << count_hit << endl;

		//int count_missed = 0;
		//int llim = 0, rlim = MAT_SZ, tlim = 0, blim = MAT_SZ;
		//for (int i = 0; i < MAT_SZ; i++) {
		//	for (int j = 0; j < MAT_SZ; j++) {
		//		Point point = Point(i, j);
		//		if (!point.inside(eyeRects[0]) && !point.inside(eyeRects[1]))
		//			continue;//only deal with pixels inside the ROI of eyes

		//		if (mask.at<uchar>(Point(i, j)) != 0) {//we're at pixels that no points landed on
		//			count_missed++;
		//			//use the closest point landed in the surrounding eight pixels
		//			//std::vector<pcl::PointXYZRGBA> pts;
		//			float minDist = FLT_MAX;
		//			pcl::PointXYZRGBA minDistPt;
		//			float cX, cY; //the center in meters of the pixel
		//			cX = -((i + 0.5) * pxSzW - w / 2);
		//			cY = -((j + 0.5) * pxSzW - w / 2);
		//			for (int ii = max(i - 1, llim); ii < min(i + 1, rlim); ii++) {
		//				for (int jj = max(j - 1, tlim); jj < min(j + 1, blim); jj++) {
		//					if (ii == i && jj == j)
		//						continue;
		//					for (int kk = 0; kk < projected_pts[ii][jj].size(); kk++) {
		//						pcl::PointXYZRGBA pt = projected_pts[ii][jj][kk];
		//						float posX = pt.x / pt.z * focalLength;
		//						float posY = pt.y / pt.z * focalLength;
		//						float dist = sqrt(pow(posX - cX, 2) + pow(posY - cY, 2));
		//						if (dist < minDist) {
		//							minDist = dist;
		//							minDistPt = pt;
		//							projected_pts[i][j].clear();
		//							projected_pts[i][j].push_back(pt);
		//						}
		//					}
		//				}
		//			}
		//			//if (projected_pts[i][j].size() > 0) 
		//			if (minDist != FLT_MAX)
		//			{
		//				//pcl::PointXYZRGBA pt = projected_pts[i][j][0];
		//				pcl::PointXYZRGBA pt = minDistPt;
		//				mat.at<cv::Vec3b>(Point(i, j))[0] = pt.b;
		//				mat.at<cv::Vec3b>(Point(i, j))[1] = pt.g;
		//				mat.at<cv::Vec3b>(Point(i, j))[2] = pt.r;
		//			}
		//		}
		//	}
		//}
		//cout << "count_missed: " << count_missed << endl;

		cv::imshow("dximg", mat);	
		//save the images
		string imgPath;
		if (usingEyediap) {
			imgPath = dir + "eyeImages\\";
			CreateDirectory(imgPath.c_str(), NULL);
		}
		else {
			imgPath = "eyeImages\\";
			CreateDirectory(imgPath.c_str(), NULL);
		}
		cv::imwrite(imgPath + std::to_string(frameNum) + "Img.png", mat);

		for (int i = 0; i < MAT_SZ; i++)
			delete [] projected_pts[i];
		delete [] projected_pts;

		//visualizeCloud(debugCloud);

		//if (!useNoseTip) {
		//
		////#if 1 //eyediap values
		////			double eyeWFactor = 0.07;
		////			double eyeYFactor = -0.045;
		////			double eyesSeparationFactor = 0.01;
		////#else //kinect values
		////			double eyeWFactor = 0.07;
		////			double eyeYFactor = 0.005;
		////			double eyesSeparationFactor = 0.02;
		////#endif
		////			double eyeAspectRatio = 5.0 / 3.0;
		////			double eyeHFactor = eyeWFactor / eyeAspectRatio;
		////			double eyeXFactor = 0; //offset from center
		////			int w = mat.size().width, h = mat.size().height;
		//			//int lx, ly, lw, lh;
		//			//lx = w * ((1 - eyeWFactor * 2 - eyesSeparationFactor) / 2 + eyeXFactor);
		//			//ly = h * ((1 - eyeHFactor) / 2 + eyeYFactor);
		//			//lw = w * eyeWFactor;
		//			//lh = h * eyeHFactor;
		//			//Rect leye(lx, ly, lw, lh);
		//			//int rx, ry, rw, rh;
		//			//rx = w * ((1 + eyesSeparationFactor) / 2 + eyeXFactor);
		//			//ry = h * ((1 - eyeHFactor) / 2 + eyeYFactor);
		//			//rw = w * eyeWFactor;
		//			//rh = h * eyeHFactor;
		//			//Rect reye(rx, ry, rw, rh);
		//			//rectangle(mat, leye, Scalar(255, 0, 0));
		//			//rectangle(mat, reye, Scalar(255, 0, 0));
		//			Mat mat1 = mat.clone();
		//			rectangle(mat1, eyeRects[0], Scalar(255, 0, 0));
		//			rectangle(mat1, eyeRects[1], Scalar(255, 0, 0));
		//			//resize(mat1, mat1, Size(800, 800));
		//			cv::imshow("dximg", mat1);
		//			
		//			Mat leyeImg, reyeImg;
		//			Mat leyeImgInpaint, reyeImgInpaint;
		//			Mat leyeMask, reyeMask;
		//
		//			leyeImg = mat(eyeRects[0]).clone();
		//			leyeMask = mask(eyeRects[0]).clone();
		//			leyeImgInpaint = leyeImg.clone();
		//			//cv::inpaint(leyeImg, leyeMask, leyeImgInpaint, 5, INPAINT_TELEA);
		//
		//			reyeImg = mat(eyeRects[1]);
		//			reyeMask = mask(eyeRects[1]);
		//			reyeImgInpaint = reyeImg.clone();
		//			//cv::inpaint(reyeImg, reyeMask, reyeImgInpaint, 5, INPAINT_TELEA);
		//
		//			////save the images
		//			//string imgPath;
		//			//if (usingEyediap) {
		//			//	imgPath = dir + "eyeImages\\";
		//			//	CreateDirectory(imgPath.c_str(), NULL);
		//			//}
		//			//else {
		//			//	imgPath = "eyeImages\\";
		//			//	CreateDirectory(imgPath.c_str(), NULL);
		//			//}
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "LImg.png", leyeImg);
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "RImg.png", reyeImg);
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "LImgIp.png", leyeImgInpaint);
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "RImgIp.png", reyeImgInpaint);
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "LMsk.png", leyeMask);
		//			//cv::imwrite(imgPath + std::to_string(frameNum) + "RMsk.png", reyeMask);
		//
		//			//display the iamges
		//			cv::imshow("leyeImg", leyeImg);
		//			cv::imshow("reyeImg", reyeImg);
		//			cv::imshow("leyeMask", leyeMask);
		//			cv::imshow("reyeMask", reyeMask);
		//			//resize(leyeImgInpaint, leyeImgInpaint, Size(leyeImgInpaint.size().width * 4, leyeImgInpaint.size().height * 4));
		//			//resize(reyeImgInpaint, reyeImgInpaint, Size(reyeImgInpaint.size().width * 4, reyeImgInpaint.size().height * 4));
		//			//cv::imshow("leyeImg", leyeImgInpaint);
		//			//cv::imshow("reyeImg", reyeImgInpaint);
		//
		//
		//			cvWaitKey(1);
		//}
		//else {
		//	selector.getReg("dximg", mat, eyeRects);
		//}

		return;
	}

	//void Microsoft2Grabber::EyeRegionSelectorMat::CallBackFunc(int event, int x, int y, int flags, void* userdata)
	//{
	//	EyeRegionSelectorMat *self = static_cast<EyeRegionSelectorMat*>(userdata);
	//	self->doCallBack(event, x, y, flags);
	//}

	//void Microsoft2Grabber::EyeRegionSelectorMat::doCallBack(int event, int x, int y, int flags) {
	//	if (event == EVENT_LBUTTONDOWN)
	//	{
	//		if (!inDrag) {
	//			rect.x = x;
	//			rect.y = y;
	//			inDrag = true;
	//		}
	//	}
	//	else if (event == EVENT_MOUSEMOVE)
	//	{
	//		if (inDrag) {
	//			if (x > rect.x && y > rect.y) {
	//				//redraw rectangle
	//				rect.width = x - rect.x;
	//				rect.height = y - rect.y;
	//				Mat img;
	//				//original_img_mutex.lock();
	//				annotated_img.copyTo(img);
	//				//original_img_mutex.unlock();
	//				rectangle(img, rect, Scalar(255, 0, 0), 1);
	//				putText(img, eyeIndx == 0 ? "L" : "R", Point(rect.x + rect.width*0.5, rect.y + rect.height*0.5), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 1);
	//				imshow(windName, img);
	//				cvWaitKey(1);
	//			}
	//		}
	//	}
	//	else if (event == EVENT_LBUTTONUP) {
	//		if (inDrag) {
	//			if (rect.x >= 0 && rect.y >= 0 
	//				&& rect.width > 0 && rect.height > 0
	//				&& rect.x + rect.width <= original_img.size().width 
	//				&& rect.y + rect.height <= original_img.size().height) 
	//			{
	//				pReg[eyeIndx] = Rect(rect.x*resizeFactor, rect.y*resizeFactor, rect.width*resizeFactor, rect.height*resizeFactor);
	//				rectangle(annotated_img, rect, Scalar(255, 0, 0), 1);
	//				putText(annotated_img, eyeIndx == 0 ? "L" : "R", Point(rect.x + rect.width*0.5, rect.y + rect.height*0.5), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 0), 1);
	//				imshow(windName, annotated_img);
	//				cvWaitKey(1);
	//				if (++eyeIndx > 1) {
	//					eyeIndx = 0;
	//					original_img.copyTo(annotated_img);
	//				}
	//			}
	//			inDrag = false;
	//		}
	//	}
	//	else if (event == EVENT_RBUTTONDOWN)
	//	{
	//		if (!inDrag && eyeIndx == 0) {
	//			setMouseCallback(windName, NULL, NULL);
	//			destroyWindow(windName);
	//			regGrabbed = true;
	//		}
	//	}
	//}

	//void Microsoft2Grabber::EyeRegionSelectorMat::getReg(string windName, Mat mat, Rect *pReg) {
	//	inDrag = false;
	//	eyeIndx = 0;
	//	this->windName = windName;
	//	//mat.copyTo(original_img);
	//	resize(mat, original_img, Size(visualImgSz, visualImgSz));
	//	resizeFactor = mat.size().width / (double) visualImgSz;
	//	this->pReg = pReg;

	//	namedWindow(windName, 1);
	//	setMouseCallback(windName, CallBackFunc, this);
	//	original_img.copyTo(annotated_img);
	//	imshow(windName, annotated_img);
	//	cvWaitKey(1);
	//}

	void Microsoft2Grabber::BaseHeadPose::calculate(boost::shared_ptr<pcl::PolygonMesh> mesh, const Calib &m_calib, boost::shared_ptr<Eigen::Affine3f> icp_camera_pose, bool recalcHeadC) {
		if (recalcHeadC) {
			pcl::PointCloud<PointXYZ> cloud;
			pcl::fromPCLPointCloud2(mesh->cloud, cloud);
			//calculate head center as the average of the mesh vertices
			//calculate nose tip as the vertex with closest z coordinate
			Eigen::Vector3f sum = Eigen::Vector3f::Zero();
			Eigen::Vector3f leastZ = Eigen::Vector3f::Zero();
			leastZ.z() = FLT_MAX;
			Eigen::Vector3f mostY = Eigen::Vector3f::Zero();
			//mostY.y() = FLT_MIN;
			mostY.y() = FLT_MAX;
			int cnt = 0;
			PointCloud<PointXYZ>::iterator it = cloud.begin();
			while (it != cloud.end()) {
				PointXYZ p = *it;
				Eigen::Vector3f pp(p.x, p.y, p.z);
				sum += pp;
				if (pp.z() < leastZ.z())
					leastZ = pp;
				//if (pp.y() > mostY.y())
				if (pp.y() < mostY.y())
					mostY = pp;
				++it; cnt++;
			}
			if (leastZ.z() == FLT_MAX 
				||
				//mostY.y() == FLT_MIN
				mostY.y() == FLT_MAX
				)
				return;

			isCalculated = true;
			headCenter_ = sum / double(cnt);
			noseTip_ = leastZ;
			headTop_ = mostY;
		}

		Eigen::Affine3f transf_mesh3 = Eigen::Affine3f::Identity();
		transf_mesh3.linear() << m_calib.R_d(0, 0), m_calib.R_d(0, 1), m_calib.R_d(0, 2),
			m_calib.R_d(1, 0), m_calib.R_d(1, 1), m_calib.R_d(1, 2),
			m_calib.R_d(2, 0), m_calib.R_d(2, 1), m_calib.R_d(2, 2);
		transf_mesh3.translation() << m_calib.T_d(0, 0), m_calib.T_d(1, 0), m_calib.T_d(2, 0);
		Eigen::Affine3f transfICP2World = transf_mesh3 * icp_camera_pose->inverse();

		headCenter = ICP2World(headCenter_, &transfICP2World);
		noseTip = ICP2World(noseTip_, &transfICP2World);
		headTop = ICP2World(headTop_, &transfICP2World);
	}


	//	void Microsoft2Grabber::getNoseTip(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud) {
	//		if (cloud == NULL)
	//			return;
	//
	//		//downsample
	//#if 1
	//		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> dsCloud(new pcl::PointCloud<pcl::PointXYZRGBA>());
	//		pcl::VoxelGrid<pcl::PointXYZRGBA > grid;		
	//		//const float leaf = 0.001f;
	//		const float leaf = 0.01f;
	//		grid.setLeafSize(leaf, leaf, leaf);
	//		m_cloudMutex.lock();
	//		grid.setInputCloud(cloud);
	//		grid.filter(*dsCloud);
	//		m_cloudMutex.unlock();
	//#else
	//		m_cloudMutex.lock();
	//		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> dsCloud(cloud);
	//		m_cloudMutex.unlock();
	//#endif
	//
	//		//get point with closest depth value
	//		PointXYZRGBA closestP;
	//		closestP.z = FLT_MAX;
	//		pcl::PointCloud<pcl::PointXYZRGBA>::iterator pC = dsCloud->begin();
	//		while (pC != dsCloud->end())
	//		{
	//			if ((*pC).z < closestP.z)
	//				closestP = *pC;
	//			pC++;
	//		}
	//
	//		noseTip = Eigen::Vector3f(closestP.x, closestP.y, closestP.z);
	//	}

#pragma endregion
};