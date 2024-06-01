#include "qr_recognition.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

using namespace cv;
using namespace std;

// QR 코드 인식 함수
void recognize_qr_code(qr_code_callback_t callback) {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error: Could not open camera.\n");
        return;
    }

    // Set camera properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 480);  // Set the width of the frames in the video stream.
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 360); // Set the height of the frames in the video stream.
    cap.set(cv::CAP_PROP_FPS, 60);           // Set the frame rate to 80 FPS.

    QRCodeDetector qrDecoder = QRCodeDetector();
    Mat frame, gray_frame, equalized_frame, bbox, rectifiedImage;
    printf("QR code recognition started...\n");

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            printf("Error: Could not read frame.\n");
            break;
        }

        // Convert to grayscale
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);

        // Apply histogram equalization
        equalizeHist(gray_frame, equalized_frame);

        string data = qrDecoder.detectAndDecode(equalized_frame, bbox, rectifiedImage);
        if (!data.empty()) {
            printf("\n============================== QR decoded Data: %s ==============================\n\n", data.c_str());

            // 콜백 함수 호출하여 디코딩된 데이터를 전달
            if (callback) {
                callback(data.c_str());
            }

            int n = bbox.rows;
            for (int i = 0; i < n; i++) {
                line(frame, Point2i(bbox.at<float>(i, 0), bbox.at<float>(i, 1)),
                     Point2i(bbox.at<float>((i + 1) % n, 0), bbox.at<float>((i + 1) % n, 1)),
                     Scalar(255, 0, 0), 3);
            }
            if (!rectifiedImage.empty()) {
                // imshow("Rectified QRCode", rectifiedImage); // This shows the rectified image if available
                printf("Rectified QRCode\n");
            }
        }

        // Show grayscale frame with detected QR code bounding box
        // imshow("QR Code Detection", equalized_frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}

// QR 코드 인식 스레드 시작 함수
extern "C" void recognize_qr_code_thread(qr_code_callback_t callback) {
    std::thread(recognize_qr_code, callback).detach();
}
