//------------------------------------------------------------------------------
// <copyright file="ImageRenderer.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <string>
#include "ImageRenderer.h"

using namespace DirectX;

static const float c_FaceBoxThickness = 6.0f;
static const float c_FacePointThickness = 10.0f;
static const float c_FacePointRadius = 1.0f;
static const float c_FacePropertyFontSize = 32.0f;
static const double c_FaceRotationIncrementInDegrees = 5.0f;

static const float c_TextLayoutWidth = 800;
static const float c_TextLayoutHeight = 600;

/// <summary>
/// Constructor
/// </summary>
ImageRenderer::ImageRenderer() : 
    m_hWnd(0),
    m_sourceWidth(0),
    m_sourceHeight(0),
    m_sourceStride(0),
    m_pD2DFactory(nullptr), 
    m_pRenderTarget(nullptr),
    m_pBitmap(0),	
	m_pTextBrush(nullptr),
    m_pTextFormat(0),
    m_pDWriteFactory(nullptr)
{
    for (int i = 0; i < BODY_COUNT; i++)
    {
        m_pFaceBrush[i] = nullptr;
		m_pTextBGBrush[i] = nullptr;
    }
}

/// <summary>
/// Destructor
/// </summary>
ImageRenderer::~ImageRenderer()
{
    DiscardResources();
    SafeRelease(m_pTextFormat);
    SafeRelease(m_pDWriteFactory);
    SafeRelease(m_pD2DFactory);
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT ImageRenderer::EnsureResources()
{
    HRESULT hr = S_OK;

    if (nullptr == m_pRenderTarget)
    {
        D2D1_SIZE_U size = D2D1::SizeU(m_sourceWidth, m_sourceHeight);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        // Create a hWnd render target, in order to render to the window set in initialize
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(m_hWnd, size),
            &m_pRenderTarget
            );

        if (SUCCEEDED(hr))
        {
            // Create a bitmap that we can copy image data into and then render to the target
            hr = m_pRenderTarget->CreateBitmap(
                size, 
                D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
                &m_pBitmap 
                );
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Red, 2.0f)), &m_pFaceBrush[0]);
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Red, 0.7f)), &m_pTextBGBrush[0]));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Green, 2.0f)), &m_pFaceBrush[1]);	
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Green, 0.7f)), &m_pTextBGBrush[1]));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::White, 2.0f)), &m_pFaceBrush[2]);
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::White, 0.7f)), &m_pTextBGBrush[2]));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Purple, 2.0f)), &m_pFaceBrush[3]);
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Purple, 0.7f)), &m_pTextBGBrush[3]));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Orange, 2.0f)), &m_pFaceBrush[4]);
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Orange, 0.7f)), &m_pTextBGBrush[4]));
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Pink, 2.0f)), &m_pFaceBrush[5]);	
			HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Pink, 0.7f)), &m_pTextBGBrush[5]));
        }

		
		HR(m_pRenderTarget->CreateSolidColorBrush((D2D1::ColorF(D2D1::ColorF::Black, 1.0f)), &m_pTextBrush));
    }

    if (FAILED(hr))
    {
        DiscardResources();
    }

    return hr;
}

/// <summary>
/// Dispose of Direct2d resources 
/// </summary>
void ImageRenderer::DiscardResources()
{
    for (int i = 0; i < BODY_COUNT; i++)
    {
        SafeRelease(m_pFaceBrush[i]);
		SafeRelease(m_pTextBGBrush[i]);
    }
	
	SafeRelease(m_pTextBrush);
    SafeRelease(m_pRenderTarget);
    SafeRelease(m_pBitmap);
}

