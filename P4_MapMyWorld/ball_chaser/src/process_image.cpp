#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
	// Request a service and pass the velocities to it to drive the robot
	ROS_INFO_STREAM("Moving the robot towards the white ball");
	ball_chaser::DriveToTarget srv;
	srv.request.linear_x = lin_x;
	srv.request.angular_z = ang_z;
	
	// Call the service and pass the requested velocities (linear_x, angular_z)
	if (!client.call(srv))
		ROS_ERROR("Failed to call the service");
}

// This callback function continuosly executed and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

	// Loop through each pixel in the image and check if there's a bright white one
	// Then, identify if this pixel falls in the left, mid, or right side of the image
	// Depending on the white ball position, call the drive_bot function and pass velocities to it
	// Request a stop when there's no white ball seen by the camera
	// docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html
	// height: image height, number of rows
	// width: image width, number of columns
	// step: Full row length in bytes
	// data: actual matrix data, size is (step * rows)
	
	int white_pixel = 255;
	// Dividing the current image in three sections. Width is based on step variable
	int left_image = img.step / 3;
	int right_image = (2 * img.step) / 3; 
	bool isBall = false;

	// Looping through the full image
	for (int i = 0; i < img.height * img.step; i++) {
		// If white pixel is present in the image for every channel (RGB
		if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i+2] == white_pixel) {
			isBall = true;
			// Cheking where the white pixel is located: left, center or right
			// If the white pixel is present in the left side of the image
			if (i % img.step < left_image) {
				// Move the robot to the left calling drive_robot with linear and angular velocities
				drive_robot(0.5, -1.0);
			}
			// If the white pixel is present in the right side of the image
			else if (i % img.step > right_image) {
				// Move the robot to the right calling drive_robot with linear and angular velocities
				drive_robot(0.5, 1.0);
			}
			// If the white pixel is present in the center of the image
			else {
				// Move the robot straight forward calling drive_robot with linear and angular velocities	
				drive_robot(1.0, 0.0);
			}
			break;
		}
	}
	if (!isBall) {
		// Stop the robot calling drive_robot passing 0.0 for linear and angular velocities
		drive_robot(0.0, 0.0);
	}
		 			
}

int main(int argc, char** argv)
{
	// Initialize the process_image node and create a handle to it
	ros::init(argc, argv, "process_image");
	ros::NodeHandle n;

	// Define a client service capable of requesting services from command_robot
	client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

	// Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
	ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

	// Handle ROS communication events
	ros::spin();

	return 0;
}
