#include "qr_recognition.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <thread>
#include "server.h"

// 외부 함수 선언
extern int connect_to_server(int port);
extern void send_client_action(int sock, ClientAction *action);
extern void receive_server_response(int sock);

using namespace cv;
using namespace std;

void recognize_qr_code(int sock) {
    VideoCapture cap(0);  // Open the default camera
    if (!cap.isOpened()) {
        printf("Error: Could not open camera.\n");
        return;
    }

    // Set camera properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 160);  // Set the width of the frames in the video stream.
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 120); // Set the height of the frames in the video stream.
    cap.set(cv::CAP_PROP_FPS, 80);           // Set the frame rate to 80 FPS.

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

            // QR 코드 데이터를 파싱하여 row와 col 값으로 변환
            ClientAction action;
            sscanf(data.c_str(), "%1d%1d", &action.row, &action.col);
            action.action = move; // 기본 동작 설정

            send_client_action(sock, &action);

            int n = bbox.rows;
            for (int i = 0; i < n; i++) {
                line(frame, Point2i(bbox.at<float>(i, 0), bbox.at<float>(i, 1)),
                     Point2i(bbox.at<float>((i + 1) % n, 0), bbox.at<float>((i + 1) % n, 1)),
                     Scalar(255, 0, 0), 3);
            }
            if (!rectifiedImage.empty()) {
                imshow("Rectified QRCode", rectifiedImage);
                printf("Rectified QRCode\n");
            }
        }

        imshow("QR Code Detection", gray_frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}