/// <summary>
/// Set the window to draw to as well as the video format
/// Implied bits per pixel is 32
/// </summary>
/// <param name="hWnd">window to draw to</param>
/// <param name="pD2DFactory">already created D2D factory object</param>
/// <param name="sourceWidth">width (in pixels) of image data to be drawn</param>
/// <param name="sourceHeight">height (in pixels) of image data to be drawn</param>
/// <param name="sourceStride">length (in bytes) of a single scanline</param>
/// <returns>indicates success or failure</returns>
HRESULT ImageRenderer::Initialize(HWND hWnd, ID2D1Factory* pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride)
{
    if (nullptr == pD2DFactory)
    {
        return E_INVALIDARG;
    }

    m_hWnd = hWnd;

    // One factory for the entire application so save a pointer here
    m_pD2DFactory = pD2DFactory;

    m_pD2DFactory->AddRef();

    // Get the frame size
    m_sourceWidth  = sourceWidth;
    m_sourceHeight = sourceHeight;
    m_sourceStride = sourceStride;

    HRESULT hr;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

    if (SUCCEEDED(hr))
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Georgia",           
            NULL,               
            DWRITE_FONT_WEIGHT_ULTRA_BLACK,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            c_FacePropertyFontSize,
            L"en-us",
            &m_pTextFormat
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
	
	HR(m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WHOLE_WORD));
    
	return hr;
}

/// <summary>
/// Prepare device to begin drawing
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT ImageRenderer::BeginDrawing()
{
    // create the resources for this draw device
    // they will be recreated if previously lost
    HRESULT hr = EnsureResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
    }

    return hr;
}

/// <summary>
/// Ends drawing
/// </summary>    
/// <returns>indicates success or failure</returns>
HRESULT ImageRenderer::EndDrawing()
{
    HRESULT hr;
    hr = m_pRenderTarget->EndDraw();

    // Device lost, need to recreate the render target
    // We'll dispose it now and retry drawing
    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardResources();
    }

    return hr;
}

/// <summary>
/// Draws a 32 bit per pixel image of previously specified width, height, and stride to the associated hwnd
/// </summary>
/// <param name="pImage">image data in RGBX format</param>
/// <param name="cbImage">size of image data in bytes</param>
/// <returns>indicates success or failure</returns>
HRESULT ImageRenderer::DrawBackground(BYTE* pImage, unsigned long cbImage)
{
    HRESULT hr = S_OK;

    // incorrectly sized image data passed in
    if (cbImage < ((m_sourceHeight - 1) * m_sourceStride) + (m_sourceWidth * 4))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        // Copy the image that was passed in into the direct2d bitmap
        hr = m_pBitmap->CopyFromMemory(NULL, pImage, m_sourceStride);

        if (SUCCEEDED(hr))
        {
            // Draw the bitmap stretched to the size of the window
            m_pRenderTarget->DrawBitmap(m_pBitmap);	
        }
    }

    return hr;
}

