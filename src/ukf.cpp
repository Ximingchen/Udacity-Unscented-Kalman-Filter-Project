#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 * This is scaffolding, do not modify
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 30;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 30;
  
  //DO NOT MODIFY measurement noise values below these are provided by the sensor manufacturer.
  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  //DO NOT MODIFY measurement noise values above these are provided by the sensor manufacturer.
  
  /**
  TODO:

  Complete the initialization. See ukf.h for other member properties.

  Hint: one or more values initialized above might be wildly off...
  */
  n_x_ = 5; // dimension of the state

  n_aug_ = 7; // dimension of the augmented state

  lambda_ = 3 - n_aug_;

  ///* state vector: [pos1 pos2 vel_abs yaw_angle yaw_rate] in SI units and rad
  //VectorXd x_;

  ///* state covariance matrix
 // MatrixXd P_;

  ///* predicted sigma points matrix
  //MatrixXd Xsig_pred_;

  ///* Weights of sigma points
  //VectorXd weights_;

  ///* Sigma point spreading parameter
  //double lambda_;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
	/**
	TODO:
	Complete this function! Make sure you switch between lidar and radar
	measurements.
	*/
	if (!is_initialized_) {
		// first measurement
		x_ << 0, 0, 0, 0, 0; // not useful

		if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
			//			Convert radar from polar to cartesian coordinates and initialize state.
			cout << "First sample: Radar" << endl;
			float rho = meas_package.raw_measurements_[0];
			float phi = meas_package.raw_measurements_[1];
			float px = rho * cos(phi);
			float py = rho * sin(phi);

			float rho_dot = meas_package.raw_measurements_[2];
			float vx = rho_dot * cos(phi);
			float vy = rho_dot * sin(phi);
			float v = sqrt(vx * vx + vy * vy);
			
			x_ << px, py, v, 0, 0;
		}
		else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
			x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 0.0, 0.0;
		}
		time_us_ = meas_package.timestamp_;// update the time-stamp done initializing, no need to predict or update
		is_initialized_ = true;
		return;
	}
	
	// initialized already
	float dt = (meas_package.timestamp_ - time_us_) / 1000000.0;	//dt - expressed in seconds
	time_us_ = meas_package.timestamp_; // updated time-stamp
	Prediction(dt);

	// measurement update
	if (meas_package.sensor_type_ == MeasurementPackage::RADAR && use_radar_) {
		// Radar updates
		UpdateRadar(meas_package.raw_measurements_);
	}
	if(meas_package.sensor_type_ == MeasurementPackage::LASER && use_laser_){
		//lidar update
		UpdateLidar(meas_package.raw_measurements_);
	}
	// print the output
	cout << "x_ = " << x_ << endl;
	cout << "P_ = " << P_ << endl;
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
  /**
  TODO:

  Complete this function! Estimate the object's location. Modify the state
  vector, x_. Predict sigma points, the state, and the state covariance matrix.
  */
	// prection involve 3 parts:
	// 1. generate sigma points
	// 2. predict sigma points
	// 3. predict mean and covariance

	VectorXd x_aug = VectorXd(n_aug_);
	x_aug.head(n_x) = x_; // the first five entries equal to the state vector x_
	x_aug(n_x) = 0;
	x_aug(n_x + 1) = 0;

	//create augmented state covariance
	MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);
	P_aug.fill(0.0);
	P_aug.topLeftCorner(n_x, n_x) = P_;
	P_aug(n_x, n_x) = std_a * std_a;					// process noise standard deviation longitudinal acceleration in m/s^2
	P_aug(n_x + 1, n_x + 1) = std_yawdd * std_yawdd;	//Process noise standard deviation yaw acceleration in rad/s^2

	//create sigma point matrix
	MatrixXd Xsig_aug = MatrixXd(n_aug, 2 * n_aug + 1);
	// Step 1: generate sigma points
	GenerateSigmaPoints(Xsig_aug, x_aug, P_aug);
	
}

/*


	// Step 2: predict sigma points

	// sample call MatrixXd Xsig_aug = MatrixXd(7, 15);


		for (int i = 0; i< 2 * n_aug + 1; i++)
		{
			//extract values for better readability
			double p_x = Xsig_aug(0, i);
			double p_y = Xsig_aug(1, i);
			double v = Xsig_aug(2, i);
			double yaw = Xsig_aug(3, i);
			double yawd = Xsig_aug(4, i);
			double nu_a = Xsig_aug(5, i);
			double nu_yawdd = Xsig_aug(6, i);

			//predicted state values
			double px_p, py_p;

			//avoid division by zero
			if (fabs(yawd) > 0.001) {
				px_p = p_x + v / yawd * (sin(yaw + yawd * delta_t) - sin(yaw));
				py_p = p_y + v / yawd * (cos(yaw) - cos(yaw + yawd * delta_t));
			}
			else {
				px_p = p_x + v * delta_t*cos(yaw);
				py_p = p_y + v * delta_t*sin(yaw);
			}

			double v_p = v;
			double yaw_p = yaw + yawd * delta_t;
			double yawd_p = yawd;

			//add noise
			px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
			py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
			v_p = v_p + nu_a * delta_t;

			yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
			yawd_p = yawd_p + nu_yawdd * delta_t;

			//write predicted sigma point into right column
			Xsig_pred(0, i) = px_p;
			Xsig_pred(1, i) = py_p;
			Xsig_pred(2, i) = v_p;
			Xsig_pred(3, i) = yaw_p;
			Xsig_pred(4, i) = yawd_p;
		}
		*Xsig_out = Xsig_pred;*/

