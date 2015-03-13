//
//  ViewController.m
//  AccelerometerTest_iOS
//
//  Created by Elliot Saba on 6/2/14.
//  Copyright (c) 2014 University of Washington. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    
    // Allocate and initialize CMMotionManager instance
    // This syntax is the Obj-C analogue to the following in C++:
    //
    // this->motionManager = CMMotionManager::alloc();
    // this->motionManager->init();
    //
    motionManager = [[CMMotionManager alloc] init];
    
    // set an update interval
    motionManager.accelerometerUpdateInterval = .2;
    
    // Tell the motionManager to start performing accelerometer updates, and call this
    [motionManager startAccelerometerUpdatesToQueue:[NSOperationQueue currentQueue]
                                        withHandler:^(CMAccelerometerData *accelerometerData, NSError *error) {
        [progress setProgress: accelerometerData.acceleration.x];
    }];
    
    // Set our progress bar to a default 0.5f
    [progress setProgress: 0.5f];
}

- (IBAction) buttonPressed
{
    [label setText: @"Hello, EE 590!"];
}






// Non-interesting stuff
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
