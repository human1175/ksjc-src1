#include "qr_recognition.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <pthread.h>
#include "client.h"
#include "server.h"

using namespace cv;
using namespace std;

void* recognize_qr_code(void* arg) {
    VideoCapture cap(0);  // Open the default camera
    if (!cap.isOpened()) {
        printf("Error: Could not open camera.\n");
        return NULL;
    }

    // Set camera properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 160);  // Set the width of the frames in the video stream to 160.
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 120); // Set the height of the frames in the video stream to 120.
    cap.set(cv::CAP_PROP_FPS, 80);           // Set the frame rate to 80 FPS.

    QRCodeDetector qrDecoder = QRCodeDetector();
    Mat frame, bbox, rectifiedImage;
    printf("QR code recognition started...\n");

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            printf("Error: Could not read frame.\n");
            break;
        }

        string data = qrDecoder.detectAndDecode(frame, bbox, rectifiedImage);
        if (!data.empty()) {
            printf("\n============================== QR decoded Data: %s ==============================\n\n", data.c_str());
            send_qr_result_to_server(data.c_str());

            int n = bbox.rows;
            for (int i = 0; i < n; i++) {
                line(frame, Point2i(bbox.at<float>(i, 0), bbox.at<float>(i, 1)),
                     Point2i(bbox.at<float>((i + 1) % n, 0), bbox.at<float>((i + 1) % n, 1)),
                     Scalar(255, 0, 0), 3);
            }
            if (!rectifiedImage.empty()) {
                imshow("Rectified QRCode", rectifiedImage);
            }
        }

        imshow("QR Code Detection", frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return NULL;
}

extern "C" void recognize_qr_code_thread() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, recognize_qr_code, NULL);
    pthread_detach(thread_id);
}
