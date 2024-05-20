#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Initialize camera
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Unable to open the camera." << endl;
        return -1;
    }

    QRCodeDetector qrDecoder;

    while (true) {
        Mat frame;
        cap >> frame; // Capture frame
        if (frame.empty()) {
            cerr << "Unable to read frame." << endl;
            break;
        }

        // Detect and decode QR code
        string data;
        vector<Point> points;
        Mat straight_qrcode;
        
        data = qrDecoder.detectAndDecode(frame, points, straight_qrcode);
        
        if (!data.empty()) {
            // Draw bounding box around the QR code
            if (points.size() == 4) {
                for (int i = 0; i < 4; i++) {
                    line(frame, points[i], points[(i + 1) % 4], Scalar(0, 255, 0), 3);
                }
            }

            // Output QR code data
            cout << "QR Code Data: " << data << endl;
            putText(frame, data, points[0], FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
        }

        // Display frame
        imshow("QR Code Detection", frame);

        // Exit on 'q' key press
        if (waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}