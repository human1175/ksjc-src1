#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

int main() {
    cv::VideoCapture cap(0);  // Open the default camera
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    cv::QRCodeDetector qrDecoder = cv::QRCodeDetector();
    cv::Mat frame, bbox, rectifiedImage;

    std::cout << "QR code recognition started..." << std::endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        std::string data = qrDecoder.detectAndDecode(frame, bbox, rectifiedImage);
        if (!data.empty()) {
            std::cout << "Decoded Data: " << data << std::endl;

            int n = bbox.rows;
            for (int i = 0; i < n; i++) {
                cv::line(frame, cv::Point2i(bbox.at<float>(i, 0), bbox.at<float>(i, 1)),
                         cv::Point2i(bbox.at<float>((i + 1) % n, 0), bbox.at<float>((i + 1) % n, 1)),
                         cv::Scalar(255, 0, 0), 3);
            }
            if (!rectifiedImage.empty()) {
                cv::imshow("Rectified QRCode", rectifiedImage);
            }
        }

        cv::imshow("QR Code Detection", frame);
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