/// <summary>
/// Draws face frame results
/// </summary>
/// <param name="iFace">the index of the face frame corresponding to a specific body in the FOV</param>
/// <param name="pFaceBox">face bounding box</param>
/// <param name="pFacePoints">face points</param>
/// <param name="pFaceRotation">face rotation</param>
/// <param name="pFaceProperties">face properties</param>
void ImageRenderer::DrawFaceFrameResults(int iFace, const RectI* pFaceBox, const PointF* pFacePoints, const Vector4* pFaceRotation, const DetectionResult* pFaceProperties, const CameraSpacePoint* pHeadPivot, const float* pAnimUnits)
{
    // draw the face frame results only if the face bounding box is valid
    if (ValidateFaceBoxAndPoints(pFaceBox, pFacePoints))
    {
        ID2D1SolidColorBrush* brush = m_pFaceBrush[iFace];

        // draw the face bounding box
        D2D1_RECT_F faceBox = D2D1::RectF(static_cast<FLOAT>(pFaceBox->Left), 
            static_cast<FLOAT>(pFaceBox->Top),
            static_cast<FLOAT>(pFaceBox->Right),
            static_cast<FLOAT>(pFaceBox->Bottom));
        m_pRenderTarget->DrawRectangle(faceBox, brush, c_FaceBoxThickness);

        //// draw each face point
        //for (int i = 0; i < FacePointType::FacePointType_Count; i++)
        //{
        //    D2D1_ELLIPSE facePoint= D2D1::Ellipse(D2D1::Point2F(pFacePoints[i].X, pFacePoints[i].Y), c_FacePointRadius, c_FacePointRadius);
        //    m_pRenderTarget->DrawEllipse(facePoint, brush, c_FacePointThickness);
        //}

        std::wstring faceText = L"";

        // extract each face property information and store it is faceText
        /*for (int iProperty = 0; iProperty < FaceProperty::FaceProperty_Count; iProperty++)
        {
            switch (iProperty)
            {
            case FaceProperty::FaceProperty_Happy:
                faceText += L"Happy :";
                break;
            case FaceProperty::FaceProperty_Engaged:
                faceText += L"Engaged :";
                break;
            case FaceProperty::FaceProperty_LeftEyeClosed:
                faceText += L"LeftEyeClosed :";
                break;
            case FaceProperty::FaceProperty_RightEyeClosed:
                faceText += L"RightEyeClosed :";
                break;
            case FaceProperty::FaceProperty_LookingAway:
                faceText += L"LookingAway :";
                break;
            case FaceProperty::FaceProperty_MouthMoved:
                faceText += L"MouthMoved :";
                break;
            case FaceProperty::FaceProperty_MouthOpen: 
                faceText += L"MouthOpen :";
                break;
            case FaceProperty::FaceProperty_WearingGlasses:
                faceText += L"WearingGlasses :";
                break;
            default:
                break;
            }
*/
            /*switch (pFaceProperties[iProperty]) 
            {
            case DetectionResult::DetectionResult_Unknown:
                faceText += L" UnKnown";
                break;
            case DetectionResult::DetectionResult_Yes:
                faceText += L" Yes";
                break;
            case DetectionResult::DetectionResult_No:
                faceText += L" No";
                break;
            case DetectionResult::DetectionResult_Maybe:
                faceText += L" Maybe";
                break;
            default:
                break;
            }
*/
            //faceText += L"\n";
        //}
		
		faceText += L"HeadPivot Coordinates\n";
		faceText += L" X-> " + std::to_wstring(pHeadPivot->X) + L" Y-> " + std::to_wstring(pHeadPivot->Y) + L" Z-> " + std::to_wstring(pHeadPivot->Z) + L" \n";

		//get the HDFace animation units
		for (int i = 0; i < FaceShapeAnimations_Count; i++)
		{
			FaceShapeAnimations faceAnim = (FaceShapeAnimations)i;
			std::wstring strValue = std::to_wstring(pAnimUnits[faceAnim]);

			//if (pAnimUnits[faceAnim] != 0.0)
			{
				switch (faceAnim)
				{
				case FaceShapeAnimations::FaceShapeAnimations_JawOpen:
					faceText += L"JawOpened: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_JawSlideRight:
					faceText += L"JawSlideRight: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LeftcheekPuff:
					faceText += L"LeftCheekPuff: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LefteyebrowLowerer:
					faceText += L"LeftEyeBrowLowered: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LefteyeClosed:
					faceText += L"LeftEyeClosed: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_RighteyebrowLowerer:
					faceText += L"RightEyeBrowLowered: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_RighteyeClosed:
					faceText += L"RightEyeClosed: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipCornerDepressorLeft:
					faceText += L"LipCornerDepressedLeft: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipCornerDepressorRight:
					faceText += L"LipCornerDepressedRight: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipCornerPullerLeft:
					faceText += L"LipCornerPulledLeft: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipCornerPullerRight:
					faceText += L"LipCornerPulledRight: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipPucker:
					faceText += L"Lips Puckered: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipStretcherLeft:
					faceText += L"LipStretchLeft: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LipStretcherRight:
					faceText += L"LipStretchRight: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LowerlipDepressorLeft:
					faceText += L"LowerLipDepressedLeft: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_LowerlipDepressorRight:
					faceText += L"LowerLipDepressedRight: " + strValue + L"\n";
					break;
				case FaceShapeAnimations::FaceShapeAnimations_RightcheekPuff:
					faceText += L"RightCheekPuffed: " + strValue + L"\n";
					break;
				
				}
			}
			
		}
        // extract face rotation in degrees as Euler angles
        int pitch, yaw, roll;
        ExtractFaceRotationInDegrees(pFaceRotation, &pitch, &yaw, &roll);

        faceText += L"FaceYaw : " + std::to_wstring(yaw) + L"\n";
        faceText += L"FacePitch : " + std::to_wstring(pitch) + L"\n";
        faceText += L"FaceRoll : " + std::to_wstring(roll) + L"\n";

		D2D1_RECT_F layoutBorderRect = D2D1::RectF(faceBox.right + 30 , faceBox.top - 230,
			faceBox.right + c_TextLayoutWidth + 30,
			faceBox.bottom + c_TextLayoutHeight + 30);

        D2D1_RECT_F layoutRect = D2D1::RectF(faceBox.right+ 50, faceBox.top - 200, 
            faceBox.right + c_TextLayoutWidth, 
            faceBox.bottom + c_TextLayoutHeight);    

        // render the face property and face rotation information
		m_pRenderTarget->FillRectangle(layoutBorderRect, m_pTextBGBrush[iFace]);

        m_pRenderTarget->DrawTextW(faceText.c_str(), 
            static_cast<UINT32>(faceText.length()), 
            m_pTextFormat, 
            layoutRect, 
            m_pTextBrush);
    }
}

