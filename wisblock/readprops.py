

import subprocess
import configparser
import traceback
import sys


def readProps(prefsLoc):
    """Read the version of our project as a string"""

    try:
        config = configparser.RawConfigParser()
        config.read(prefsLoc)
        version = dict(config.items('VERSION'))
        verObj = dict(short = "{}.{}.{}".format(version["major"], version["minor"], version["build"]),
            long = "{}.{}.{}".format(version["major"], version["minor"], version["build"]))
    except:
        verObj = dict(short = "0.0.0", long = "0.0.0")

    # print("firmware version " + verStr)
    return verObj
# print("path is" + ','.join(sys.path))
