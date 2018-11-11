# Unscented Kalman Filter Project Starter Code
Self-Driving Car Engineer Nanodegree Program

In this project utilize an Unscented Kalman Filter to estimate the state of a moving object of interest with noisy lidar and radar measurements. Passing the project requires obtaining RMSE values that are lower that the tolerance outlined in the project rubric. 

See the description under repository - Udacity-Extended-Kalman-Filter for environment setup etc.

## Generating Additional Data

This is optional!

If you'd like to generate your own radar and lidar data, see the
[utilities repo](https://github.com/udacity/CarND-Mercedes-SF-Utilities) for
Matlab scripts that can generate additional data.


## Basic pipeline of the code - UKF algorithm
The ukf algorithm consists of two main steps:
  I: Prediction (time update in KF)
     1. Generate sigma points 
        * Since x(k+1) = F(x(k), ..) is non-linear, given the current state, we sample a few points from the current assume distribution of the state according to a deterministic algorithm. We assume that the current distribution is normal with mean x and covariance P.
        * The number of generated sigma points is related to the dimension of the state space
        * We also consider augmenting the state space so that the process errors remain independent from each other and are normal
     2. Predict sigma points
        * Take each of the sigma points and process them through the dynamical equation
     3. Predict mean and covariance - the predicted mean will be served as the estimated state
        * The new mean and covariance is calculated by the weighted mean and weighted covariance of the processed sigma points in I.2
        * Thus, the mean served as x(k+1|k) in the Kalman Filter.
  II: Update (measurement update KF)
     1. Predict measurement
        * Since z(k+1) = G(x(k+1), ..), to calculate the estimated output of z(k+1), again we have to use the sigma point trick. However, as a number of sigma points have already been generated in I.3, we can just use those. Thus, this step corresponds to I.2 above.
     2. Update state
        * When measurement arrives, the new posterior estimation is calcuated using the Kalman gain:
        K = P(x,z)* P(z,z)^-1;
        x+ = x + K(z - z_pred);
        P = P - K P(z, z) K^T.

## Project Instructions and Rubric

This information is only accessible by people who are already enrolled in Term 2
of CarND. If you are enrolled, see [the project page](https://classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/c3eb3583-17b2-4d83-abf7-d852ae1b9fff/concepts/f437b8b0-f2d8-43b0-9662-72ac4e4029c1)
for instructions and the project rubric.

## How to write a README
A well written README file can enhance your project and portfolio.  Develop your abilities to create professional README files by completing [this free course](https://www.udacity.com/course/writing-readmes--ud777).

