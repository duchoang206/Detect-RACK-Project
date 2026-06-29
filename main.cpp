#include <iostream>
#include <opencv2/opencv.hpp>

// Global variables for mouse drawing state
cv::Rect g_drawn_box;
bool g_is_drawing = false;
bool g_has_box = false;

// We will pass the current frame size to the mouse callback
cv::Size g_frame_size(1, 1);

// Mouse callback function to let user draw a bounding box (Left Click) 
// and print coordinates for calibration (Right Click)
void onMouse(int event, int x, int y, int flags, void* userdata) {
    int width = g_frame_size.width;
    int height = g_frame_size.height;

    if (event == cv::EVENT_LBUTTONDOWN) {
        g_is_drawing = true;
        g_drawn_box = cv::Rect(x, y, 0, 0);
        g_has_box = false;
    } else if (event == cv::EVENT_MOUSEMOVE) {
        if (g_is_drawing) {
            g_drawn_box.width = x - g_drawn_box.x;
            g_drawn_box.height = y - g_drawn_box.y;
        }
    } else if (event == cv::EVENT_LBUTTONUP) {
        g_is_drawing = false;
        
        // Correct negative width/height if drawn backwards
        if (g_drawn_box.width < 0) {
            g_drawn_box.x += g_drawn_box.width;
            g_drawn_box.width = -g_drawn_box.width;
        }
        if (g_drawn_box.height < 0) {
            g_drawn_box.y += g_drawn_box.height;
            g_drawn_box.height = -g_drawn_box.height;
        }
        
        // Only register if the box size is valid
        if (g_drawn_box.width > 5 && g_drawn_box.height > 5) {
            g_has_box = true;
        }
    } else if (event == cv::EVENT_RBUTTONDOWN) {
        // Print clicked coordinates for calibration
        double norm_x = (double)x / width;
        double norm_y = (double)y / height;
        std::cout << "Clicked Point: Pixels=(" << x << ", " << y 
                  << ") | Code Format: cv::Point(width * " << norm_x << ", height * " << norm_y << ")" << std::endl;
    }
}

int main() {
    std::cout << "=== RTSP Camera Stream with Custom ROI Calibration ===" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "  - Left-click & drag: Draw a bounding box to simulate a detected object." << std::endl;
    std::cout << "  - Right-click: Print clicked coordinates to terminal (useful for calibrating ROI corners)." << std::endl;
    std::cout << "  - Press 'ESC': Exit the program." << std::endl;

    std::string rtsp_url = "rtsp://admin:rtc%402025@192.168.5.201:554/cam/realmonitor?channel=1&subtype=0";

    // Connect to RTSP stream
    cv::VideoCapture cap(rtsp_url, cv::CAP_FFMPEG);

    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to connect to RTSP stream!" << std::endl;
        return -1;
    }

    std::string window_name = "RTSP Real-time Stream & ROI Detection";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL);
    
    // Bind the mouse callback
    cv::setMouseCallback(window_name, onMouse, nullptr);

    cv::Mat frame;
    while (true) {
        if (!cap.read(frame)) {
            std::cerr << "Warning: Failed to grab frame, camera offline..." << std::endl;
            cv::waitKey(100);
            continue;
        }

        // Update the global frame size for the mouse callback
        g_frame_size = frame.size();
        int width = frame.cols;
        int height = frame.rows;

        // Custom ROI points focused on the yellow-and-black floor markings under the metal stand
        // These coordinates are estimated from the camera perspective:
        std::vector<cv::Point> roi_points;
        roi_points.push_back(cv::Point(width * 0.605, height * 0.360)); // Top-Left marker
        roi_points.push_back(cv::Point(width * 0.750, height * 0.400)); // Top-Right marker
        roi_points.push_back(cv::Point(width * 0.810, height * 0.540)); // Bottom-Right marker
        roi_points.push_back(cv::Point(width * 0.680, height * 0.440)); // Bottom-Left marker

        // Check if the simulated object is inside the custom ROI
        bool is_inside = false;
        cv::Point check_point(0, 0);

        if (g_is_drawing || g_has_box) {
            // Calculate bottom-center of the box
            check_point.x = g_drawn_box.x + g_drawn_box.width / 2;
            check_point.y = g_drawn_box.y + g_drawn_box.height;

            // Run pointPolygonTest
            double test = cv::pointPolygonTest(roi_points, cv::Point2f(check_point.x, check_point.y), false);
            if (test >= 0) {
                is_inside = true;
            }
        }

        // Render the ROI Zone (Red if object inside, Green if safe)
        cv::Scalar roi_color = is_inside ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
        cv::Mat overlay = frame.clone();
        std::vector<std::vector<cv::Point>> polygons = { roi_points };
        
        // Draw translucent ROI area
        cv::fillPoly(overlay, polygons, roi_color);
        cv::addWeighted(overlay, 0.25, frame, 0.75, 0, frame);
        
        // Draw boundary lines
        cv::polylines(frame, polygons, true, roi_color, 3);

        // Draw the simulated bounding box
        if (g_is_drawing || g_has_box) {
            // Draw box (Cyan)
            cv::rectangle(frame, g_drawn_box, cv::Scalar(255, 255, 0), 2);
            // Draw reference point (Red dot)
            cv::circle(frame, check_point, 6, cv::Scalar(0, 0, 255), -1);
            
            // Label reference point
            cv::putText(frame, "Ref Point", 
                        cv::Point(check_point.x + 10, check_point.y - 10), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
        }

        // Display status text
        std::string status_text = "STATUS: SAFE";
        if (is_inside) {
            status_text = "WARNING: Stand/Object in Custom Zone!";
        }
        cv::putText(frame, status_text, cv::Point(30, 50), 
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, roi_color, 3);

        // Help text at the bottom
        cv::putText(frame, "Right-click corners of floor markings to print coordinates in console.", 
                    cv::Point(15, height - 20), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1);

        // Display frame
        cv::imshow(window_name, frame);

        // Press ESC to exit
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "Program closed." << std::endl;
    return 0;
}
