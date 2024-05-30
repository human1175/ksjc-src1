#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "server.h"
#include "client.h"

#define SERVER_IP "127.0.0.1"

using namespace cv;
using namespace std;

int connect_to_server(int port) {
    int sock;
    struct sockaddr_in server_addr;

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to the server on port %d...\n", port);
    return sock;
}

void send_client_action(int sock, ClientAction *action) {
    if (send(sock, action, sizeof(ClientAction), 0) < 0) {
        perror("Send failed");
    } else {
        printf("Sent action: row=%d, col=%d, action=%d\n", action->row, action->col, action->action);
    }
}

void receive_server_response(int sock) {
    DGIST dgist;
    int len = recv(sock, &dgist, sizeof(DGIST), 0);
    if (len > 0) {
        printf("Received map and player information from server.\n");
        // 서버로부터 받은 정보를 출력하거나 처리하는 로직 추가
    } else {
        perror("Receive failed");
    }
}

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
                // imshow("Rectified QRCode", rectifiedImage); // This shows the rectified image if available
                printf("Rectified QRCode\n");
            }
        }

        // Show grayscale frame with detected QR code bounding box
        // imshow("QR Code Detection", gray_frame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
}