void UKF::GenerateSigmaPoints(MatrixXd& Xsig_aug, VectorXd & x_aug, MatrixXd& P_aug) {
	MatrixXd L = P_aug.llt().matrixL();
	//create augmented sigma points
	Xsig_aug.col(0) = x_aug;
	//create augmented mean state
	for (int i = 0; i< n_aug_; i++)
	{
		Xsig_aug.col(i + 1) = x_aug + sqrt(lambda_ + n_aug_) * L.col(i);
		Xsig_aug.col(i + 1 + n_aug_) = x_aug - sqrt(lambda_ + n_aug_) * L.col(i);
	}
}
/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use lidar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the lidar NIS.
  */

}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use radar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the radar NIS.
  */
	//set measurement dimension, radar can measure r, phi, and r_dot
	/*
	int n_z = 3;

	//set vector for weights
	VectorXd weights = VectorXd(2 * n_aug + 1);
	double weight_0 = lambda / (lambda + n_aug);
	weights(0) = weight_0;
	for (int i = 1; i<2 * n_aug + 1; i++) {  //2n+1 weights
		double weight = 0.5 / (n_aug + lambda);
		weights(i) = weight;
	}

	//create example matrix with predicted sigma points
	Xsig_pred // a 5 x 2*7 + 1 matrix
	
	//create example matrix with sigma points in measurement space
	MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug + 1);
	Zsig <<
		6.1190, 6.2334, 6.1531, 6.1283, 6.1143, 6.1190, 6.1221, 6.1190, 6.0079, 6.0883, 6.1125, 6.1248, 6.1190, 6.1188, 6.12057,
		0.24428, 0.2337, 0.27316, 0.24616, 0.24846, 0.24428, 0.24530, 0.24428, 0.25700, 0.21692, 0.24433, 0.24193, 0.24428, 0.24515, 0.245239,
		2.1104, 2.2188, 2.0639, 2.187, 2.0341, 2.1061, 2.1450, 2.1092, 2.0016, 2.129, 2.0346, 2.1651, 2.1145, 2.0786, 2.11295;

	//create example vector for mean predicted measurement
	VectorXd z_pred = VectorXd(n_z);
	z_pred <<
		6.12155,
		0.245993,
		2.10313;

	//create example matrix for predicted measurement covariance
	MatrixXd S = MatrixXd(n_z, n_z);
	S <<
		0.0946171, -0.000139448, 0.00407016,
		-0.000139448, 0.000617548, -0.000770652,
		0.00407016, -0.000770652, 0.0180917;

	//create example vector for incoming radar measurement
	VectorXd z = VectorXd(n_z);
	z <<
		5.9214,
		0.2187,
		2.0062;

	//create matrix for cross correlation Tc
	MatrixXd Tc = MatrixXd(n_x, n_z);

	/
	******************************************************************************
	* Student part begin
	******************************************************************************

	//calculate cross correlation matrix
	Tc.fill(0.0);
	for (int i = 0; i < 2 * n_aug + 1; i++) {  //2n+1 simga points

											   //residual
		VectorXd z_diff = Zsig.col(i) - z_pred;
		//angle normalization
		while (z_diff(1)> M_PI) z_diff(1) -= 2.*M_PI;
		while (z_diff(1)<-M_PI) z_diff(1) += 2.*M_PI;

		// state difference
		VectorXd x_diff = Xsig_pred.col(i) - x;
		//angle normalization
		while (x_diff(3)> M_PI) x_diff(3) -= 2.*M_PI;
		while (x_diff(3)<-M_PI) x_diff(3) += 2.*M_PI;

		Tc = Tc + weights(i) * x_diff * z_diff.transpose();
	}

	//Kalman gain K;
	MatrixXd K = Tc * S.inverse();

	//residual
	VectorXd z_diff = z - z_pred;

	//angle normalization
	while (z_diff(1)> M_PI) z_diff(1) -= 2.*M_PI;
	while (z_diff(1)<-M_PI) z_diff(1) += 2.*M_PI;

	//update state mean and covariance matrix
	x = x + K * z_diff;
	P = P - K * S*K.transpose();
	*/
}