/// <summary>
/// Validates face bounding box and face points to be within screen space
/// </summary>
/// <param name="pFaceBox">the face bounding box</param>
/// <param name="pFacePoints">the face points</param>
/// <returns>success or failure</returns>
bool ImageRenderer::ValidateFaceBoxAndPoints(const RectI* pFaceBox, const PointF* pFacePoints)
{
    bool isFaceValid = false;

    if (pFaceBox != nullptr)
    {
        INT32 screenWidth = m_sourceWidth;
        INT32 screenHeight = m_sourceHeight;

        INT32 width = pFaceBox->Right - pFaceBox->Left;
        INT32 height = pFaceBox->Bottom - pFaceBox->Top;

        // check if we have a valid rectangle within the bounds of the screen space
        isFaceValid = width > 0 && 
            height > 0 && 
            pFaceBox->Right <= screenWidth && 
            pFaceBox->Bottom <= screenHeight;

        //if (isFaceValid)
        //{
        //    for (int i = 0; i < FacePointType::FacePointType_Count; i++)
        //    {
        //        // check if we have a valid face point within the bounds of the screen space                        
        //        bool isFacePointValid = pFacePoints[i].X > 0.0f &&
        //            pFacePoints[i].Y > 0.0f &&
        //            pFacePoints[i].X < m_sourceWidth &&
        //            pFacePoints[i].Y < m_sourceHeight;

        //        if (!isFacePointValid)
        //        {
        //            isFaceValid = false;
        //            break;
        //        }
        //    }
        //}
    }

    return isFaceValid;
}


/// <summary>
/// Converts rotation quaternion to Euler angles 
/// And then maps them to a specified range of values to control the refresh rate
/// </summary>
/// <param name="pQuaternion">face rotation quaternion</param>
/// <param name="pPitch">rotation about the X-axis</param>
/// <param name="pYaw">rotation about the Y-axis</param>
/// <param name="pRoll">rotation about the Z-axis</param>
void ImageRenderer::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
{
    double x = pQuaternion->x;
    double y = pQuaternion->y;
    double z = pQuaternion->z;
    double w = pQuaternion->w;

    // convert face rotation quaternion to Euler angles in degrees		
    double dPitch, dYaw, dRoll;
    dPitch = atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / M_PI * 180.0;
    dYaw = asin(2 * (w * y - x * z)) / M_PI * 180.0;
    dRoll = atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / M_PI * 180.0;

    // clamp rotation values in degrees to a specified range of values to control the refresh rate
    double increment = c_FaceRotationIncrementInDegrees;
    *pPitch = static_cast<int>((dPitch + increment/2.0 * (dPitch > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
    *pYaw = static_cast<int>((dYaw + increment/2.0 * (dYaw > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
    *pRoll = static_cast<int>((dRoll + increment/2.0 * (dRoll > 0 ? 1.0 : -1.0)) / increment) * static_cast<int>(increment);
}