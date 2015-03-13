//
//  ViewController.h
//  AccelerometerTest_iOS
//
//  Created by Elliot Saba on 6/2/14.
//  Copyright (c) 2014 University of Washington. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>


@interface ViewController : UIViewController {
    // Create outlets for InterfaceBuilder that represent that label and progress view
    IBOutlet UILabel * label;
    IBOutlet UIProgressView * progress;

    // Create a pointer that points to something of type CMMotionManager called motionManager
    CMMotionManager *motionManager;
}

// Keep around our own operationQueue (like a Dispatcher)
@property NSOperationQueue * operationQueue;

// The function that gets called when we press the button
-(IBAction) buttonPressed;

@end
