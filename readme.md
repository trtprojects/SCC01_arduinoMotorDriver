# [Repo Name Here]
* Motor Driver Code for SCC

## Description
* For the flow through system, in SCC, a set of motors are needed to raster through a 96-well plate.  Associated code is contained herein.

* SCC_96_Well_Fixture_R1.ino: arduino motor control code
* arduinoSerialComm.vi: labview VI to send serial communication to arduino and trigger events

## Getting Started

### Dependencies
* Code herein uses Arduino IDE and/or LabVIEW.
* TODO: Add in requirements for Arduino and Labview.

### Installing
* How/where to download your program (site requirements, if created, file here)
* Any modifications needed to be made to files/folders

### Executing program
Arduino script may be used with arduino IDE as follows:
* sample input: XXY12345 generic
    * First two letters identify the function called
    * Third letter is parsed to determine the axis dependent commands for the X, Y & Z axis Otherwise the letter is meaningless.
    * The last segment is turned into a number for length of move, etc.  For defining the well other than 1,1, need three letters first (ex.  nwp [New Well position]) and as you suggest, 0512 cam be parsed to x=5 and Y=12.

More specific command info:
Low level commands, probably for debug:
* abX1234 (command individual motors as you describe)
* psX (request position of axis X, would need to add two-way) (maybe you could run something like this after each homing to make sure everything’s behaving well)
* hm  (have you been ‘homed’ since power on?) <TRUE | FALSE>
 
Application type commands:
* mv – move to next well
* gf – go to first well
* nw_XXYY – as you describe, but I suggest to require both X and Y to be two digits?  So nw0508.. maybe not necessary, but easier?
* hm – home all motors
* cw – get current position in well coordinates < XXYY >
* st – return status.. maybe responses are <MOVING | READY | ERROR>

## Help
* Any advise for common problems or issues.
```
command to run if program contains helper info
```

## Authors
Names and contributions (i.e. developer, reviewer, etc)

## Release History 
* Projects with releases should utilize git tags to annotate the release in git history.  Additional details about the releases should be provided here. [May not be used for all projects.]
* 0.2
    * Various bug fixes and optimizations
    * See [commit change]() or See [release history]()
* 0.1
    * Initial Release

## References
* Literature or code snippet references should be included here.
