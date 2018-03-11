# FS_detection
#Fire and smoke detection collaboration repository

Things have been done so far:
1. finished labeling MIVIS dataset

task as of 11.03.2018
1.Try different background subtraction (BGS) algorithms and make comparison
 1.1 dynamic background update and dark channel prior
 2.2 from BGS Library (https://github.com/andrewssobral/bgslibrary)
2. with a valid BGS output, give the cropped, masked patch to AlexNet for training
