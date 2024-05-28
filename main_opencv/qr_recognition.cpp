#include "qr_recognition.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace cv;
using namespace std;

void recognize_qr_code() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error opening video stream or file\n");
        return;
    }

    QRCodeDetector qrDecoder = QRCodeDetector();
    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;

        Mat bbox, rectifiedImage;
        string data = qrDecoder.detectAndDecode(frame, bbox, rectifiedImage);
        if (data.length() > 0) {
            printf("Decoded Data: %s\n", data.c_str());
            if (!rectifiedImage.empty()) {
                imshow("Rectified QRCode", rectifiedImage);
            }
        }

        imshow("Frame", frame);
        if (waitKey(1) == 27)
            break; // Stop if 'ESC' key is pressed
    }
    cap.release();
    destroyAllWindows();
}
