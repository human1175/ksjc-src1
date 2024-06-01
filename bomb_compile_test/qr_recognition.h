#ifndef QR_RECOGNITION_H
#define QR_RECOGNITION_H

#ifdef __cplusplus
extern "C" {
#endif

// QR 코드 인식 콜백 함수 타입 정의
typedef void (*qr_code_callback_t)(const char*);

// QR 코드 인식 스레드 시작 함수
void recognize_qr_code_thread(qr_code_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // QR_RECOGNITION_H
