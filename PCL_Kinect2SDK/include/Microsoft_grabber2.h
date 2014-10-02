/*
Copyright (C) 2012 Steven Hickson

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
#pragma once
#include <pcl/pcl_config.h>

#ifndef __PCL_IO_MICROSOFT_GRABBER__
#define __PCL_IO_MICROSOFT_GRABBER__

#include <pcl/io/eigen.h>
#include <pcl/io/boost.h>
#include <pcl/io/grabber.h>
#include <string>
#include <deque>
#include <pcl/common/synchronizer.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/time.h>
#include <pcl/console/print.h>
#include <pcl/exceptions.h>
#include <iostream>

#include <assert.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <objbase.h>
#include "Kinect.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <pcl/visualization/cloud_viewer.h>

using namespace cv;

namespace pcl
{
	struct PointXYZ;
	struct PointXYZRGB;
	struct PointXYZRGBA;
	struct PointXYZI;
	template <typename T> class PointCloud;
	class MatDepth : public cv::Mat { }; //I have to use this to get around the assuming code in registerCallback in grabber.h
	class KinectData {
	public:
		KinectData() { };
		KinectData(const cv::Mat &image_, const MatDepth &depth_, const PointCloud<PointXYZRGBA> &cloud_) : image(image_), depth(depth_), cloud(cloud_) { };

		pcl::PointCloud<pcl::PointXYZRGBA> cloud;
		cv::Mat image;
		MatDepth depth;
	};

	/** \brief Grabber for OpenNI devices (i.e., Primesense PSDK, Microsoft Kinect, Asus XTion Pro/Live)
	* \author Nico Blodow <blodow@cs.tum.edu>, Suat Gedikli <gedikli@willowgarage.com>
	* \ingroup io
	*/
	class PCL_EXPORTS Microsoft2Grabber : public Grabber
	{
	public:
		//dxchange
		bool usingEyediap;
		//CameraSpacePoint m_headpos;
		Eigen::Vector3f m_headpos;
		TrackingState m_headpos_Tracked;
		boost::shared_ptr<PointCloud<PointXYZRGBA>> m_cloud;
		boost::mutex m_cloudMutex;
		void transform(double w, double h, double focalLength, Mat &mat, bool selectEyeRegions = false);
		boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer;
		void visualizeCloud(boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA>> cloud);
		class Calib {
		public:
			Size resolution_d;
			Matx33f intrinsics_d;
			Matx33f R_d;
			Matx31f T_d;
			Matx12f k_coefficients;
			Matx12f alpha;
			Mat beta;
			Size resolution_rgb;
			Matx33f intrinsics_rgb;
			Matx33f R_rgb;
			Matx31f T_rgb;
			Size resolution_rgbHD;
			Matx33f intrinsics_rgbHD;
			Matx33f R_rgbHD;
			Matx31f T_rgbHD;
			void getParams(string dir);
		} m_calib;
		void GetPointCloudFromData2(const boost::shared_ptr<cv::Mat> &img, const MatDepth &depth, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, Calib calib) const;
		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA> > convertToXYZRGBAPointCloud2(const boost::shared_ptr<cv::Mat> &image, const MatDepth &depth_image, const boost::shared_ptr<cv::Mat> &imageHD, Calib calib);
		bool frameGrabbed;
		bool frameProcessed;
		string dir;
		int frameNum;
		//Eigen::Vector3f m_headpos_base; //the head position when the initial model is first grabbed
		//void getNoseTip(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud);
		//Eigen::Vector3f noseTip;
		//Eigen::Vector3f noseTip_head_rot_v; //the head's rotation vector using noseTip method
		//double noseTip_head_rot_a; //the head's rotation angle using noseTip method
		//Rect eyeRects[2]; //[left eye; right eye]
		//class EyeRegionSelectorMat {
		//public:
		//	EyeRegionSelectorMat() {};
		//	void getReg(string windName, Mat mat, Rect *pReg);
		//	static void CallBackFunc(int event, int x, int y, int flags, void* userdata);
		//	void doCallBack(int event, int x, int y, int flags);
		//	bool regGrabbed;
		//private:
		//	Mat original_img;
		//	Mat annotated_img;
		//	string windName;
		//	Rect *pReg;
		//	bool inDrag;
		//	Rect rect;
		//	int eyeIndx; //0 for left eye, 1 for right eye
		//	static const int visualImgSz = 800;
		//	double resizeFactor;
		//} selector;
		void GetPointCloudFromDataHD(const MatDepth &depth, const boost::shared_ptr<cv::Mat> &imgHD, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, Calib calib);
		boost::shared_ptr<pcl::PolygonMesh> getOrientedMesh();
		void getOrientedMesh1(); //avoids copying the vertices. much faster
		boost::shared_ptr<pcl::PolygonMesh> mesh_ptr_;
		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZ>> mesh_cld_ptr_;
		bool icp_success;
		bool icp_success_last_frame;
		boost::shared_ptr<Eigen::Affine3f> icp_camera_pose;
		inline bool isInsideTriangle(cv::Point2f pt, cv::Point2f a, cv::Point2f b, cv::Point2f c) {
			Point2f v = pt;
			Point2f v0 = a;
			Point2f v1 = b - a;
			Point2f v2 = c - a;
			double value1 = (det2d(v, v2) - det2d(v0, v2)) / det2d(v1, v2);
			double value2 = -(det2d(v, v1) - det2d(v0, v1)) / det2d(v1, v2);
			return (value1 >= 0) && (value2 >= 0) && (value1 + value2 <= 1);
		};
		double det2d(Point2f u, Point2f v) { return u.x*v.y - u.y*v.x; };
		bool depth_first_search(bool first, int x, int y, Point2f p1, Point2f p2, Point2f p3, double dist, Vertices face, Vertices **s_best, double **d_best, bool** visited, int width, int height);
		//depth_first_search1 is same as depth_first_search except it doesn't pass and keep track the face vector.  it gets the job done faster
		bool depth_first_search1(bool first, int x, int y, Point2f p1, Point2f p2, Point2f p3, double dist, double **d_best, bool** visited, int width, int height);
		double ** d_best;
		Vertices **s_best;
		bool **visited;
		int arrayW, arrayH; //w and h of d_best, s_best, and visited.
		inline double faceNormalDotCentroid(Eigen::Vector3f ppp0, Eigen::Vector3f ppp1, Eigen::Vector3f ppp2, bool normalize) {
			//find face normal
			Eigen::Vector3f normV = (ppp1 - ppp0).cross(ppp2 - ppp0);
			//find face centroid
			Eigen::Vector3f centV = (ppp0 + ppp1 + ppp2) / 3.0;
			//Eigen::Vector3f centV = Vector3fMap::UnitZ();
			//dot product of centroid and normal
			double dotP = normV.dot(centV);
			//normalize
			if (normalize)
				dotP = dotP / normV.norm() / centV.norm();
			return dotP;
		};
		class BaseHeadPose {
		public:
			BaseHeadPose() : isCalculated(false) {};
			void calculate(boost::shared_ptr<pcl::PolygonMesh> mesh, const Calib &m_calib, boost::shared_ptr<Eigen::Affine3f> icp_camera_pose, bool recalcHeadC);
			inline Eigen::Vector3f Microsoft2Grabber::BaseHeadPose::ICP2World(Eigen::Vector3f v, Eigen::Affine3f *transfICP2World) {
				return transfICP2World->linear() * v + transfICP2World->translation();
			};
			Eigen::Vector3f headCenter;
			Eigen::Vector3f noseTip;
			Eigen::Vector3f headTop;
			bool isCalculated;
		private:
			//these are the caluculated headCenter and noseTip in the depth camera's frame
			//we save them so not have the need to recalculate
			Eigen::Vector3f headCenter_;
			Eigen::Vector3f noseTip_;
			Eigen::Vector3f headTop_;
		} baseHeadPose;
		Eigen::Matrix3f alignVectors(Eigen::Vector3f v1, Eigen::Vector3f v2);
		bool trackingOnly;


		typedef boost::shared_ptr<Microsoft2Grabber> Ptr;
		typedef boost::shared_ptr<const Microsoft2Grabber> ConstPtr;

		typedef enum
		{
			Microsoft_Default_Mode = 0, // VGA@30Hz
			Microsoft_SXGA_15Hz = 1    // Need to fill the rest of this up
		} Mode;

		//define callback signature typedefs
		typedef void (sig_cb_microsoft_image)(const boost::shared_ptr<cv::Mat> &);
		typedef void (sig_cb_microsoft_depth_image)(const MatDepth &);
		typedef void (sig_cb_microsoft_image_depth_image)(const boost::shared_ptr<cv::Mat> &, const MatDepth &, float);
		typedef void (sig_cb_microsoft_point_cloud_rgba)(const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZRGBA> >&);
		typedef void (sig_cb_microsoft_all_data)(const boost::shared_ptr<const KinectData> &);
		/*typedef void (sig_cb_microsoft_ir_image) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZI> >&);
		typedef void (sig_cb_microsoft_point_cloud) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZ> >&);
		typedef void (sig_cb_microsoft_point_cloud_rgb) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZRGB> >&);
		typedef void (sig_cb_microsoft_point_cloud_i) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZI> >&);*/

		Microsoft2Grabber(const int instance = 0, bool usingEyediap = false, bool trackingOnly = false);
		//const Mode& depth_mode = OpenNI_Default_Mode,
		//const Mode& image_mode = OpenNI_Default_Mode);

		/** \brief virtual Destructor inherited from the Grabber interface. It never throws. */
		virtual ~Microsoft2Grabber() throw ();

		/** \brief Start the data acquisition. */
		virtual void
			start();

		/** \brief Stop the data acquisition. */
		virtual void
			stop();

		/** \brief Check if the data acquisition is still running. */
		virtual bool
			isRunning() const;

		virtual std::string
			getName() const;

		/** \brief Obtain the number of frames per second (FPS). */
		virtual float
			getFramesPerSecond() const;

		/** \brief Get a boost shared pointer to the \ref OpenNIDevice object. */
		/*inline boost::shared_ptr<Microsoft2Grabber>
		getDevice () const;*/


		//Kinect Camera Settings
		ICoordinateMapper*      m_pCoordinateMapper;
		bool CameraSettingsSupported;

		void GetPointCloudFromData(const boost::shared_ptr<cv::Mat> &img, const MatDepth &depth, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, bool alignToColor, bool preregistered) const;

		//These should not be called except within the thread by the KinectCapture class process manager
		void ProcessThreadInternal();

		void SetLargeCloud() {
			m_largeCloud = true;
		}

		void SetNormalCloud() {
			m_largeCloud = false;
		}

	protected:
		boost::signals2::signal<sig_cb_microsoft_image>* image_signal_;
		boost::signals2::signal<sig_cb_microsoft_depth_image>* depth_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_image_depth_image>* image_depth_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_rgba>* point_cloud_rgba_signal_;
		boost::signals2::signal<sig_cb_microsoft_all_data>* all_data_signal_;
		/*boost::signals2::signal<sig_cb_microsoft_ir_image>* ir_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud>* point_cloud_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_i>* point_cloud_i_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_rgb>* point_cloud_rgb_signal_;
		*/
		Synchronizer<boost::shared_ptr<cv::Mat>, MatDepth > rgb_sync_;

		bool m_depthStarted, m_videoStarted, m_audioStarted, m_infraredStarted, m_person, m_preregistered;
		// Current Kinect
		IKinectSensor*          m_pKinectSensor;

		//IColorFrameReader*      m_pColorFrameReader;
		IMultiSourceFrameReader*m_pMultiSourceFrameReader;

		static const int        cColorWidth = 1920;
		static const int        cColorHeight = 1080;
		static const int        cDepthWidth = 512;
		static const int        cDepthHeight = 424;

		//eyediap dataset resolution values
		static const int        cColorWidth1 = 640;
		static const int        cColorHeight1 = 480;
		static const int        cDepthWidth1 = 640;
		static const int        cDepthHeight1 = 480;

		cv::Size m_colorSize, m_depthSize;
		RGBQUAD* m_pColorRGBX;
		UINT16 *m_pDepthBuffer;
		UINT nDepthBufferSize;
		ColorSpacePoint *m_pColorCoordinates;
		CameraSpacePoint *m_pCameraSpacePoints;
		cv::Mat m_colorImage, m_depthImage;
#define COLOR_PIXEL_TYPE CV_8UC4
#define DEPTH_PIXEL_TYPE CV_16UC1

		bool m_largeCloud;
		HANDLE hStopEvent, hKinectThread, hDepthMutex, hColorMutex;
		WAITABLE_HANDLE hFrameEvent;
		bool m_depthUpdated, m_colorUpdated, m_infraredUpdated, m_skeletonUpdated;
		LONGLONG m_rgbTime, m_depthTime, m_infraredTime;
		INT64 timestep;
		//boost::mutex m_depthMutex, m_colorMutex, m_infraredMutex;

		void Release();
		void GetNextFrame();
		void FrameArrived(IMultiSourceFrameArrivedEventArgs *pArgs);
		void DepthFrameArrived(IDepthFrameReference* pDepthFrameReference);
		void ColorFrameArrived(IColorFrameReference* pColorFrameReference);
		void BodyIndexFrameArrived(IBodyIndexFrameReference* pBodyIndexFrameReference);

		//dxchange
		void BodyFrameArrived(IBodyFrameReference* pBodyFrameReference);
		void ProcessBody(int nBodyCount, IBody** ppBodies);

		bool GetCameraSettings();

		void imageDepthImageCallback(const boost::shared_ptr<cv::Mat> &image, const MatDepth &depth_image);
		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA> > convertToXYZRGBAPointCloud(const boost::shared_ptr<cv::Mat> &image,
			const MatDepth &depth_image) const;
		/** \brief Convert a Depth + RGB image pair to a pcl::PointCloud<PointT>
		* \param[in] image the RGB image to convert
		* \param[in] depth_image the depth image to convert
		*/
		/*template <typename PointT> typename pcl::PointCloud<PointT>::Ptr
		convertToXYZRGBPointCloud (const boost::shared_ptr<openni_wrapper::Image> &image,
		const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const;*/
	};

}

#endif //__PCL_IO_MICROSOFT_GRABBER__