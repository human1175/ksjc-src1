#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <iostream>

using namespace cv;
using namespace std;
using namespace zbar;

int main() {
    // Initialize camera
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Unable to open the camera." << endl;
        return -1;
    }

    // Initialize zbar scanner
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    while (true) {
        Mat frame;
        cap >> frame; // Capture frame
        if (frame.empty()) {
            cerr << "Unable to read frame." << endl;
            break;
        }

        // Convert frame to grayscale
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Wrap image data for zbar
        int width = gray.cols;
        int height = gray.rows;
        uchar *raw = (uchar *)gray.data;
        Image image(width, height, "Y800", raw, width * height);

        // Scan for QR codes
        int n = scanner.scan(image);
        for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
            vector<Point> vp;
            // Draw bounding box around QR code
            for (int i = 0; i < symbol->get_location_size(); i++) {
                vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
            }
            RotatedRect r = minAreaRect(vp);
            Point2f pts[4];
            r.points(pts);
            for (int i = 0; i < 4; i++) {
                line(frame, pts[i], pts[(i + 1) % 4], Scalar(0, 255, 0), 3);
            }

            // Output QR code data
            string qr_data = symbol->get_data();
            cout << "QR code data: " << qr_data << endl;
            putText(frame, qr_data, pts[1], FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
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