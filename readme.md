# [SCC_arduinoMotorDriver]
* Motor Driver Code for SCC

## Description
* For the flow through system, in SCC, a set of motors are needed to raster through a 96-well plate.  Associated code is contained herein.

* SCC_96_Well_Fixture_TNC2209.ino: arduino motor control code
* arduinoOpenComPort.vi: opens serial port communication to Arduino
* arduinoCloseComPort.vi: closes serial port communcation to Arduino
* arduinoSendCommand.vi: Sends command to open serial port - takes raw text command as argument; includes parameters to control timeout and command validation

## Getting Started

### Dependencies
* Code herein is Arduino or LabVIEW.
* TODO: Add in requirements for Arduino and Labview.

### Installing
* How/where to download your program (site requirements, if created, file here)
* Any modifications needed to be made to files/folders

### Executing program
Arduino script may be used with arduino IDE as follows:
* sample input: XXY12345 generic
    * First two letters identify the function called
    * Third letter (optional) is parsed to determine the axis dependent commands for the X, Y & Z axis Otherwise the letter is meaningless.
    * The last segment (optional) is turned into a number for length of move, etc.  For defining the well other than 1,1, need three letters first (ex.  nwp [New Well position]) and as you suggest, 0512 cam be parsed to x=5 and Y=12.

More specific command info:
Low level commands, probably for debug:
* abAnnnn (command individual motor on axis 'A' to position 'nnnn') ('A' needs to be lower case, like 'x' or 'y')
* reAnnnn (command individual motor on axis 'A' to move by 'nnnn' steps, relative to current position) ('A' needs to be lower case, like 'x' or 'y')
* psX (request position of axis X, would need to add two-way) (maybe you could run something like this after each homing to make sure everything’s behaving well)
* tm  (test motor)
* he  (produce help output)
 
Application type commands:
* poAnnnn – (set position of motor on axis 'A' to position 'nnnn') (I think this tells the motor what position it's at - not tell it to move)
* mo – move to next well
* fw – go to first well
* ho – home all motors
* cw – get current position in well coordinates < NN >
* st – return status.. maybe responses are <MOVING | READY | ERROR>

## Help
* Any advise for common problems or issues.
```
command to run if program contains helper info
```

## Authors
Various Chrises

## Release History 
* Projects with releases should utilize git tags to annotate the release in git history.  Additional details about the releases should be provided here. [May not be used for all projects.]
* 0.2
    * Various bug fixes and optimizations
    * See [commit change]() or See [release history]()
* 0.1
    * Initial Release

## References
* Literature or code snippet references should be included here.
