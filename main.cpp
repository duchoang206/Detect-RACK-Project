#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main() {
    std::cout << "=== RTSP Camera Real-time Stream ===" << std::endl;

    // URL-encode '@' in password 'rtc@2025' to 'rtc%402025'
    std::string rtsp_url = "rtsp://admin:rtc%402025@192.168.5.201:554/cam/realmonitor?channel=1&subtype=0";

    std::cout << "Connecting to camera stream..." << std::endl;
    std::cout << "RTSP URL: " << rtsp_url << std::endl;

    // Open RTSP stream using FFmpeg backend
    cv::VideoCapture cap(rtsp_url, cv::CAP_FFMPEG);

    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to connect to RTSP stream!" << std::endl;
        std::cerr << "Please verify:" << std::endl;
        std::cerr << "  1. The camera IP (192.168.5.201) is reachable (ping 192.168.5.201)." << std::endl;
        std::cerr << "  2. The login credentials (admin / rtc@2025) are correct." << std::endl;
        std::cerr << "  3. The camera's RTSP port (554) is open." << std::endl;
        return -1;
    }

    std::cout << "\nConnection established successfully!" << std::endl;
    std::cout << "Press 'ESC' in the window to stop the stream." << std::endl;

    // Optional: Print properties of the stream
    double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "Stream Info: Resolution=" << width << "x" << height << ", Target FPS=" << fps << std::endl;

    // Create a named window that can be resized
    std::string window_name = "RTSP Real-time Stream";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL);

    cv::Mat frame;
    while (true) {
        // Grab and retrieve frame
        if (!cap.read(frame)) {
            std::cerr << "Warning: Failed to grab frame, camera might be offline or reconnecting..." << std::endl;
            // Sleep for a short moment before trying again to avoid high CPU load
            cv::waitKey(100);
            continue;
        }

        // Display frame
        cv::imshow(window_name, frame);

        // Press ESC (ASCII 27) to exit
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // Clean up
    cap.release();
    cv::destroyAllWindows();
    std::cout << "Stream stopped and resources released." << std::endl;
    return 0;
}
